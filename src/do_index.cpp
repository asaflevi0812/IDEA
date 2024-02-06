/**
 * @file do_index.cpp
 * @author Asaf Levi
 * @brief THIS FILE CONTAINS THE INDEX LOGIC FOR IDEA
 */

#include "destor.h"
#include "jcr.h"
#include "recipe/recipestore.h"
#include "storage/containerstore.h"
#include "clucene-wrapper/libclucene-wrapper.h"
#include "utils/lru_cache.h"
#include "utils/utils.h"
#include "reverse_mapping/external_reverse_mapping.h"
#include "reverse_mapping/berkeley_db_reverse_mapping.h"
#include "reverse_mapping/pathstore.h"
#include "search.h"
#include "rankstore.h"
#include <iostream>
#include <string>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>

#define MAX_UINT_8 255
SyncQueue *restore_container_queue;

extern void* lru_restore_thread(void *arg);

extern double searcher_load_time;
extern double dtor_time;
extern bool reindex;

const char index_log_path[] = "index.log";
const char lookup_log_path[] = "lookup.log";
const char temp_path[] = "temp.log";

static void write_file(const char* log_path, const char* fmt, ...) {
	va_list args;

	va_start(args, fmt);

	FILE *fp = fopen(log_path, "a");
	
	if (fp) {
		vfprintf(fp, fmt, args);
		fclose(fp);
	}
	else {
		puts("Could not open log file! Results will not be saved.\n The path tried to open: ");
		puts(log_path);
		puts("\n");
	}

	va_end(args);
}

static void* read_recipe_thread(void *arg) {

	int i, j, k;
	bool should_reset = true;

	for (i = 0; i < jcr.bv->number_of_files; i++) {
		TIMER_DECLARE(1);
		TIMER_BEGIN(1);

		struct fileRecipeMeta *r = read_next_file_recipe_meta(jcr.bv, should_reset);
		should_reset = false;

		struct chunk *c = new_chunk(sdslen(r->filename) + 1);
		strcpy(c->data, r->filename);
		SET_CHUNK(c, CHUNK_FILE_START);

		TIMER_END(1, jcr.read_recipe_time);

		sync_queue_push(restore_recipe_queue, c);
		bool should_reset = true;
		for (j = 0; j < r->chunknum; j++) {
			TIMER_DECLARE(1);
			TIMER_BEGIN(1);

			struct chunkPointer* cp = read_next_n_chunk_pointers(jcr.bv, 1, &k, should_reset);
			should_reset = false;
			struct chunk* c = new_chunk(0);
			memcpy(&c->fp, &cp->fp, sizeof(fingerprint));
			c->size = cp->size;
			c->id = cp->id;

			TIMER_END(1, jcr.read_recipe_time);

			sync_queue_push(restore_recipe_queue, c);
			free(cp);
		}

		c = new_chunk(0);
		SET_CHUNK(c, CHUNK_FILE_END);
		sync_queue_push(restore_recipe_queue, c);

		free_file_recipe_meta(r);
	}

	sync_queue_term(restore_recipe_queue);
	return NULL;
}

void* index_restore_data(void *arg) {
	struct chunk *c = NULL;
	sds filepath = NULL;
	std::string data;
	auto index_writer = get_index_writer(NAIVE_INDEX_DIR);

	while ((c = sync_queue_pop(restore_chunk_queue))) {

		TIMER_DECLARE(1);
		TIMER_BEGIN(1);

		if (CHECK_CHUNK(c, CHUNK_FILE_START)) {
			VERBOSE("Searching: %s", c->data);

			filepath = sdsdup(c->data);
			data = std::string();
		} else if (CHECK_CHUNK(c, CHUNK_FILE_END)) {
			if (std::string((const char*)filepath) != "enwiki-20170220-pages-articles20.xml-p018754736p021222156") {
				std::string name = std::string((const char*)filepath) + " backup version: " + std::to_string(jcr.bv->bv_num);
				std::wstring wdata = toWstring(data);
				std::wstring wname = toWstring(name);
				jcr.file_num++;
				index_writer = protectedAddDocument(index_writer,
													create_document(wname, wdata),
													NAIVE_INDEX_DIR);
			}


			if (filepath)
				sdsfree(filepath);
			filepath = NULL;
		} else {
			assert(destor.simulation_level == SIMULATION_NO);
			data.append((const char*)c->data, c->size);
		}

		free_chunk(c);

		TIMER_END(1, jcr.search_chunk_time);
	}

	index_writer->close();
    jcr.status = JCR_STATUS_DONE;
    return NULL;
}

void do_naive_index_revision(int revision, double* p_read_recipe_time, double* p_read_chunk_time, 
							 uint64_t* p_file_num, uint64_t* p_chunk_num, uint64_t* p_data_size, 
							 uint64_t* p_read_container_num) {

	init_restore_jcr(revision, ".");

	if (!reindex && CHECK_FLAG(jcr.bv->indexed, NAIVE_INDEX)) {
		free_backup_version(jcr.bv);
		return;
	}

	destor_log(DESTOR_NOTICE, "starting to index revision #%d\n", revision);

	restore_chunk_queue = sync_queue_new(100);
	restore_recipe_queue = sync_queue_new(100);

    jcr.status = JCR_STATUS_RUNNING;
	pthread_t recipe_t, read_t, search_t;

	pthread_create(&recipe_t, NULL, read_recipe_thread, NULL);

	if (destor.restore_cache[0] == RESTORE_CACHE_LRU) {
		destor_log(DESTOR_NOTICE, "restore cache is LRU");
		pthread_create(&read_t, NULL, lru_restore_thread, NULL);
	} else if (destor.restore_cache[0] == RESTORE_CACHE_OPT) {
		destor_log(DESTOR_NOTICE, "restore cache is OPT");
		pthread_create(&read_t, NULL, optimal_restore_thread, NULL);
	} else if (destor.restore_cache[0] == RESTORE_CACHE_ASM) {
		destor_log(DESTOR_NOTICE, "restore cache is ASM");
		pthread_create(&read_t, NULL, assembly_restore_thread, NULL);
	} else {
		fprintf(stderr, "Invalid restore cache.\n");
		exit(1);
	}

	pthread_create(&search_t, NULL, index_restore_data, NULL);

    do{
		join_thread_with_timeout(search_t, 5);
        fprintf(stderr, "%" PRId64 " bytes, %" PRId32 " chunks, %d files processed, %.3f read chunk time\r", 
                jcr.data_size, jcr.chunk_num, jcr.file_num, jcr.read_chunk_time / 1000000);
    }while(jcr.status == JCR_STATUS_RUNNING || jcr.status != JCR_STATUS_DONE);
    fprintf(stderr, "%" PRId64 " bytes, %" PRId32 " chunks, %d files processed, %.3f read chunk time\n", 
        jcr.data_size, jcr.chunk_num, jcr.file_num, jcr.read_chunk_time / 1000000);

	assert(sync_queue_size(restore_chunk_queue) == 0);
	assert(sync_queue_size(restore_recipe_queue) == 0);

	*p_read_recipe_time += jcr.read_recipe_time;
	*p_read_chunk_time += jcr.read_chunk_time;
	*p_file_num += jcr.file_num;
	*p_chunk_num += jcr.chunk_num;
	*p_data_size += jcr.data_size;
	*p_read_container_num += jcr.read_container_num;
	
	jcr.bv->indexed |= NAIVE_INDEX;
	update_backup_version(jcr.bv);

	free_backup_version(jcr.bv);

	destor_log(DESTOR_NOTICE, "finished to index revision #%d\n", revision);

	puts("");
}

void do_naive_index() {
	puts("==== naive index begin ====");

	double total_time = 0;
	double read_recipe_time = 0;
	double  read_chunk_time = 0;
	double throughput = 0;
	double speed_factor = 0;
	double read_recipe_throughput = 0;
	double read_chunk_throughput = 0;
	uint64_t file_num = 0;
	uint64_t chunk_num = 0;
	uint64_t data_size = 0;
	uint64_t read_container_num = 0;
	uint64_t index_size = 0;

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

	init_recipe_store();
	init_container_store();

	int revision = 0;
	while (backup_version_exists(revision)) {
		do_naive_index_revision(revision, &read_recipe_time, &read_chunk_time, &file_num, &chunk_num, &data_size, &read_container_num);
		revision++;
	}

	close_container_store();
	close_recipe_store();
	
	TIMER_END(1, total_time);

	total_time /= 1000000;
	read_recipe_time /= 1000000;
	read_chunk_time /= 1000000;
	throughput = data_size * 1000000 / (1024.0 * 1024 * total_time);
	speed_factor = data_size / (1024.0 * 1024 * read_container_num);
	read_recipe_throughput = data_size * 1000000 / read_recipe_time / 1024 / 1024;
	read_chunk_throughput = data_size * 1000000 / read_chunk_time / 1024 / 1024;
	index_size = dir_size(NAIVE_INDEX_DIR);

	printf("total time(s): %.3f\n", total_time);
	printf("number of files: %" PRId32 "\n", file_num);
	printf("number of chunks: %" PRId32"\n", chunk_num);
	printf("total size(B): %" PRId64 "\n", data_size);
	printf("throughput(MB/s): %.2f\n", throughput);
	printf("speed factor: %.2f\n", speed_factor);
	printf("read_recipe_time : %.3fs, %.2fMB/s\n", read_recipe_time, read_recipe_throughput);
	printf("read_chunk_time : %.3fs, %.2fMB/s\n", read_chunk_time, read_chunk_throughput);
	printf("index size(B): %" PRId64 "\n", index_size);
	

	puts("==== naive index end ====");

	// prepared header for csv
	// index_type,total_time,0,0,file_num,chunk_num,data_size,read_recipe_time,read_chunk_time,0,0,complete_index_size,0
	// index_type,total_time,process_data_time,create_reverse_mapping_time,file_num,chunk_num,data_size,read_recipe_time,read_chunk_time,chunk_index_size,db_reverse_mapping_size,complete_index_size,pathstore_size
	write_file(index_log_path, "naive,%.3f,0,0,%u,%u,%llu,%.3f,%.3f,0,0,%llu,0\n", 
	total_time, file_num, chunk_num, data_size, read_recipe_time, read_chunk_time, index_size);
}

void do_naive_lookup(unsigned char **keywords, size_t keywords_num) {

	double lookup_time = 0;

	destor_log(DESTOR_NOTICE, "naive lookup");

	puts("==== naive lookup start ====");

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

	uint64_t results_size = 0;
	FILE* results_file = fopen("lookup_results.out", "wb");

	std::vector<std::string> keywords_vec(keywords_num);
	for (int i = 0; i < keywords_num; i++, optind++) {
		keywords_vec[i] = (const char*)keywords[i];
	}

	if (destor.offsets_mode == NO_OFFSETS && !destor.tf_idf) {
		std::vector<std::string> results(search_keywords_in_index(keywords_vec, NAIVE_INDEX_DIR));
		results_size = results.size();

		for	(size_t i = 0; i < results.size(); i++) {
			fprintf(results_file, "found result: %s\n", results[i].c_str());
		}
	} else if (destor.offsets_mode == TERM_VECTOR_OFFSETS || destor.offsets_mode == GLOBAL_OFFSETS) {
		auto search_function_ptr = search_keywords_in_index_with_tv_offsets;
		if (destor.offsets_mode == GLOBAL_OFFSETS) {
			search_function_ptr = search_keywords_in_index_with_global_offsets;
		}
		std::map<std::string, std::vector<uint64_t>> results(search_function_ptr(keywords_vec, NAIVE_INDEX_DIR, nullptr));
		results_size = results.size();

		for (const auto& pair : results) {
			fprintf(results_file, "found result: %s\n", pair.first.c_str());
			for (uint64_t offset : pair.second) {
				fprintf(results_file, "\tin offset: %llu\n", offset);
			}
		}
	} else if (destor.tf_idf) {
		std::map<std::string, double> results(search_keywords_in_index_with_scores(keywords_vec, NAIVE_INDEX_DIR, nullptr));
		results_size = results.size();

		for (const auto& pair : results) {
			fprintf(results_file, "found result: %s, score: %.3lf\n", pair.first.c_str(), pair.second);
		}
	}

	fclose(results_file);

	TIMER_END(1, lookup_time);

	lookup_time /= 1000000;
	searcher_load_time /= 1000000;

	printf("lookup time: %.3f\n", lookup_time);
	printf("results number: %llu\n", results_size);

	puts("==== naive lookup end ====");

	
	// write_file(temp_path, "%.3f,%.3f,%.3f,%.3f,%.3f\n", searcher_load_time, lookup_time, 0.0, 0.0, lookup_time);

	// prepared header for csv
	// index_type,keywords_num,total_lookup_time,0,0,0,results,0,0,0
	// index_type,keywords_num,total_lookup_time,index_lookup_time,reverse_lookup_time,chunk_found,results,external_reverse_mapping_load_time, reverse_mapping_get_files_time, reverse_mapping_get_offsets_time
	write_file(lookup_log_path, "naive,%d,%.3f,0,0,0,%llu,0,0,0,%.3f,%.3f,%.3f\n", keywords_num, lookup_time, results_size, searcher_load_time, dtor_time, lookup_time - dtor_time);
}

static void* read_container_thread(void *arg) {
	containerid container_id;
	containerid container_count = (containerid)get_container_count();
	
	for (container_id = 0; container_id < container_count; container_id++){
		TIMER_DECLARE(1);
		TIMER_BEGIN(1);

		struct containerMeta* conMeta = retrieve_container_meta_by_id(container_id);
		if (!reindex && CHECK_FLAG(conMeta->is_indexed, DEDUP_INDEX)) {
			free_container_meta(conMeta);
			continue;
		} else {
			free_container_meta(conMeta);
		}

		struct container* con = retrieve_container_by_id(container_id); // maybe read more than one container at a time
		
		TIMER_END(1, jcr.read_chunk_time);

		sync_queue_push(restore_container_queue, con);
		jcr.read_container_num++;
	}
	
	sync_queue_term(restore_container_queue);
	return NULL;
}

void* index_containers_data(void *arg) {
	struct metaEntry* me;
	struct container* con;
	
	auto index_writer = get_index_writer(DEDUP_INDEX_DIR);

	while ((con = sync_queue_pop(restore_container_queue))) {
		TIMER_DECLARE(1);
		TIMER_BEGIN(1);
		
		GHashTableIter iter;
		gpointer key, value;

		g_hash_table_iter_init(&iter, con->meta.map);
		while(g_hash_table_iter_next(&iter, &key, &value)){
			me = (struct metaEntry*)value;
			std::string data((const char*)con->data + me->off, me->len);
			std::string fp_str(hex_H_prefix(me->fp, sizeof(me->fp)));
			std::wstring wdata = toWstring(data);
			std::wstring wfp_str = toWstring(fp_str);

			DocumentPtr document;
			if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
				document = create_multiple_names_document(get_external_reverse_mapping(fp_str), wdata);
			} else {
				document = create_document(wfp_str, wdata);
			}
			index_writer = protectedAddDocument(index_writer, 
												document, 
												DEDUP_INDEX_DIR);

			jcr.chunk_num++;
			jcr.data_size += me->len;

			if (destor.tf_idf) {
				insert_chunk_term_count(fp_str, getLastNumTokens());
			}
		}

		con->meta.is_indexed |= DEDUP_INDEX;
		containerWriteRequest *req = malloc(sizeof(req));
		req->should_write_only_meta = 1;
		req->c = con;
		write_container_async(req);

		TIMER_END(1, jcr.search_chunk_time);
	}

	index_writer->close();
    jcr.status = JCR_STATUS_DONE;
    return NULL;
}

static void* create_reverse_mapping(void *arg) {
	struct chunk* c;
	std::string filename;
	IndexWriterPtr index_writer;
	std::string recipe;
	uint64_t offset = 0;
	uint64_t tokenCount = 0;
	uint64_t fileCount = 0;
	std::map<std::string, std::vector<uint64_t>> fp_to_offset;
	std::map<std::string, uint32_t> fp_to_counter;

	if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
		index_writer = get_index_writer(LUCENE_REVERSE_MAPPING_DIR);
	} else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
			   destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
		init_external_reverse_mapping();
	} else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
		init_db_reverse_mapping();
	} else if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
		init_external_reverse_mapping();
	}
	if (destor.is_path_indirected) {
		init_pathstore();
	}
	if (destor.tf_idf) {
		init_rankstore();
	}

	while ((c = sync_queue_pop(restore_recipe_queue))) {
		
		if (CHECK_CHUNK(c, CHUNK_FILE_START)) {
			fp_to_offset.clear();
			fp_to_counter.clear();
			offset = 0;
			tokenCount = 0;
			jcr.file_num++;
			filename = std::string((const char*)c->data) + " backup version: " + std::to_string(jcr.bv->bv_num);
			if (destor.is_path_indirected) {
				unsigned int counter = add_path(filename);
				filename = std::to_string(counter);
			}
			if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
				recipe = "";
			}
		} else if (!CHECK_CHUNK(c, CHUNK_FILE_END)) {
			std::string fp = hex_H_prefix(c->fp, sizeof(c->fp));

			if (destor.offsets_mode == TERM_VECTOR_OFFSETS || destor.offsets_mode == GLOBAL_OFFSETS) {
				auto iterator = fp_to_offset.find(fp);
				if (iterator != fp_to_offset.end()) {
					iterator->second.push_back(offset);
				} else {
					fp_to_offset.emplace(fp, std::vector<uint64_t>{offset});
				}
				offset += c->size;
			}

			if (destor.tf_idf) {
				auto iterator = fp_to_counter.find(fp);
				if (iterator != fp_to_counter.end()) {
					iterator->second++;
				} else {
					fp_to_counter.emplace(fp, 1);
				}
				tokenCount += num_tokens_chunk(fp);
			}

			if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
				recipe += fp + " ";
			} else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
			   		   destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
				insert_external_reverse_mapping(fp, filename);
			} else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
				if (destor.offsets_mode == NO_OFFSETS && !destor.tf_idf) {
					insert_db_reverse_mapping(fp, filename);
				}
			} else if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
				insert_external_reverse_mapping(fp, filename);
			}
		} else {
			if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
				std::wstring wfilename = toWstring(filename);
				std::wstring wrecipe = toWstring(recipe);
				index_writer = protectedAddDocument(index_writer, 
													create_document(wfilename, wrecipe), 
													LUCENE_REVERSE_MAPPING_DIR);
			}
			if (destor.offsets_mode == TERM_VECTOR_OFFSETS || destor.offsets_mode == GLOBAL_OFFSETS) {
				for (const auto& pair : fp_to_offset) {
					insert_db_reverse_mapping_offsets(pair.first, filename, pair.second);
				}
			}
			if (destor.tf_idf) {
				for (const auto& pair : fp_to_counter) {
					insert_db_reverse_mapping_counter(pair.first, filename, pair.second);
				}

				insert_filename_term_count(filename, tokenCount);
			}
		}
		free_chunk(c);
	}

	if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
		index_writer->close();
	} else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
			   destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
		close_external_reverse_mapping(destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING);
	} else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
		close_db_reverse_mapping();
	}
	if (destor.is_path_indirected) {
		close_pathstore();
	}

	if (destor.tf_idf) {
		setGlobalFileCount(jcr.file_num);
		close_rankstore();
	}

	jcr.status = JCR_STATUS_DONE;
	return NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void do_physical_phase(double& process_data_time, uint64_t& chunk_num, uint64_t& data_size, double& read_chunk_time) {
	
	puts("==== physical phase ====");
	TIMER_DECLARE(0);
	TIMER_BEGIN(0); // process_data_time

    jcr.status = JCR_STATUS_RUNNING;
	pthread_t init_t, read_t, index_t;
	
	pthread_create(&read_t, NULL, read_container_thread, NULL);
	pthread_create(&index_t, NULL, index_containers_data, NULL);


    do{
		join_thread_with_timeout(index_t, 5);
        fprintf(stderr, "%" PRId64 " bytes, %" PRId32 " chunks, %d files processed, %.3f read chunk time\r", 
                jcr.data_size, jcr.chunk_num, jcr.file_num, jcr.read_chunk_time / 1000000);
    }while(jcr.status == JCR_STATUS_RUNNING || jcr.status != JCR_STATUS_DONE);
    fprintf(stderr, "%" PRId64 " bytes, %" PRId32 " chunks, %d files processed, %.3f read chunk time\n", 
        jcr.data_size, jcr.chunk_num, jcr.file_num, jcr.read_chunk_time / 1000000);

	assert(sync_queue_size(restore_container_queue) == 0);

	gracefully_terminate_append_thread();
	close_container_store();

	TIMER_END(0, process_data_time);

	data_size = jcr.data_size;
	chunk_num = jcr.chunk_num;
	read_chunk_time = jcr.read_chunk_time;
}

void do_logical_phase(uint64_t& file_num, double& create_reverse_mapping_time, double& read_recipe_time) {
	puts("==== logical phase ====");

	TIMER_DECLARE(0);
	TIMER_BEGIN(0);
	
	init_recipe_store();

	int revision = 0;
	while (backup_version_exists(revision)) {
		init_restore_jcr(revision, ".");

		if (!reindex && CHECK_FLAG(jcr.bv->indexed, destor.reverse_mapping_type)) {
			printf("revision %d is already indexed\n", revision);
			free_backup_version(jcr.bv);
			revision++;
			continue;
		}

		restore_recipe_queue = sync_queue_new(100);
		
		destor_log(DESTOR_NOTICE, "job id: %d", jcr.id);
		destor_log(DESTOR_NOTICE, "backup path: %s", jcr.bv->path);


		pthread_t recipe_t, reverse_mapping_t;
		pthread_create(&recipe_t, NULL, read_recipe_thread, NULL);
		pthread_create(&reverse_mapping_t, NULL, create_reverse_mapping, NULL);

		jcr.status = JCR_STATUS_RUNNING;

		do{
			join_thread_with_timeout(recipe_t, 5);
			join_thread_with_timeout(reverse_mapping_t, 5);
			fprintf(stderr, "%" PRId64 " bytes, %" PRId32 " chunks, %d files processed\r", 
					jcr.data_size, jcr.chunk_num, jcr.file_num);
		}while(jcr.status == JCR_STATUS_RUNNING || jcr.status != JCR_STATUS_DONE);
		fprintf(stderr, "%" PRId64 " bytes, %" PRId32 " chunks, %d files processed\n", 
			jcr.data_size, jcr.chunk_num, jcr.file_num);

		file_num += jcr.file_num;
		read_recipe_time += jcr.read_recipe_time;

		assert(sync_queue_size(restore_recipe_queue) == 0);

		jcr.bv->indexed |= destor.reverse_mapping_type;
		update_backup_version(jcr.bv);

		free_backup_version(jcr.bv);

		revision++;
	}

	close_recipe_store();

	TIMER_END(0, create_reverse_mapping_time);
}

void do_deduplicated_index() {

	uint64_t file_num = 0;
	uint64_t chunk_num = 0;
	uint64_t read_container_num = 0;
	uint64_t data_size = 0;
	double read_chunk_time = 0;
	double read_recipe_time = 0;
	double process_data_time = 0;
	double create_reverse_mapping_time = 0;
	double total_time = 0;

	destor_log(DESTOR_NOTICE, "deduplicated index");

	TIMER_DECLARE(1);
	TIMER_BEGIN(1); // total_time

	init_container_store();

	restore_container_queue = sync_queue_new(100);

	puts("==== index begin ====");

	if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
		do_logical_phase(file_num, create_reverse_mapping_time, read_recipe_time);
		do_physical_phase(process_data_time, chunk_num, data_size, read_chunk_time);
	} else {
		do_physical_phase(process_data_time, chunk_num, data_size, read_chunk_time);
		do_logical_phase(file_num, create_reverse_mapping_time, read_recipe_time);
	}
	
	TIMER_END(1, total_time);
	
	puts("==== index end ====");

	double throughput = data_size * 1000000 / (1024.0 * 1024 * total_time);
	double read_chunk_throughput = data_size * 1000000 / read_chunk_time / 1024 / 1024;
	double read_recipe_throughput = data_size * 1000000 / read_recipe_time / 1024 / 1024;
	double physical_phase_throughput = data_size * 1000000 / process_data_time / 1024 / 1024;
	double logical_phase_throughput = data_size * 1000000 / create_reverse_mapping_time / 1024 / 1024;
	double external_reverse_mapping_save_time = jcr.external_reverse_mapping_save_time / 1000000;

	uint64_t index_size = dir_size(DEDUP_INDEX_DIR);
	uint64_t lucene_reverse_mapping_size = destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING ?
										   dir_size(LUCENE_REVERSE_MAPPING_DIR) : 0;
	uint64_t db_reverse_mapping_size = destor.reverse_mapping_type == DB_REVERSE_MAPPING ?
										   dir_size(DB_REVERSE_MAPPING_DIR) : 0;
	uint64_t external_reverse_mapping_size = destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
											 destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING ?
										   dir_size(EXTERNAL_REVERSE_MAPPING_DIR) : 0;
	uint64_t complete_index_size;
	uint64_t pathstore_size = 0;
	uint64_t rankstore_size = 0;
	
	if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
		destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
		complete_index_size = index_size + external_reverse_mapping_size;
	} else if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
		complete_index_size = index_size + lucene_reverse_mapping_size;
	} else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
		complete_index_size = index_size + db_reverse_mapping_size;
	}
	if (destor.is_path_indirected) {
		pathstore_size = file_size(PATHSTORE_PATH);
		complete_index_size += pathstore_size;
	}
	if (destor.tf_idf) {
		rankstore_size = file_size(RANKSTORE_DB_PATH) + file_size(RANKSTORE_PATH);
		complete_index_size += rankstore_size;
	}

	total_time /= 1000000;
	read_chunk_time /= 1000000;
	read_recipe_time /= 1000000;
	process_data_time /= 1000000;
	create_reverse_mapping_time /= 1000000;

	printf("number of files: %" PRId32 "\n", file_num);
	printf("number of chunks: %" PRId32"\n", jcr.chunk_num);
	printf("total size(B): %" PRId64 "\n", data_size);
	printf("total time(s): %.3f\n", total_time);
	printf("throughput(MB/s): %.2f\n", throughput);
	printf("read_chunk_time : %.3fs, %.2fMB/s\n", read_chunk_time, read_chunk_throughput);
	printf("read_recipe_time : %.3fs, %.2fMB/s\n", read_recipe_time, read_recipe_throughput);
	printf("phyisical phase time: %.3fs, %.2fMB/s\n", process_data_time, physical_phase_throughput);
	printf("logical phase time: %.3fs, %.2fMB/s\n", create_reverse_mapping_time, logical_phase_throughput);
	printf("chunk index size: %llu\n", index_size);
	printf("lucene reverse mapping size: %llu\n", lucene_reverse_mapping_size);
	printf("external reverse mapping size: %llu\n", external_reverse_mapping_size);
	printf("db reverse mapping size: %llu\n", db_reverse_mapping_size);
	printf("pathstore size: %llu\n", pathstore_size);
	printf("complete index size: %llu\n", complete_index_size);
	printf("external reverse mapping save time: %.3f\n", external_reverse_mapping_save_time);
	
	// prepared header for csv
	// index_type,total_time,process_data_time,create_reverse_mapping_time,file_num,chunk_num,data_size,read_recipe_time,read_chunk_time,chunk_index_size,db_reverse_mapping_size,complete_index_size,pathstore_size
	write_file(index_log_path, "dedup,%.3f,%.3f,%.3f,%u,%u,%llu,%.3f,%.3f,%llu,%llu,%llu,%llu\n", 
	total_time, process_data_time, create_reverse_mapping_time, file_num, chunk_num, data_size, read_recipe_time, read_chunk_time, index_size, db_reverse_mapping_size, complete_index_size, pathstore_size);
}

void do_deduplicated_lookup(unsigned char **keywords, size_t keywords_num) {

	double lookup_time = 0, reverse_lookup_time = 0, 
	index_lookup_time = 0, time_without_emitting = 0, 
	indirection_time = 0,keywords_load_time = 0;

	destor_log(DESTOR_NOTICE, "dedup lookup");

	puts("==== deduplicated lookup start ====");

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);
	TIMER_DECLARE(2);
	TIMER_BEGIN(2);
	TIMER_DECLARE(4);
	TIMER_BEGIN(4);
	TIMER_DECLARE(5);
	TIMER_BEGIN(5);

	std::vector<std::string> keywords_vec(keywords_num);
	for (int i = 0; i < keywords_num; i++, optind++) {
		keywords_vec[i] = (const char*)keywords[i];
	}

	TIMER_END(5, keywords_load_time);


	std::vector<std::string> fingerprints;
	std::map<std::string, uint32_t> fingerprints_with_freqs;
	std::map<std::string, std::vector<uint64_t>> fingerprints_with_offsets;
	uint64_t global_file_count = 0;
	double idf = 0.0;

	if (destor.offsets_mode == NO_OFFSETS) { 
		if (destor.tf_idf) {
			fingerprints_with_freqs = search_keywords_in_index_with_termfreqs(keywords_vec, DEDUP_INDEX_DIR);
			global_file_count = getGlobalFileCount();
		} else {
			fingerprints = search_keywords_in_index(keywords_vec, DEDUP_INDEX_DIR);
		}
	} else if (destor.offsets_mode == TERM_VECTOR_OFFSETS) {
		fingerprints_with_offsets = search_keywords_in_index_with_tv_offsets(keywords_vec, DEDUP_INDEX_DIR);
	} else if (destor.offsets_mode == GLOBAL_OFFSETS) {
		fingerprints_with_offsets = search_keywords_in_index_with_global_offsets(keywords_vec, DEDUP_INDEX_DIR);
	}

	TIMER_END(2, index_lookup_time);


	printf("Found %llu chunks\n", fingerprints.size() + fingerprints_with_offsets.size() + fingerprints_with_freqs.size());
	printf("keywords load time: %.3f\n", keywords_load_time / 1000000);
	
	TIMER_DECLARE(3);
	TIMER_BEGIN(3);

	std::vector<std::string> results;
	std::map<std::string, std::vector<uint64_t>> results_with_offsets;
	std::map<std::string, uint32_t> results_with_freqs;
	std::vector<double> scores;

	std::set<std::string> results_set;
	std::set<std::string> actual_results_set;

	if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
		results = search_keywords_in_index(fingerprints, LUCENE_REVERSE_MAPPING_DIR);
	} else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING) {
		init_external_reverse_mapping();
		results_set = get_external_reverse_mapping_multiple(fingerprints, false);
		close_external_reverse_mapping(false);
	} else if (destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
		results_set = get_external_reverse_mapping_multiple(fingerprints, false);
	} else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
		init_db_reverse_mapping();
		if (destor.offsets_mode == NO_OFFSETS) { 
			if (destor.tf_idf) {
				results_with_freqs = get_db_reverse_mapping_multiple_counter(fingerprints_with_freqs);
			}
			else {
				results_set = get_db_reverse_mapping_multiple(fingerprints);
			}
		} else if (destor.offsets_mode == TERM_VECTOR_OFFSETS || destor.offsets_mode == GLOBAL_OFFSETS) {
			results_with_offsets = get_db_reverse_mapping_multiple_with_offsets(fingerprints_with_offsets); 
		}
		close_db_reverse_mapping();
	} else if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
		for (const auto& fp : fingerprints) {
			results_set.insert(fp);
		}
	}

	TIMER_END(3, reverse_lookup_time);

	if (destor.is_path_indirected) {
		TIMER_DECLARE(5);
		TIMER_BEGIN(5);
		
		if (destor.tf_idf) {
			init_rankstore();
		}
		
		init_pathstore();
		for (const auto& result : results_set) {
			if (destor.is_textual_doc_id) {
				actual_results_set.insert(get_path(std::atoi(result.data())));
			} else {
				actual_results_set.insert(get_path(*(uint32_t*)result.data()));
			}
		}
		for (auto& result : results_with_offsets) {
			if (destor.is_textual_doc_id) {
				result.first = (get_path(std::atoi(result.first.data())));
			} else {
				result.first = (get_path(*(uint32_t*)result.first.data()));
			}
		}

		if (destor.tf_idf) {
			idf = 1 + std::log(global_file_count / (results_with_freqs.size() + 1)); 
		}

		for (auto& result : results_with_freqs) {
			double weight = std::sqrt( 1 / (double)num_tokens_filename(result.first));
			weight = Similarity::decodeNorm(Similarity::encodeNorm(weight));
			scores.push_back(std::sqrt(result.second) * weight * idf);
		}
		for (auto& result : results_with_freqs) {
			if (destor.is_textual_doc_id) {
				result.first = (get_path(std::atoi(result.first.data())));
			} else {
				result.first = (get_path(*(uint32_t*)result.first.data()));
			}
		}
		close_pathstore();

		results_set.clear();

		TIMER_END(5, indirection_time);
	}

	TIMER_END(4, time_without_emitting);

	FILE* results_file = fopen("dedup_lookup_results.out", "wb");


	for	(const std::string& result : results) {
		fprintf(results_file, "found result: %s\n", result.c_str());
	}
	for	(const std::string& result : results_set) {
		fprintf(results_file, "found result: %s\n", result.c_str());
	}
	for	(const std::string& result : actual_results_set) {
		fprintf(results_file, "found result: %s\n", result.c_str());
	}
	for (const auto& pair : results_with_offsets) {

		fprintf(results_file, "found result: %s\n", pair.first.c_str());
		for (uint64_t offset : pair.second) {
			fprintf(results_file, "\tin offset: %llu\n", offset);
		}
	}

	size_t i = 0;
	for (const auto& pair : results_with_freqs) {
		fprintf(results_file, "found result: %s, score: %.3lf\n", pair.first.c_str(), scores[i]);
		i++;
	}

	fclose(results_file);

	if (destor.tf_idf) {
		close_rankstore();
	}

	TIMER_END(1, lookup_time);

	double external_reverse_mapping_load_time = jcr.external_reverse_mapping_load_time / 1000000;

	lookup_time /= 1000000;
	index_lookup_time /= 1000000;
	reverse_lookup_time /= 1000000;
	time_without_emitting /= 1000000;
	indirection_time /= 1000000;
	searcher_load_time /= 1000000;
	reverse_lookup_time -= external_reverse_mapping_load_time;

	printf("lookup time: %.3f\n", lookup_time);
	printf("index lookup time: %.3f\n", index_lookup_time);
	printf("indirection time: %.3f\n", indirection_time);
	printf("external reverse mapping load time: %.3f\n", external_reverse_mapping_load_time);
	printf("external reverse mapping mini index load time: %.3f\n", jcr.reverse_mapping_get_offsets_time);
	printf("external reverse mapping files lookup time: %.3f\n", jcr.reverse_mapping_get_files_time);
	printf("reverse lookup time: %.3f\n", reverse_lookup_time);
	printf("time without emitting: %.3f\n", time_without_emitting);
	printf("found chunks number: %llu\n", fingerprints.size());
	printf("results number: %llu\n", results.size() + results_set.size() + actual_results_set.size() + results_with_offsets.size() + results_with_freqs.size());

	puts("==== deduplicated lookup end ====");

	// write_file(temp_path, "%.3f,%.3f,%.3f,%.3f,%.3f\n", searcher_load_time, index_lookup_time, 
	// 								reverse_lookup_time, indirection_time, lookup_time);

	// prepared header for csv
	// index_type,keywords_num,total_lookup_time,index_lookup_time,reverse_lookup_time,chunk_found,results,external_reverse_mapping_load_time, reverse_mapping_get_files_time, indirection_time
	write_file(lookup_log_path, "dedup,%d,%.3f,%.3f,%.3f,%llu,%llu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n", 
		keywords_num, lookup_time, index_lookup_time, reverse_lookup_time, fingerprints.size(), results.size() + results_set.size() + actual_results_set.size() + results_with_offsets.size() + results_with_freqs.size(), external_reverse_mapping_load_time, jcr.reverse_mapping_get_files_time, indirection_time, searcher_load_time, dtor_time, lookup_time - dtor_time);
}

struct IndexLookup {
	std::vector<std::string> keywords;
	SyncQueue* queue;
};

void index_lookup_thread(void* params) {
	struct IndexLookup* indexLookup = (struct IndexLookup*)params;
	search_keywords_in_index(indexLookup->keywords, 
							 DEDUP_INDEX_DIR,
							 indexLookup->queue);
	sync_queue_push(indexLookup->queue, nullptr);
}

struct ReverseLookup {
	SyncQueue* in_queue;
	SyncQueue* out_queue;
	uint64_t* pinput_num;
	double* work_time;
};

void reverse_lookup_fetch_thread(void* params) {
	struct ReverseLookup* reverseParams = params;
	std::string* fp = sync_queue_pop(reverseParams->in_queue);
	TIMER_DECLARE(1);

	while (fp) {
		TIMER_BEGIN(1);
		get_db_reverse_mapping_fetch(*fp, reverseParams->out_queue);
		delete fp;
		(*(reverseParams->pinput_num))++;
		TIMER_END(1, *(reverseParams->work_time));
		fp = sync_queue_pop(reverseParams->in_queue);
	}

	sync_queue_push(reverseParams->out_queue, nullptr);
}

void reverse_lookup_iter_thread(void* params) {
	struct ReverseLookup* reverseParams = params;
	TIMER_DECLARE(1);

	DBT* data = sync_queue_pop(reverseParams->in_queue);
	while (data) {
		TIMER_BEGIN(1);
		get_db_reverse_mapping_iterate(data, reverseParams->out_queue);
		free(data->data);
		delete data;
		TIMER_END(1, *(reverseParams->work_time));
		data = sync_queue_pop(reverseParams->in_queue);
	}

	sync_queue_push(reverseParams->out_queue, nullptr);
}

void path_lookup_thread(void* params) {
	struct ReverseLookup* pathParams = params;
	TIMER_DECLARE(1);

	std::string* fileId = sync_queue_pop(pathParams->in_queue);
	while (fileId) {
		TIMER_BEGIN(1);
		unsigned int res = *(unsigned int*)fileId->data();
		std::string filePath = get_path(res);
		sync_queue_push(pathParams->out_queue, new std::string(filePath));
		delete fileId;
		TIMER_END(1, *(pathParams->work_time));
		fileId = sync_queue_pop(pathParams->in_queue);
	}

	sync_queue_push(pathParams->out_queue, nullptr);
}

void do_deduplicated_lookup_threaded(unsigned char **keywords, size_t keywords_num, sds keywords_file) {
	double lookup_time = 0, reverse_lookup_time = 0, 
	index_lookup_time = 0, time_without_emitting = 0, indirection_time = 0, 
	external_reverse_mapping_load_time = 0, fetch_time = 0, iterate_time = 0, 
	path_time = 0, first_result_time = 0;
	uint64_t fingerprints_size = 0, results_size = 0;

	if (destor.reverse_mapping_type != DB_REVERSE_MAPPING) {
		puts("*******UNSUPPORTED REVERSE MAPPING TYPE*******");
	}

	destor_log(DESTOR_NOTICE, "dedup lookup");

	puts("==== deduplicated lookup start ====");

	TIMER_DECLARE(1);
	TIMER_DECLARE(2);
	TIMER_BEGIN(1);
	TIMER_BEGIN(2);

	init_db_reverse_mapping();
	init_pathstore();

	std::vector<std::string> keywords_vec(keywords_num);
	for (int i = 0; i < keywords_num; i++, optind++) {
		keywords_vec[i] = (const char*)keywords[i];
	}
	
	pthread_t chunk_index_lookup_t, reverse_mapping_fetch_t, reverse_mapping_iter_t, indirected_path_lookup_t;
	SyncQueue *chunk_index_lookup_queue, *reverse_mapping_fetch_queue, *reverse_mapping_iter_queue, *indirected_path_queue, *output_queue;
	chunk_index_lookup_queue = sync_queue_new(100);
	reverse_mapping_fetch_queue = sync_queue_new(100);
	output_queue = reverse_mapping_iter_queue = sync_queue_new(100);
	if (destor.is_path_indirected) {
		output_queue = indirected_path_queue = sync_queue_new(100);
	}

	IndexLookup lookupParams{std::move(keywords_vec), chunk_index_lookup_queue};
	ReverseLookup reverseFetchParams{chunk_index_lookup_queue, reverse_mapping_fetch_queue, &fingerprints_size, &fetch_time};
	ReverseLookup reverseIterParams{reverse_mapping_fetch_queue, reverse_mapping_iter_queue, nullptr, &iterate_time};
	ReverseLookup pathParams{reverse_mapping_iter_queue, indirected_path_queue, nullptr, &path_time};
	pthread_create(&chunk_index_lookup_t, NULL, index_lookup_thread, &lookupParams);
	pthread_create(&reverse_mapping_fetch_t, NULL, reverse_lookup_fetch_thread, &reverseFetchParams);
	pthread_create(&reverse_mapping_iter_t, NULL, reverse_lookup_iter_thread, &reverseIterParams);
	if (destor.is_path_indirected) {
		pthread_create(&indirected_path_lookup_t, NULL, path_lookup_thread, &pathParams);
	}

	FILE* results_file = fopen("dedup_lookup_results.out", "wb");
	std::set<std::string> results;

	std::string* path = sync_queue_pop(output_queue);
	TIMER_END(2, first_result_time);
	while (path) {
		auto ret = results.insert(*path);
		if (ret.second) { // actually inserted to set
			results_size++;
			fprintf(results_file, "found result: %s\n", path->c_str());
		}
		delete path;
		path = sync_queue_pop(output_queue);
	}

	sync_queue_free(chunk_index_lookup_queue, nullptr);
	sync_queue_free(reverse_mapping_fetch_queue, nullptr);
	sync_queue_free(reverse_mapping_iter_queue, nullptr);
	if (destor.is_path_indirected) {
		sync_queue_free(indirected_path_queue, nullptr);
	}
	
	close_pathstore();
	close_db_reverse_mapping();

	fclose(results_file);
	TIMER_END(1, lookup_time);

	fetch_time /= 1000000;
	iterate_time /= 1000000;
	path_time /= 1000000;
	first_result_time /= 1000000;
	lookup_time /= 1000000;

	printf("lookup time: %.3f\n", lookup_time);
	printf("reverse fetch work time: %.3f\n", fetch_time);
	printf("reverse iterate work time: %.3f\n", iterate_time);
	printf("path lookup work time: %.3f\n", path_time);
	printf("time to first result: %.3f\n", first_result_time);
	printf("found chunks number: %llu\n", fingerprints_size);
	printf("results number: %llu\n", results_size);

	puts("==== deduplicated lookup end ====");

	// prepared header for csv
	// index_type,keywords_num,total_lookup_time,index_lookup_time,reverse_lookup_time,chunk_found,results,external_reverse_mapping_load_time, reverse_mapping_get_files_time, reverse_mapping_get_offsets_time
	write_file(lookup_log_path, "dedup,%d,%.3f,%.3f,%.3f,%llu,%llu,%.3f,%.3f,%.3f,%.3f,%.3f\n", 
		keywords_num, lookup_time, index_lookup_time, reverse_lookup_time, fingerprints_size, results_size, external_reverse_mapping_load_time, jcr.reverse_mapping_get_files_time, jcr.reverse_mapping_get_offsets_time, dtor_time, lookup_time - dtor_time);
}