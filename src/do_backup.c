#include "destor.h"
#include "jcr.h"
#include "utils/sync_queue.h"
#include "utils/utils.h"
#include "index/index.h"
#include "backup.h"
#include "storage/containerstore.h"

/* defined in index.c */
extern struct {
	/* Requests to the key-value store */
	int lookup_requests;
	int update_requests;
	int lookup_requests_for_unique;
	/* Overheads of prefetching module */
	int read_prefetching_units;
}index_overhead;

extern pthread_t filter_t;
extern pthread_t index_t;

// DedupSearch
/* Output of read phase. */
SyncQueue* read_queue;
/* Output of chunk phase. */
SyncQueue* chunk_queue;
/* Output of hash phase. */
SyncQueue* hash_queue;
/* Output of trace phase. */
SyncQueue* trace_queue;
/* Output of dedup phase */
SyncQueue* dedup_queue;
/* Output of rewrite phase. */
SyncQueue* rewrite_queue;

void do_backup(char *path, int index_mode) {

	printf("backup started with index mode %d\n", index_mode);

	init_recipe_store();
	init_container_store();
	init_index();

	init_backup_jcr(path);

	puts("==== backup begin ====");

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

    time_t start = time(NULL);
	if (destor.simulation_level == SIMULATION_ALL) {
		start_read_trace_phase();
	} else {
		/// IDEA added these options (INDEX_MODE_NAIVE and INDEX_MODE_IDEA), 
		/// replacing phases for IDEA's inline indexing
		if (index_mode == INDEX_MODE_NAIVE) {
			start_read_index_phase();
		} else {
			start_read_phase();
		}
		start_chunk_phase();
		start_hash_phase();
	}
	start_dedup_phase();
	start_rewrite_phase();
	if (index_mode == INDEX_MODE_DEDUP) {
		start_dedup_index_filter_phase();
	} else {
		start_filter_phase();
	}

    do{
		if (index_mode == INDEX_MODE_DEDUP) {
			join_thread_with_timeout(index_t, 5);
		} else {
			join_thread_with_timeout(filter_t, 5);
		}
        /*time_t now = time(NULL);*/
        fprintf(stderr,"job %" PRId32 ", %" PRId64 " bytes, %" PRId32 " chunks, %d files processed\r", 
                jcr.id, jcr.data_size, jcr.chunk_num, jcr.file_num);
    }while(jcr.status == JCR_STATUS_RUNNING || jcr.status != JCR_STATUS_DONE);
    fprintf(stderr,"job %" PRId32 ", %" PRId64 " bytes, %" PRId32 " chunks, %d files processed\n", 
        jcr.id, jcr.data_size, jcr.chunk_num, jcr.file_num);

	if (destor.simulation_level == SIMULATION_ALL) {
		stop_read_trace_phase();
	} else {
		if (index_mode == INDEX_MODE_NAIVE) {
			stop_read_index_phase();
		} else {
			stop_read_phase();
		}
		stop_chunk_phase();
		stop_hash_phase();
	}
	stop_dedup_phase();
	stop_rewrite_phase();
	if (index_mode == INDEX_MODE_DEDUP) {
		stop_dedup_index_filter_phase();
	} else {
		stop_filter_phase();
	}

	TIMER_END(1, jcr.total_time);

	close_index();
	close_container_store();
	close_recipe_store();

	update_backup_version(jcr.bv);

	free_backup_version(jcr.bv);

	puts("==== backup end ====");

	uint64_t total_index_size = 0, reverse_mapping_size = 0, 
	external_reverse_mapping_size = 0, lucene_reverse_mapping_size = 0, 
	db_reverse_mapping_size = 0,
	dedup_index_size = 0;
	if (index_mode == INDEX_MODE_NAIVE) {
		total_index_size = dir_size(NAIVE_INDEX_DIR);
	} else if (index_mode == INDEX_MODE_DEDUP) {
		if (destor.reverse_mapping_type == EXTERNAL_REVERSE_MAPPING ||
			destor.reverse_mapping_type == READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING) {
			reverse_mapping_size = external_reverse_mapping_size = dir_size(EXTERNAL_REVERSE_MAPPING_DIR);
		} else if (destor.reverse_mapping_type == LUCENE_REVERSE_MAPPING) {
			reverse_mapping_size = lucene_reverse_mapping_size = dir_size(LUCENE_REVERSE_MAPPING_DIR);
		} else if (destor.reverse_mapping_type == DB_REVERSE_MAPPING) {
			reverse_mapping_size = db_reverse_mapping_size = dir_size(DB_REVERSE_MAPPING_DIR);
		}
		dedup_index_size = dir_size(DEDUP_INDEX_DIR);
		total_index_size = reverse_mapping_size + dedup_index_size;
	}

	printf("job id: %" PRId32 "\n", jcr.id);
	printf("backup path: %s\n", jcr.path);
	printf("number of files: %d\n", jcr.file_num);
	printf("number of chunks: %" PRId32 " (%" PRId64 " bytes on average)\n", jcr.chunk_num,
			jcr.data_size / jcr.chunk_num);
	printf("number of unique chunks: %" PRId32 "\n", jcr.unique_chunk_num);
	printf("total size(B): %" PRId64 "\n", jcr.data_size);
	printf("stored data size(B): %" PRId64 "\n",
			jcr.unique_data_size + jcr.rewritten_chunk_size);
	printf("deduplication ratio: %.4f, %.4f\n",
			jcr.data_size != 0 ?
					(jcr.data_size - jcr.unique_data_size
							- jcr.rewritten_chunk_size)
							/ (double) (jcr.data_size) :
					0,
			jcr.data_size
					/ (double) (jcr.unique_data_size + jcr.rewritten_chunk_size));
	printf("total time(s): %.3f\n", jcr.total_time / 1000000);
	printf("throughput(MB/s): %.2f\n",
			(double) jcr.data_size * 1000000 / (1024 * 1024 * jcr.total_time));
	printf("number of zero chunks: %" PRId32 "\n", jcr.zero_chunk_num);
	printf("size of zero chunks: %" PRId64 "\n", jcr.zero_chunk_size);
	printf("number of rewritten chunks: %" PRId32 "\n", jcr.rewritten_chunk_num);
	printf("size of rewritten chunks: %" PRId64 "\n", jcr.rewritten_chunk_size);
	printf("rewritten rate in size: %.3f\n",
			jcr.rewritten_chunk_size / (double) jcr.data_size);

	destor.data_size += jcr.data_size;
	destor.stored_data_size += jcr.unique_data_size + jcr.rewritten_chunk_size;

	destor.chunk_num += jcr.chunk_num;
	destor.stored_chunk_num += jcr.unique_chunk_num + jcr.rewritten_chunk_num;
	destor.zero_chunk_num += jcr.zero_chunk_num;
	destor.zero_chunk_size += jcr.zero_chunk_size;
	destor.rewritten_chunk_num += jcr.rewritten_chunk_num;
	destor.rewritten_chunk_size += jcr.rewritten_chunk_size;

	printf("read_time : %.3fs, %.2fMB/s\n", jcr.read_time / 1000000,
			jcr.data_size * 1000000 / jcr.read_time / 1024 / 1024);
	printf("chunk_time : %.3fs, %.2fMB/s\n", jcr.chunk_time / 1000000,
			jcr.data_size * 1000000 / jcr.chunk_time / 1024 / 1024);
	printf("hash_time : %.3fs, %.2fMB/s\n", jcr.hash_time / 1000000,
			jcr.data_size * 1000000 / jcr.hash_time / 1024 / 1024);

	printf("dedup_time : %.3fs, %.2fMB/s\n",
			jcr.dedup_time / 1000000,
			jcr.data_size * 1000000 / jcr.dedup_time / 1024 / 1024);

	printf("rewrite_time : %.3fs, %.2fMB/s\n", jcr.rewrite_time / 1000000,
			jcr.data_size * 1000000 / jcr.rewrite_time / 1024 / 1024);

	printf("filter_time : %.3fs, %.2fMB/s\n",
			jcr.filter_time / 1000000,
			jcr.data_size * 1000000 / jcr.filter_time / 1024 / 1024);

	printf("write_time : %.3fs, %.2fMB/s\n", jcr.write_time / 1000000,
			jcr.data_size * 1000000 / jcr.write_time / 1024 / 1024);
	
	printf("index size : %" PRId64 "\n", total_index_size);

	char logfile[] = "backup.log";
	FILE *fp = fopen(logfile, "a");
	/*
	 * job id,
	 * the size of backup
	 * accumulative consumed capacity,
	 * deduplication rate,
	 * rewritten rate,
	 * total container number,
	 * sparse container number,
	 * inherited container number,
	 * 4 * index overhead (4 * int)
	 * throughput,
	 * chunk time,
	 * average chunk size
	 * chunk count
	 * chunking type
	 * index mode
	 * total index size
	 * external reverse mapping size
	 * lucene reverse mapping size
	 * db reverse mapping size
	 * dedup index size
	 * total time
	 */
	const char* chunking_types[] = { "default", "whitespace", "even", "four", "eight", "whitespace-reversed" };
	const char* index_modes[] = { "no", "naive", "dedup" };
	fprintf(fp, "%" PRId32 ",%" PRId64 ",%" PRId64 ",%.4f,%.4f,%" PRId32 ",%" PRId32 ",%" PRId32 
	",%" PRId32",%" PRId32 ",%" PRId32",%" PRId32",%.2f,%.3f,%.2f,%" PRId32",%s,%s,%" PRId64 
	",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%.3f\n",
			jcr.id,
			jcr.data_size,
			destor.stored_data_size,
			jcr.data_size != 0 ? (jcr.data_size - jcr.rewritten_chunk_size - jcr.unique_data_size)/(double) (jcr.data_size) : 0,
			jcr.data_size != 0 ? (double) (jcr.rewritten_chunk_size) / (double) (jcr.data_size) : 0,
			jcr.total_container_num,
			jcr.sparse_container_num,
			jcr.inherited_sparse_num,
			index_overhead.lookup_requests,
			index_overhead.lookup_requests_for_unique,
			index_overhead.update_requests,
			index_overhead.read_prefetching_units,
			(double) jcr.data_size * 1000000 / (1024 * 1024 * jcr.total_time),
			jcr.chunk_time / 1000000,
			jcr.data_size / (double) jcr.chunk_num,
			jcr.chunk_num,
			chunking_types[destor.chunking_type],
			index_modes[index_mode],
			total_index_size,
			external_reverse_mapping_size,
			lucene_reverse_mapping_size,
			db_reverse_mapping_size,
			dedup_index_size,
			jcr.total_time / 1000000);

	fclose(fp);

}
