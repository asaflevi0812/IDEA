/**
 * @file dedup_index_filter_phase.c
 * @author Asaf Levi
 * @brief ADDED FOR IDEA
 *        replaces the filter phase of destor 
 *        for inline indexing. It operates in
 *        as a similar manner but also enqueues
 *        file recipes when they are created
 *        straight into chunk-to-file construction.
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "destor.h"
#include "jcr.h"
#include "storage/containerstore.h"
#include "recipe/recipestore.h"
#include "rewrite_phase.h"
#include "backup.h"
#include "index/index.h"
#include "clucene-wrapper/libclucene-wrapper.h"
#include "utils/utils.h"
#include "reverse_mapping/external_reverse_mapping.h"
#include "reverse_mapping/berkeley_db_reverse_mapping.h"
#include "reverse_mapping/pathstore.h"

extern pthread_t filter_t;
pthread_t index_t;
static int64_t chunk_num;
static SyncQueue* index_queue;

extern struct storage_buffer{ // DedupSearch
	/* accessed in dedup phase */
	struct container *container_buffer;
	/* In order to facilitate sampling in container,
	 * we keep a list for chunks in container buffer. */
	GSequence *chunks;
} storage_buffer;

extern struct index_lock { // DedupSearch
	/* g_mutex_init() is unnecessary if in static storage. */
	GMutex mutex;
	GCond cond; // index buffer is not full
	int wait_threshold;
} index_lock;

/*
 * When a container buffer is full, we push it into container_queue.
 */
static void* dedup_index_filter_thread(void *arg) {
    int enable_rewrite = 1;
    struct fileRecipeMeta* r = NULL;

    while (1) {
        struct chunk* c = sync_queue_pop(rewrite_queue);

        if (c == NULL)
            /* backup job finish */
            break;

        /* reconstruct a segment */
        struct segment* s = new_segment();

        /* segment head */
        assert(CHECK_CHUNK(c, CHUNK_SEGMENT_START));
        free_chunk(c);

        c = sync_queue_pop(rewrite_queue);
        while (!(CHECK_CHUNK(c, CHUNK_SEGMENT_END))) {
            g_sequence_append(s->chunks, c);
            if (!CHECK_CHUNK(c, CHUNK_FILE_START)
                    && !CHECK_CHUNK(c, CHUNK_FILE_END))
                s->chunk_num++;

            c = sync_queue_pop(rewrite_queue);
        }
        free_chunk(c);

        /* For self-references in a segment.
         * If we find an early copy of the chunk in this segment has been rewritten,
         * the rewrite request for it will be denied to avoid repeat rewriting. */
        GHashTable *recently_rewritten_chunks = g_hash_table_new_full(g_int64_hash,
        		g_fingerprint_equal, NULL, free_chunk);
        GHashTable *recently_unique_chunks = g_hash_table_new_full(g_int64_hash,
        			g_fingerprint_equal, NULL, free_chunk);

        pthread_mutex_lock((pthread_mutex_t*)&index_lock.mutex); // DedupSearch

        TIMER_DECLARE(1);
        TIMER_BEGIN(1);
        /* This function will check the fragmented chunks
         * that would be rewritten later.
         * If we find an early copy of the chunk in earlier segments,
         * has been rewritten,
         * the rewrite request for it will be denied. */
        index_check_buffer(s);

        
    	GSequenceIter *iter = g_sequence_get_begin_iter(s->chunks);
    	GSequenceIter *end = g_sequence_get_end_iter(s->chunks);
        for (; iter != end; iter = g_sequence_iter_next(iter)) {
            c = g_sequence_get(iter);

    		if (CHECK_CHUNK(c, CHUNK_FILE_START) || CHECK_CHUNK(c, CHUNK_FILE_END))
    			continue;

            VERBOSE("Filter phase: %dth chunk in %s container %lld", chunk_num,
                    CHECK_CHUNK(c, CHUNK_OUT_OF_ORDER) ? "out-of-order" : "", c->id);

            /* Cache-Aware Filter */
            if (destor.rewrite_enable_cache_aware && restore_aware_contains(c->id)) {
                assert(c->id != TEMPORARY_ID);
                VERBOSE("Filter phase: %dth chunk is cached", chunk_num);
                SET_CHUNK(c, CHUNK_IN_CACHE);
            }

            /* A cfl-switch for rewriting out-of-order chunks. */
            if (destor.rewrite_enable_cfl_switch) {
                double cfl = restore_aware_get_cfl();
                if (enable_rewrite && cfl > destor.rewrite_cfl_require) {
                    VERBOSE("Filter phase: Turn OFF the (out-of-order) rewrite switch of %.3f",
                            cfl);
                    enable_rewrite = 0;
                } else if (!enable_rewrite && cfl < destor.rewrite_cfl_require) {
                    VERBOSE("Filter phase: Turn ON the (out-of-order) rewrite switch of %.3f",
                            cfl);
                    enable_rewrite = 1;
                }
            }

            if(CHECK_CHUNK(c, CHUNK_DUPLICATE) && c->id == TEMPORARY_ID){
            	struct chunk* ruc = g_hash_table_lookup(recently_unique_chunks, &c->fp);
            	assert(ruc);
            	c->id = ruc->id;
            }
            struct chunk* rwc = g_hash_table_lookup(recently_rewritten_chunks, &c->fp);
            if(rwc){
            	c->id = rwc->id;
            	SET_CHUNK(c, CHUNK_REWRITE_DENIED);
            }

            /* A fragmented chunk will be denied if it has been rewritten recently */
            if (!CHECK_CHUNK(c, CHUNK_DUPLICATE) 
					|| (!CHECK_CHUNK(c, CHUNK_REWRITE_DENIED)
            		&& (CHECK_CHUNK(c, CHUNK_SPARSE)
                    || (enable_rewrite && CHECK_CHUNK(c, CHUNK_OUT_OF_ORDER)
                        && !CHECK_CHUNK(c, CHUNK_IN_CACHE))))) {
                /*
                 * If the chunk is unique, or be fragmented and not denied,
                 * we write it to a container.
                 * Fragmented indicates: sparse, or out of order and not in cache,
                 */
                if (storage_buffer.container_buffer == NULL){
                	storage_buffer.container_buffer = create_container();
                	if(destor.index_category[1] == INDEX_CATEGORY_PHYSICAL_LOCALITY)
                		storage_buffer.chunks = g_sequence_new(free_chunk);
                }

                if (container_overflow(storage_buffer.container_buffer, c->size)) {

                    if(destor.index_category[1] == INDEX_CATEGORY_PHYSICAL_LOCALITY){
                        /*
                         * TO-DO
                         * Update_index for physical locality
                         */
                        GHashTable *features = sampling(storage_buffer.chunks,
                        		g_sequence_get_length(storage_buffer.chunks));
                        index_update(features, get_container_id(storage_buffer.container_buffer));
                        g_hash_table_destroy(features);
                        g_sequence_free(storage_buffer.chunks);
                        storage_buffer.chunks = g_sequence_new(free_chunk);
                    }
                    TIMER_END(1, jcr.filter_time);
                    struct containerWriteRequest *req = malloc(sizeof(struct containerWriteRequest));
                    req->should_write_only_meta = 0;
                    req->c = storage_buffer.container_buffer;
                    write_container_async(req);
                    TIMER_BEGIN(1);
                    storage_buffer.container_buffer = create_container();
                    storage_buffer.container_buffer->meta.is_indexed |= DEDUP_INDEX;
                }

                if(add_chunk_to_container(storage_buffer.container_buffer, c)){
                    c->flag |= CHUNK_ADDED_TO_CONTAINERSTORE;

                	struct chunk* wc = new_chunk(0);
                	memcpy(&wc->fp, &c->fp, sizeof(fingerprint));
                	wc->id = c->id;
                	if (!CHECK_CHUNK(c, CHUNK_DUPLICATE)) {
                		jcr.unique_chunk_num++;
                		jcr.unique_data_size += c->size;
                		g_hash_table_insert(recently_unique_chunks, &wc->fp, wc);
                    	VERBOSE("Filter phase: %dth chunk is recently unique, size %d", chunk_num,
                    			g_hash_table_size(recently_unique_chunks));
                	} else {
                		jcr.rewritten_chunk_num++;
                		jcr.rewritten_chunk_size += c->size;
                		g_hash_table_insert(recently_rewritten_chunks, &wc->fp, wc);
                	}

                	if(destor.index_category[1] == INDEX_CATEGORY_PHYSICAL_LOCALITY){
                		struct chunk* ck = new_chunk(0);
                		memcpy(&ck->fp, &c->fp, sizeof(fingerprint));
                		g_sequence_append(storage_buffer.chunks, ck);
                	}

                	VERBOSE("Filter phase: Write %dth chunk to container %lld",
                			chunk_num, c->id);
                }else{
                	VERBOSE("Filter phase: container %lld already has this chunk", c->id);
            		assert(destor.index_category[0] != INDEX_CATEGORY_EXACT
            				|| destor.rewrite_algorithm[0]!=REWRITE_NO);
                }

            }else{
                if(CHECK_CHUNK(c, CHUNK_REWRITE_DENIED)){
                    VERBOSE("Filter phase: %lldth fragmented chunk is denied", chunk_num);
                }else if (CHECK_CHUNK(c, CHUNK_OUT_OF_ORDER)) {
                    VERBOSE("Filter phase: %lldth chunk in out-of-order container %lld is already cached",
                            chunk_num, c->id);
                }
            }

            assert(c->id != TEMPORARY_ID);

            /* Collect historical information. */
            har_monitor_update(c->id, c->size);

            /* Restore-aware */
            restore_aware_update(c->id, c->size);

            chunk_num++;
        }


        int full = index_update_buffer(s);

        /* Write a SEGMENT_BEGIN */
        segmentid sid = append_segment_flag(jcr.bv, CHUNK_SEGMENT_START, s->chunk_num);


    	iter = g_sequence_get_begin_iter(s->chunks);
    	end = g_sequence_get_end_iter(s->chunks);
        for (; iter != end; iter = g_sequence_iter_next(iter)) {
            c = g_sequence_get(iter);

        	if(r == NULL){
        		assert(CHECK_CHUNK(c,CHUNK_FILE_START));
        		r = new_file_recipe_meta(c->data);
        	}else if(!CHECK_CHUNK(c,CHUNK_FILE_END)){
        		struct chunkPointer cp;
        		cp.id = c->id;
        		assert(cp.id>=0);
        		memcpy(&cp.fp, &c->fp, sizeof(fingerprint));
        		cp.size = c->size;
        		append_n_chunk_pointers(jcr.bv, &cp ,1);
        		r->chunknum++;
        		r->filesize += c->size;

    	    	jcr.chunk_num++;
	    	    jcr.data_size += c->size;
        	}else{
        		assert(CHECK_CHUNK(c,CHUNK_FILE_END));
        		append_file_recipe_meta(jcr.bv, r);
        		free_file_recipe_meta(r);
        		r = NULL;

	            jcr.file_num++;
        	}
        }

       	/* Write a SEGMENT_END */
       	append_segment_flag(jcr.bv, CHUNK_SEGMENT_END, 0);

        if(destor.index_category[1] == INDEX_CATEGORY_LOGICAL_LOCALITY){
             /*
              * TO-DO
              * Update_index for logical locality
              */
            s->features = sampling(s->chunks, s->chunk_num);
         	if(destor.index_category[0] == INDEX_CATEGORY_EXACT){
         		/*
         		 * For exact deduplication,
         		 * unique fingerprints are inserted.
         		 */
         		VERBOSE("Filter phase: add %d unique fingerprints to %d features",
         				g_hash_table_size(recently_unique_chunks),
         				g_hash_table_size(s->features));
         		GHashTableIter iter;
         		gpointer key, value;
         		g_hash_table_iter_init(&iter, recently_unique_chunks);
         		while(g_hash_table_iter_next(&iter, &key, &value)){
         			struct chunk* uc = value;
         			fingerprint *ft = malloc(sizeof(fingerprint));
         			memcpy(ft, &uc->fp, sizeof(fingerprint));
         			g_hash_table_insert(s->features, ft, NULL);
         		}

         		/*
         		 * OPTION:
         		 * 	It is still an open problem whether we need to update
         		 * 	rewritten fingerprints.
         		 * 	It would increase index update overhead, while the benefit
         		 * 	remains unclear.
         		 * 	More experiments are required.
         		 */
         		VERBOSE("Filter phase: add %d rewritten fingerprints to %d features",
         				g_hash_table_size(recently_rewritten_chunks),
         				g_hash_table_size(s->features));
         		g_hash_table_iter_init(&iter, recently_rewritten_chunks);
         		while(g_hash_table_iter_next(&iter, &key, &value)){
         			struct chunk* uc = value;
         			fingerprint *ft = malloc(sizeof(fingerprint));
         			memcpy(ft, &uc->fp, sizeof(fingerprint));
         			g_hash_table_insert(s->features, ft, NULL);
         		}
         	}
         	index_update(s->features, sid);
        }

        sync_queue_push(index_queue, s);

        if(index_lock.wait_threshold > 0 && full == 0){
        	pthread_cond_broadcast((pthread_cond_t*)&index_lock.cond); // DedupSearch
        }
        TIMER_END(1, jcr.filter_time);
        pthread_mutex_unlock((pthread_mutex_t*)&index_lock.mutex); // DedupSearch

        g_hash_table_destroy(recently_rewritten_chunks);
        g_hash_table_destroy(recently_unique_chunks);

    }

    if (storage_buffer.container_buffer
    		&& !container_empty(storage_buffer.container_buffer)){
        if(destor.index_category[1] == INDEX_CATEGORY_PHYSICAL_LOCALITY){
            /*
             * TO-DO
             * Update_index for physical locality
             */
        	GHashTable *features = sampling(storage_buffer.chunks,
        			g_sequence_get_length(storage_buffer.chunks));
        	index_update(features, get_container_id(storage_buffer.container_buffer));
        	g_hash_table_destroy(features);
        	g_sequence_free(storage_buffer.chunks);
        }
        struct containerWriteRequest *req = malloc(sizeof(struct containerWriteRequest));
        req->should_write_only_meta = 0;
        req->c = storage_buffer.container_buffer;
        write_container_async(req);
    }

    sync_queue_push(index_queue, NULL);
    return NULL;
}

static void* index_phase_thread(void* arg) {

    struct segment* s = NULL;
    struct chunk* c = NULL;
    auto dedup_index_writer = get_index_writer(DEDUP_INDEX_DIR);
    std::string filename;
    IndexWriterPtr reverse_index_writer;
    std::string recipe;

    if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
        reverse_index_writer = get_index_writer(LUCENE_REVERSE_MAPPING_DIR);
    } else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING || 
               destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING ||
               destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
        init_external_reverse_mapping();
    } else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
        init_db_reverse_mapping();
    }
    if (destor.is_path_indirected) {
        init_pathstore();
    }

    while ((s = sync_queue_pop(index_queue)) != NULL) {
        GSequenceIter *iter = g_sequence_get_begin_iter(s->chunks);
    	GSequenceIter *end = g_sequence_get_end_iter(s->chunks);
        for (; iter != end; iter = g_sequence_iter_next(iter)) {
            c = g_sequence_get(iter);
            if (CHECK_CHUNK(c, CHUNK_FILE_START)) {                
                filename = std::string((const char*)c->data) + " backup version: " + std::to_string(jcr.bv->bv_num);
                if (destor.is_path_indirected) {
                    unsigned int counter = add_path(filename);
                    filename = std::to_string(counter);
                }
                if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
                    // create reverse mapping recipe
                    recipe = "";
                }
            } else if (CHECK_CHUNK(c, CHUNK_FILE_END)) {
                // add recipe to reverse mapping
                if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
                    std::wstring wfilename = toWstring(filename);
                    std::wstring wrecipe = toWstring(recipe);
                    reverse_index_writer = protectedAddDocument(reverse_index_writer, 
                                                                create_document(wfilename, wrecipe),  
                                                                LUCENE_REVERSE_MAPPING_DIR);
                }
            } else {
                // add fingerprint->file to reverse mapping recipe
                if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
                    recipe += hex_H_prefix(c->fp, sizeof(c->fp)) + " ";
                } else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
                           destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING ||
                           destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
                    insert_external_reverse_mapping(hex_H_prefix(c->fp, sizeof(c->fp)), filename);
                } else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
                    insert_db_reverse_mapping(hex_H_prefix(c->fp, sizeof(c->fp)), filename);
                }
            }

            if (CHECK_CHUNK(c, CHUNK_ADDED_TO_CONTAINERSTORE)) {
                std::string data((const char*)c->data, c->size);
                std::string fp_str(hex_H_prefix(c->fp, sizeof(c->fp)));
                std::wstring wdata = toWstring(data);
                std::wstring wfp_str = toWstring(fp_str);
                if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
                    dedup_index_writer = protectedAddDocument(dedup_index_writer,
                                                              create_fingerprint_document(wfp_str, wdata),
                                                              DEDUP_INDEX_DIR);
                } else {
                    dedup_index_writer = protectedAddDocument(dedup_index_writer,
                                                            create_document(wfp_str, wdata), 
                                                            DEDUP_INDEX_DIR);
                }
            }
        }
        free_segment(s);
    }
    
    dedup_index_writer->close();
    if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
        reverse_index_writer->close();
    } else if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
               destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
        close_external_reverse_mapping(destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING);
    } else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
        close_db_reverse_mapping();
    } else if (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING) {
        auto index_writer = get_index_writer(DEDUP_INDEX_DIR);
        for (const auto& key : get_external_reverse_mapping_keys()) {
            std::vector<std::wstring> names = get_external_reverse_mapping(key.first);
            index_writer->updateDocument(newLucene<Term>(L"fp", toWstring(key.first)), 
                                         create_names_only_document(names),
                                         get_analyzer());
        }
        index_writer->close();
    }
    if (destor.is_path_indirected) {
        close_pathstore();
    }

    /* All files done */
    jcr.bv->indexed |= destor.reverse_mapping_type;
    jcr.status = JCR_STATUS_DONE;
    return NULL;
}

void start_dedup_index_filter_phase() {

	storage_buffer.container_buffer = NULL;

    init_restore_aware();

    index_queue = sync_queue_new(10);

    pthread_create(&filter_t, NULL, dedup_index_filter_thread, NULL);
    pthread_create(&index_t, NULL, index_phase_thread, NULL);
}

void stop_dedup_index_filter_phase() {
    close_har();
	NOTICE("filter phase stops successfully!");

}
