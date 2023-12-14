/*
 * destor.h
 *
 *  Created on: Nov 8, 2013
 *      Author: fumin
 */

#ifndef DESTOR_H_
#define DESTOR_H_

#define _GNU_SOURCE // DedupSearch it reduces portability of the program...

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <openssl/sha.h>
#include <glib.h>
#include <getopt.h>

#include "chunking/chunking.h"
#include "utils/sds.h"

#define TIMER_DECLARE(n) struct timeval b##n,e##n
#define TIMER_BEGIN(n) gettimeofday(&b##n, NULL)
#define TIMER_END(n,t) gettimeofday(&e##n, NULL); \
    (t)+=e##n.tv_usec-b##n.tv_usec+1000000*(e##n.tv_sec-b##n.tv_sec)

#define DESTOR_CONFIGLINE_MAX 1024

#define DESTOR_BACKUP 1
#define DESTOR_RESTORE 2
#define DESTOR_MAKE_TRACE 3
#define DESTOR_NAIVE_LOOKUP 4
#define DESTOR_DEDUPLICATED_INDEX 5
#define DESTOR_NAIVE_INDEX 6
#define DESTOR_DEDUPLICATED_LOOKUP 7

/* Log levels */
#define DESTOR_DEBUG 0
#define DESTOR_VERBOSE 1
#define DESTOR_NOTICE 2
#define DESTOR_WARNING 3
#define DESTOR_DEFAULT_VERBOSITY DESTOR_NOTICE
#define DESTOR_MAX_LOGMSG_LEN 1024

/* Simuation-level, ascending */
#define SIMULATION_NO 0
#define SIMULATION_RESTORE 1
#define SIMULATION_APPEND 2
#define SIMULATION_ALL 3

/* trace format */
#define TRACE_DESTOR 0
#define TRACE_FSL 1

#define CHUNK_FIXED 0
#define CHUNK_RABIN 1
#define CHUNK_NORMALIZED_RABIN 2
#define CHUNK_FILE 3 /* approximate file-level */
#define CHUNK_AE 4 /* Asymmetric Extremum CDC */
#define CHUNK_TTTD 5

/* index modes */
#define INDEX_MODE_NO 0
#define INDEX_MODE_NAIVE 1
#define INDEX_MODE_DEDUP 2
/* indexing flags */
#define NAIVE_INDEX 1
#define DEDUP_INDEX 2
/* reverse mapping */
#define LUCENE_REVERSE_MAPPING 4
#define EXTERNAL_REVERSE_MAPPING 8
#define READ_OPTIMIZED_EXTERNAL_REVERSE_MAPPING 16
#define EXHAUSTIVE_SEARCH_REVERSE_MAPPING 32
#define DB_REVERSE_MAPPING 64
#define IN_DOC_REVERSE_MAPPING 128
/* offset modes */
#define NO_OFFSETS 0
#define GLOBAL_OFFSETS 1
#define TERM_VECTOR_OFFSETS 2
#define GLOBAL_TERM_VECTOR_OFFSETS 3


#define CHECK_FLAG(var, flag) ((var & flag) != 0)

/*
 * A global fingerprint index is required.
 * A successful query returns a container id or a segment id for prefetching.
 * For those chunks failing to be deduplicated with prefetched containers or segments,
 * we further deduplicate them in the fingerprint index.
 * Thus none of stored chunks are duplicate.
 */
#define INDEX_CATEGORY_EXACT 0
/*
 * No global fingerprint index.
 * We only deduplicate chunks with prefetched containers or segments.
 * Without the global fingerprint index,
 * we require an in-memory feature index to determine which container or segment is prefetched.
 */
#define INDEX_CATEGORY_NEAR_EXACT 1
/*
 * Container is the unit of physical locality (locality).
 * Container is fixed-sized, and identified by an id.
 * In the category,
 * we select features in unit of container.
 */
#define INDEX_CATEGORY_PHYSICAL_LOCALITY 2
#define INDEX_CATEGORY_LOGICAL 2
/*
 * Segment is the unit of logical locality (similarity).
 * Segment can be either fixed-sized or variable-sized,
 * and identified by an id.
 * we select features in unit of segment.
 */
#define INDEX_CATEGORY_LOGICAL_LOCALITY 3
#define INDEX_CATEGORY_SIMILARITY 3

/*
 * Supported key-value store,
 * including hash table, MySQL.
 */
#define INDEX_KEY_VALUE_HTABLE 0
#define INDEX_KEY_VALUE_MYSQL 1
/*
 * Feature is used for prefetching segments (similarity) or containers (locality).
 * For example, when we find a duplicate chunk,
 * its feature is mapped to a container or a segment.
 * Pretching the container or segment can facilitate detecting following duplicate chunks.
 * ALL considers each fingerprint as a feature.
 * SAMPLE selects features via random sampling.
 * MIN selects minimal fingerprint(s) as feature.
 * UNIFORM selects a feature every n fingerprints.
 */
#define INDEX_SAMPLING_RANDOM 1
#define INDEX_SAMPLING_MIN 2
#define INDEX_SAMPLING_UNIFORM 3
#define INDEX_SAMPLING_OPTIMIZED_MIN 4

/*
 * Unlike container that is a physical unit,
 * segment is a logical unit.
 * We can divided a backup stream into segments via different methods.
 */
#define INDEX_SEGMENT_FIXED 0
#define INDEX_SEGMENT_CONTENT_DEFINED 1
#define INDEX_SEGMENT_FILE_DEFINED 2

/*
 * Many eligible segments may be found via looking up in feature index.
 * Further select champion segments in the eligible segments.
 * MIX indicates a mix of base and top.
 */
#define INDEX_SEGMENT_SELECT_BASE 0
#define INDEX_SEGMENT_SELECT_TOP 1
#define INDEX_SEGMENT_SELECT_MIX 2

/*
 * A specific fingerprint index,
 * similar with a combo.
 */
#define INDEX_SPECIFIC_NO 0
#define INDEX_SPECIFIC_DDFS 1
#define INDEX_SPECIFIC_EXTREME_BINNING 2
#define INDEX_SPECIFIC_SILO 3
#define INDEX_SPECIFIC_SPARSE 4
#define INDEX_SPECIFIC_SAMPLED 5
#define INDEX_SPECIFIC_BLOCK_LOCALITY_CACHING 6

#define RESTORE_CACHE_LRU 0
#define RESTORE_CACHE_OPT 1
#define RESTORE_CACHE_ASM 2

#define REWRITE_NO 0
#define REWRITE_CFL_SELECTIVE_DEDUPLICATION 1
#define REWRITE_CONTEXT_BASED 2
#define REWRITE_CAPPING 3

#define TEMPORARY_ID (-1L)

/* the buffer size for read phase */
#define DEFAULT_BLOCK_SIZE 1048576 //1MB

/* the max input buffer size for keywords */
#define MAX_KEYWORD_SIZE 128

/* states of normal chunks. */
#define CHUNK_UNIQUE (0x0000)
#define CHUNK_DUPLICATE (0x0100)
#define CHUNK_SPARSE (0x0200)
#define CHUNK_OUT_OF_ORDER (0x0400)
/* IN_CACHE will deny rewriting an out-of-order chunk */
#define CHUNK_IN_CACHE (0x0800)
/* This flag will deny all rewriting, including a sparse chunk */
#define CHUNK_REWRITE_DENIED (0x1000)
/* This flag represents a chunk that was added to a container during filter phase */
#define CHUNK_ADDED_TO_CONTAINERSTORE (0x2000)

/* signal chunk */
#define CHUNK_FILE_START (0x0001)
#define CHUNK_FILE_END (0x0002)
#define CHUNK_SEGMENT_START (0x0004)
#define CHUNK_SEGMENT_END (0x0008)

/* Flags for restore */
#define CHUNK_WAIT 0x0010
#define CHUNK_READY 0x0020

#define SET_CHUNK(c, f) (c->flag |= f)
#define UNSET_CHUNK(c, f) (c->flag &= ~f)
#define CHECK_CHUNK(c, f) (c->flag & f)

#define NAIVE_INDEX_DIR ((std::string((char*)destor.index_directory) + "/naive_index").c_str())
#define DEDUP_INDEX_DIR ((std::string((char*)destor.index_directory) + (destor.reverse_mapping_type == IN_DOC_REVERSE_MAPPING ? "/dedup_in_doc" : "/dedup_index")).c_str())
#define EXTERNAL_REVERSE_MAPPING_DIR ((std::string((char*)destor.reverse_directory) + "/external_reverse_mapping").c_str())
#define LUCENE_REVERSE_MAPPING_DIR ((std::string((char*)destor.reverse_directory) + "/lucene_reverse_mapping").c_str())
#define EXTERNAL_REVERSE_MAPPING_PATH ((std::string((char*)destor.reverse_directory) + "/external_reverse_mapping/reverse_mapping.dump").c_str())
#define EXTERNAL_REVERSE_MAPPING_MINI_INDEX_PATH ((std::string((char*)destor.reverse_directory) + "/external_reverse_mapping/reverse_mapping.index").c_str())
#define DB_REVERSE_MAPPING_DIR ((std::string((char*)destor.reverse_directory) + (destor.is_path_indirected ? "/db_reverse_mapping_path/" : "/db_reverse_mapping/")).c_str())
#define DB_REVERSE_MAPPING_PATH ((std::string((char*)destor.reverse_directory) + (destor.is_path_indirected ? "/db_reverse_mapping_path/reverse_mapping.db" : "/db_reverse_mapping/reverse_mapping.db")).c_str())
#define PATHSTORE_PATH ((std::string((char*)destor.reverse_directory) + "/pathstore.db").c_str())
#define RANKSTORE_PATH ((std::string((char*)destor.reverse_directory) + "/rankstore").c_str())
#define RANKSTORE_DB_PATH ((std::string((char*)destor.reverse_directory) + "/rankstore.db").c_str())

struct destor {
	sds working_directory;
	sds index_directory;
	sds reverse_directory;
	int simulation_level;
    int trace_format;
	int verbosity;

	int chunk_algorithm;
	int chunk_max_size;
	int chunk_min_size;
	int chunk_avg_size;

	/* the cache type and size */
	int restore_cache[2];
	int restore_opt_window_size;

	/* Specify fingerprint index,
	 * exact or near-exact,
	 * physical logical locality */
	int index_category[2];
	/* optional */
	int index_specific;

	/* in number of containers, for DDFS/ChunkStash/Sampled Index. */
	int index_cache_size;
	int index_bloom_filter_size;

	/*
	 * [0] specifies the sampling method, and
	 * [1] specifies the sampling ratio.
	 */
	int index_sampling_method[2];
	/* Specify the key-value store. */
	int index_key_value_store;
	/* The max number of prefetching units a key can refer to */
	int index_value_length;
	/* the size of the key in byte */
	int index_key_size;

	/*
	 * [0] specifies the algorithm,
	 * and [1] specifies the average segment size.
	 */
	int index_segment_algorithm[2];
	int index_segment_min;
	int index_segment_max;

	int index_segment_selection_method[2];
	int index_segment_prefech;

	int rewrite_algorithm[2];
	int rewrite_enable_cfl_switch;
	/* for CFL-based selective deduplication */
	double rewrite_cfl_require;
	double rewrite_cfl_usage_threshold;
	/* for Context-Based Rewriting (CBR) */
	double rewrite_cbr_limit;
	double rewrite_cbr_minimal_utility;
	/* for capping */
	int rewrite_capping_level;

	/* for History-Aware Rewriting (HAR) */
	int rewrite_enable_har;
	double rewrite_har_utilization_threshold;
	double rewrite_har_rewrite_limit;

	/* for Cache-Aware Filter */
	int rewrite_enable_cache_aware;

	/* statistics of destor	 */
	int64_t chunk_num;
	int64_t stored_chunk_num;

	int64_t data_size;
	int64_t stored_data_size;

	int64_t zero_chunk_num;
	int64_t zero_chunk_size;

	int64_t rewritten_chunk_num;
	int64_t rewritten_chunk_size;

	int32_t index_memory_footprint;

	/* The number of live containers */
	int32_t live_container_num;

	int backup_retention_time;

	/* Whether to align the chunk boundaries by some condition */
	ChunkingType chunking_type;

	/* Which index reverse mapping is used */
	uint32_t reverse_mapping_type;

	/* Whether path storage should be deduplicated with another level of indirection */
	bool is_path_indirected;
	
	/* whether lookup is threaded */
	bool is_threaded_lookup;

	/* whether doc id is textual - 
	   affects only lookup, indexing with binary doc id is deprecated */
	bool is_textual_doc_id;

	/* the offset storing mode for the index */
	uint32_t offsets_mode;

	/* whether to index and emit tf-idf */
	bool tf_idf;

}; // DedupSearch

extern struct destor destor; // DedupSearch

typedef unsigned char fingerprint[20];
typedef int64_t containerid; //container id
typedef int64_t segmentid;

struct chunk {
	int32_t size;
	int flag;
	containerid id;
	fingerprint fp;
	unsigned char *data;
};

/* struct segment only makes sense for index. */
struct segment {
	segmentid id;
	/* The actual number because there are signal chunks. */
	int32_t chunk_num;
	GSequence *chunks;
	GHashTable* features;
};

struct chunk* new_chunk(int32_t);
void free_chunk(struct chunk*);

struct segment* new_segment();
struct segment* new_segment_full();
void free_segment(struct segment* s);

gboolean g_fingerprint_equal(fingerprint* fp1, fingerprint* fp2);
gint g_fingerprint_cmp(fingerprint* fp1, fingerprint* fp2, gpointer user_data);
gint g_chunk_cmp(struct chunk* a, struct chunk* b, gpointer user_data);

void hash2code(unsigned char hash[20], char code[40]);

#define DEBUG(fmt, arg...) destor_log(DESTOR_DEBUG, fmt, ##arg);
#define VERBOSE(fmt, arg...) destor_log(DESTOR_VERBOSE, fmt, ##arg);
#define NOTICE(fmt, arg...) destor_log(DESTOR_NOTICE, fmt, ##arg);
#define WARNING(fmt, arg...) destor_log(DESTOR_WARNING, fmt, ##arg);

void destor_log(int level, const char *fmt, ...);

#endif /* DESTOR_H_ */
