/**
 * @file containerstore.c
 * IDEA Added here the ability to 
 * mark chunks as indexed.
 * This required to replace fixed-variables 
 * inside the file to a general struct, which
 * will further enable adding more fields to 
 * the file in the future :).
 */
#include "containerstore.h"
#include "../utils/serial.h"
#include "../utils/sync_queue.h"
#include "../jcr.h"
#include "errno.h"


static int64_t container_count = 0;


static int fd;
/* Control the concurrent accesses to fp. */
static pthread_mutex_t mutex;

static pthread_t append_t;

static bool append_thread_terminated = false;

static SyncQueue *container_buffer;

/*
 * We must ensure a container is either in the buffer or written to disks.
 */
static void *append_thread(void *arg)
{
	while (1)
	{
		struct containerWriteRequest *req = sync_queue_get_top(container_buffer);
		if (req == NULL) {
			break;
		}

		TIMER_DECLARE(1);
		TIMER_BEGIN(1);

		if (req->should_write_only_meta) {
			write_container_meta_no_fp(req->c);
		} else {
			write_container(req->c);
		}

		TIMER_END(1, jcr.write_time);

		sync_queue_pop(container_buffer);

		free_container(req->c);
		free(req);
	}

	return NULL;
}

void init_container_store()
{

	sds containerfile = sdsdup(destor.working_directory);
	containerfile = sdscat(containerfile, "/container.pool");

	fd = open(containerfile, O_RDWR | O_SYNC);
	if (fd != -1)
	{
		read(fd, &container_count, 8);
	}
	else
	{
		fd = open(containerfile, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0644);
		if (fd == -1) {
			perror("Can not create container.pool for read and write because");
			exit(1);
		}
	}

	sdsfree(containerfile);

	container_buffer = sync_queue_new(25);

	pthread_mutex_init(&mutex, NULL);

	pthread_create(&append_t, NULL, append_thread, NULL);

	NOTICE("Init container store successfully");
}

void close_container_store()
{
	sync_queue_term(container_buffer);

	if (!append_thread_terminated)
		pthread_join(append_t, NULL);
	NOTICE("append phase stops successfully!");

	lseek(fd, 0, SEEK_SET);
	write(fd, &container_count, sizeof(container_count));

	close(fd);
	fd = -1;

	pthread_mutex_destroy(&mutex);
}

void gracefully_terminate_append_thread() {
	sync_queue_push(container_buffer, NULL);

	pthread_join(append_t, NULL);
	append_thread_terminated = true;
}

static void init_container_meta(struct containerMeta *meta)
{
	meta->chunk_num = 0;
	meta->data_size = 0;
	meta->id = TEMPORARY_ID;
	meta->is_indexed = 0;
	meta->map = g_hash_table_new_full(g_int_hash, g_fingerprint_equal, NULL,
									  free);
}

/*
 * For backup.
 */
struct container *create_container()
{
	struct container *c = (struct container *)malloc(sizeof(struct container));
	if (destor.simulation_level < SIMULATION_APPEND)
		c->data = calloc(1, CONTAINER_SIZE);
	else
		c->data = 0;

	init_container_meta(&c->meta);
	c->meta.id = container_count++;
	return c;
}

containerid get_container_id(struct container *c) // DedupSearch removed inline
{
	return c->meta.id;
}

void write_container_async(struct containerWriteRequest *req)
{
	struct container *c = req->c;
	assert(c->meta.chunk_num == g_hash_table_size(c->meta.map));

	if (container_empty(c))
	{
		/* An empty container
		 * It possibly occurs in the end of backup */
		container_count--;
		VERBOSE("Append phase: Deny writing an empty container %lld",
				c->meta.id);
		return;
	}

	sync_queue_push(container_buffer, req);
}

/*
 * Called by Append phase
 */
void write_container_meta_no_fp(struct container* c) {
	unsigned char *cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
	ser_declare;
	ser_begin(cur, CONTAINER_META_SIZE);
	ser_int64(c->meta.id);
	ser_int32(c->meta.chunk_num);
	ser_int32(c->meta.data_size);
	ser_int32(c->meta.is_indexed);
	ser_end(cur, CONTAINER_META_SIZE);

	pthread_mutex_lock(&mutex);

	cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];

	if (lseek(fd, CONTAINER_POOL_META + (c->meta.id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE, SEEK_SET) == -1)
	{
		perror("Fail seek in container store.");
		exit(1);
	}
	if (write(fd, cur, CONTAINER_META_SIZE_NO_FP) != CONTAINER_META_SIZE_NO_FP)
	{
		perror("Fail to write a container in container store.");
		exit(1);
	}

	pthread_mutex_unlock(&mutex);
}

/*
 * Called by Append phase
 */
void write_container(struct container *c)
{

	assert(c->meta.chunk_num == g_hash_table_size(c->meta.map));

	if (container_empty(c))
	{
		/* An empty container
		 * It possibly occurs in the end of backup */
		container_count--;
		VERBOSE("Append phase: Deny writing an empty container %lld",
				c->meta.id);
		return;
	}

	VERBOSE("Append phase: Writing container %lld of %d chunks", c->meta.id,
			c->meta.chunk_num);

	if (destor.simulation_level < SIMULATION_APPEND)
	{

		unsigned char *cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
		ser_declare;
		ser_begin(cur, CONTAINER_META_SIZE);
		ser_int64(c->meta.id);
		ser_int32(c->meta.chunk_num);
		ser_int32(c->meta.data_size);
		ser_int32(c->meta.is_indexed);

		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init(&iter, c->meta.map);
		while (g_hash_table_iter_next(&iter, &key, &value))
		{
			struct metaEntry *me = (struct metaEntry *)value;
			ser_bytes(&me->fp, sizeof(fingerprint));
			ser_bytes(&me->len, sizeof(int32_t));
			ser_bytes(&me->off, sizeof(int32_t));
		}

		ser_end(cur, CONTAINER_META_SIZE);

		pthread_mutex_lock(&mutex);

		if (lseek(fd, c->meta.id * CONTAINER_SIZE + CONTAINER_POOL_META, SEEK_SET) == -1)
		{
			perror("Fail seek in container store.");
			exit(1);
		}
		if (write(fd, c->data, CONTAINER_SIZE) != CONTAINER_SIZE)
		{
			perror("Fail to write a container in container store.");
			printf("errno: %d\n", errno);
			exit(1);
		}

		pthread_mutex_unlock(&mutex);
	}
	else
	{
		char buf[CONTAINER_META_SIZE];
		memset(buf, 0, CONTAINER_META_SIZE);

		ser_declare;
		ser_begin(buf, CONTAINER_META_SIZE);
		ser_int64(c->meta.id);
		ser_int32(c->meta.chunk_num);
		ser_int32(c->meta.data_size);
		ser_int32(c->meta.is_indexed);

		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init(&iter, c->meta.map);
		while (g_hash_table_iter_next(&iter, &key, &value))
		{
			struct metaEntry *me = (struct metaEntry *)value;
			ser_bytes(&me->fp, sizeof(fingerprint));
			ser_bytes(&me->len, sizeof(int32_t));
			ser_bytes(&me->off, sizeof(int32_t));
		}

		ser_end(buf, CONTAINER_META_SIZE);

		pthread_mutex_lock(&mutex);

		// if (fseek(fp, c->meta.id * CONTAINER_META_SIZE + CONTAINER_POOL_META, SEEK_SET) != 0)
		if (lseek(fd, c->meta.id * CONTAINER_META_SIZE + CONTAINER_POOL_META, SEEK_SET) == -1)
		{
			perror("Fail seek in container store.");
			exit(1);
		}
		// if (fwrite(buf, CONTAINER_META_SIZE, 1, fp) != 1)
		if (write(fd, buf, CONTAINER_META_SIZE) != CONTAINER_META_SIZE)
		{
			perror("Fail to write a container in container store.");
			exit(1);
		}

		pthread_mutex_unlock(&mutex);
	}
}

struct container *retrieve_container_by_id(containerid id)
{
	struct container *c = (struct container *)malloc(sizeof(struct container));

	init_container_meta(&c->meta);

	unsigned char *cur = 0;
	if (destor.simulation_level >= SIMULATION_RESTORE)
	{
		c->data = malloc(CONTAINER_META_SIZE);

		pthread_mutex_lock(&mutex);

		if (destor.simulation_level >= SIMULATION_APPEND)
			// fseek(fp, id * CONTAINER_META_SIZE + CONTAINER_POOL_META, SEEK_SET);
			lseek(fd, id * CONTAINER_META_SIZE + CONTAINER_POOL_META, SEEK_SET);
		else
			// fseek(fp, (id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE + CONTAINER_POOL_META,
			// 	  SEEK_SET);
			lseek(fd, (id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE + CONTAINER_POOL_META,
				  SEEK_SET);

		//fread(c->data, CONTAINER_META_SIZE, 1, fp);
		read(fd, c->data, CONTAINER_META_SIZE); // DedupSearch

		pthread_mutex_unlock(&mutex);

		cur = c->data;
	}
	else
	{
		c->data = malloc(CONTAINER_SIZE);
		
		pthread_mutex_lock(&mutex);

		lseek(fd, id * CONTAINER_SIZE + CONTAINER_POOL_META, SEEK_SET);
		read(fd, c->data, CONTAINER_SIZE);
		
		pthread_mutex_unlock(&mutex);

		cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
	}

	unser_declare;
	unser_begin(cur, CONTAINER_META_SIZE);

	unser_int64(c->meta.id);
	unser_int32(c->meta.chunk_num);
	unser_int32(c->meta.data_size);
	unser_int32(c->meta.is_indexed);

	if (c->meta.id != id)
	{
		WARNING("expect %lld, but read %lld", id, c->meta.id);
		assert(c->meta.id == id);
	}

	int i;
	for (i = 0; i < c->meta.chunk_num; i++)
	{
		struct metaEntry *me = (struct metaEntry *)malloc(
			sizeof(struct metaEntry));
		unser_bytes(&me->fp, sizeof(fingerprint));
		unser_bytes(&me->len, sizeof(int32_t));
		unser_bytes(&me->off, sizeof(int32_t));
		g_hash_table_insert(c->meta.map, &me->fp, me);
	}

	unser_end(cur, CONTAINER_META_SIZE);

	if (destor.simulation_level >= SIMULATION_RESTORE)
	{
		free(c->data);
		c->data = 0;
	}

	return c;
}

static struct containerMeta *container_meta_duplicate(struct container *c)
{
	struct containerMeta *base = &c->meta;
	struct containerMeta *dup = (struct containerMeta *)malloc(
		sizeof(struct containerMeta));
	init_container_meta(dup);
	dup->id = base->id;
	dup->chunk_num = base->chunk_num;
	dup->data_size = base->data_size;
	dup->is_indexed = base->is_indexed;

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, base->map);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		struct metaEntry *me = (struct metaEntry *)malloc(
			sizeof(struct metaEntry));
		memcpy(me, value, sizeof(struct metaEntry));
		g_hash_table_insert(dup->map, &me->fp, me);
	}

	return dup;
}

static struct containerMeta* container_meta_duplicate_from_request(struct containerWriteRequest* req) {
	return container_meta_duplicate(req->c);
}

struct containerMeta *retrieve_container_meta_by_id(containerid id)
{
	struct containerMeta *cm = NULL;

	/* First, we find it in the buffer */
	cm = sync_queue_find(container_buffer, container_request_check_id, &id,
						 container_meta_duplicate_from_request);

	if (cm)
		return cm;

	cm = (struct containerMeta *)malloc(sizeof(struct containerMeta));
	init_container_meta(cm);

	unsigned char buf[CONTAINER_META_SIZE];

	pthread_mutex_lock(&mutex);

	if (destor.simulation_level >= SIMULATION_APPEND)
		lseek(fd, id * CONTAINER_META_SIZE + CONTAINER_POOL_META, SEEK_SET);
	else
		lseek(fd, (id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE + CONTAINER_POOL_META, SEEK_SET);

	read(fd, buf, CONTAINER_META_SIZE);

	pthread_mutex_unlock(&mutex);

	unser_declare;
	unser_begin(buf, CONTAINER_META_SIZE);

	unser_int64(cm->id);
	unser_int32(cm->chunk_num);
	unser_int32(cm->data_size);
	unser_int32(cm->is_indexed);

	if (cm->id != id)
	{
		WARNING("expect %lld, but read %lld", id, cm->id);
		assert(cm->id == id);
	}

	int i;
	for (i = 0; i < cm->chunk_num; i++)
	{
		struct metaEntry *me = (struct metaEntry *)malloc(
			sizeof(struct metaEntry));
		unser_bytes(&me->fp, sizeof(fingerprint));
		unser_bytes(&me->len, sizeof(int32_t));
		unser_bytes(&me->off, sizeof(int32_t));
		g_hash_table_insert(cm->map, &me->fp, me);
	}

	return cm;
}

static struct metaEntry *get_metaentry_in_container_meta(
	struct containerMeta *cm, fingerprint *fp)
{
	return g_hash_table_lookup(cm->map, fp);
}

struct chunk *get_chunk_in_container(struct container *c, fingerprint *fp)
{
	struct metaEntry *me = get_metaentry_in_container_meta(&c->meta, fp);

	assert(me);

	struct chunk *ck = new_chunk(me->len);

	if (destor.simulation_level < SIMULATION_RESTORE)
		memcpy(ck->data, c->data + me->off, me->len);

	ck->size = me->len;
	ck->id = c->meta.id;
	memcpy(ck->fp, fp, sizeof(fingerprint));

	return ck;
}

int container_overflow(struct container *c, int32_t size)
{
	if (c->meta.data_size + size > CONTAINER_SIZE - CONTAINER_META_SIZE)
		return 1;

	if ((c->meta.chunk_num + 1) * sizeof(struct metaEntry) + CONTAINER_META_SIZE_NO_FP > CONTAINER_META_SIZE)
		return 1;
	return 0;
}

/*
 * For backup.
 * return 1 indicates success.
 * return 0 indicates fail.
 */
int add_chunk_to_container(struct container *c, struct chunk *ck)
{
	assert(!container_overflow(c, ck->size));
	if (g_hash_table_contains(c->meta.map, &ck->fp))
	{
		NOTICE("Writing a chunk already in the container buffer!");
		ck->id = c->meta.id;
		return 0;
	}

	struct metaEntry *me = (struct metaEntry *)malloc(sizeof(struct metaEntry));
	memcpy(&me->fp, &ck->fp, sizeof(fingerprint));
	me->len = ck->size;
	me->off = c->meta.data_size;

	g_hash_table_insert(c->meta.map, &me->fp, me);
	c->meta.chunk_num++;

	if (destor.simulation_level < SIMULATION_APPEND)
		memcpy(c->data + c->meta.data_size, ck->data, ck->size);

	c->meta.data_size += ck->size;

	ck->id = c->meta.id;

	return 1;
}

void free_container_meta(struct containerMeta *cm)
{
	g_hash_table_destroy(cm->map);
	free(cm);
}

void free_container(struct container *c)
{
	g_hash_table_destroy(c->meta.map);
	if (c->data)
		free(c->data);
	free(c);
}

int container_empty(struct container *c)
{
	return c->meta.chunk_num == 0 ? 1 : 0;
}

/*
 * Return 0 if doesn't exist.
 */
int lookup_fingerprint_in_container_meta(struct containerMeta *cm,
										 fingerprint *fp)
{
	return g_hash_table_lookup(cm->map, fp) == NULL ? 0 : 1;
}

int lookup_fingerprint_in_container(struct container *c, fingerprint *fp)
{
	return lookup_fingerprint_in_container_meta(&c->meta, fp);
}

gint g_container_cmp_desc(struct container *c1, struct container *c2,
						  gpointer user_data)
{
	return g_container_meta_cmp_desc(&c1->meta, &c2->meta, user_data);
}

gint g_container_meta_cmp_desc(struct containerMeta *cm1,
							   struct containerMeta *cm2, gpointer user_data)
{
	return cm2->id - cm1->id;
}

int container_request_check_id(struct containerWriteRequest* req, containerid* id) {
	return container_check_id(req->c, id);
}

int container_check_id(struct container *c, containerid *id)
{
	return container_meta_check_id(&c->meta, id);
}

int container_meta_check_id(struct containerMeta *cm, containerid *id)
{
	return cm->id == *id ? 1 : 0;
}

/*
 * foreach the fingerprints in the container.
 * Apply the 'func' for each fingerprint.
 */
void container_meta_foreach(struct containerMeta *cm, void (*func)(fingerprint *, void *), void *data)
{
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, cm->map);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		func(key, data);
	}
}

int64_t get_container_count()
{
	return container_count;
}
