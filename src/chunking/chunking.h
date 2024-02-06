/**
 * SEVERAL FEATURES WERE ADDED FOR THE IDEA PROJECT
 * @file chunking.h
 * @author Asaf Levi
 * @brief created to support generic chunking methods
 *        with various functions and alignments.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef CHUNK_H_
#define CHUNK_H_

void windows_reset();
void chunkAlg_init();
int rabin_chunk_data(unsigned char *p, int n);
int normalized_rabin_chunk_data(unsigned char *p, int n);

void ae_init();
int ae_chunk_data(unsigned char *p, int n);

int tttd_chunk_data(unsigned char *p, int n);

int do_not_align(unsigned char*, int, int);
int align_chunk_to_whitespace(unsigned char *p, int n, int original_chunk_size);
int align_chunk_to_whitespace_reversed(unsigned char *p, int n, int original_chunk_size);
int align_chunk_to_evenbyte(unsigned char *p, int n, int original_chunk_size);
int align_chunk_to_fourbyte(unsigned char *p, int n, int original_chunk_size);
int align_chunk_to_eightbyte(unsigned char *p, int n, int original_chunk_size);

typedef enum {
	DEFAULT = 0,
	WHITESPACE,
	EVENBYTE,
	FOURBYTE,
	EIGHTBYTE,
	WHITESPACE_REVERSED,
	NUMBER_OF_CHUNKING_TYPES
} ChunkingType;

extern int(*alignment_function[])(unsigned char*, int, int);


#endif
