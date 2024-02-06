/**
 * ADDED FOR THE IDEA PROJECT
 * @file chunking.c
 * @author Asaf Levi
 * @brief created to support generic chunking methods
 *        with various alignments.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "../destor.h"

#include <ctype.h>
#include "chunking.h"

#define MAX_SKEW 128

int(*alignment_function[NUMBER_OF_CHUNKING_TYPES])(unsigned char*, int, int) = {
	/*DEFAULT=*/do_not_align,
	/*WHITESPACE=*/align_chunk_to_whitespace,
	/*EVENBYTE=*/align_chunk_to_evenbyte,
	/*FOURBYTE=*/align_chunk_to_fourbyte,
	/*EIGHTBYTE=*/align_chunk_to_eightbyte,
	/*WHITESPACE_REVERSED=*/align_chunk_to_whitespace_reversed,
};

static inline int isdivisible(int k, int divisor) {
	return (k % divisor) == 0;
}

static int iseven(int k) {
	return isdivisible(k, 2);
}

static int isfourbyte(int k) {
	return isdivisible(k, 4);
}

static int iseightbyte(int k) {
	return isdivisible(k, 8);
}

static int align_chunk_by_condition(unsigned char *p, int n, int original_chunk_size, int(*condition_func)(int)) {
	p += original_chunk_size;
	n -= original_chunk_size;
	int boundary_skew = 0;

	while (original_chunk_size + boundary_skew <= destor.chunk_max_size &&
		boundary_skew < n && !condition_func(p[boundary_skew])) {
		boundary_skew++;
	}
	if (original_chunk_size + boundary_skew <= destor.chunk_max_size && boundary_skew < n) {
		boundary_skew++; // skip whitespace to be at end of chunk instead of start of chunk.
	}

	return boundary_skew;
}

static int align_chunk_by_condition_reversed(unsigned char *p, int n, int original_chunk_size, int(*condition_func)(int)) {
	p += original_chunk_size;
	n -= original_chunk_size;
	
	if (n == 0) 
		return 0;
	
	int boundary_skew = 0;

	while (-boundary_skew < MAX_SKEW &&
		original_chunk_size + boundary_skew > 0 && !condition_func(p[boundary_skew])) {
		boundary_skew--;
	}

	if ((-original_chunk_size) == boundary_skew || 
		(boundary_skew == -MAX_SKEW && !condition_func(p[boundary_skew]))) {
		return 0;
	}

	if (boundary_skew < -1000)
		printf("skew=%d, original=%d\n", boundary_skew, original_chunk_size);
	return boundary_skew;
}

int do_not_align(unsigned char*, int, int)
{
	return 0;
}

int align_chunk_to_whitespace(unsigned char* p, int n, int original_chunk_size) {
	return align_chunk_by_condition(p, n, original_chunk_size, isspace);
}

int align_chunk_to_whitespace_reversed(unsigned char* p, int n, int original_chunk_size) {
	return align_chunk_by_condition_reversed(p, n, original_chunk_size, isspace);
}

int align_chunk_to_evenbyte(unsigned char* p, int n, int original_chunk_size) {
	return align_chunk_by_condition(p, n, original_chunk_size, iseven);
}

int align_chunk_to_fourbyte(unsigned char* p, int n, int original_chunk_size) {
	return align_chunk_by_condition(p, n, original_chunk_size, isfourbyte);
}

int align_chunk_to_eightbyte(unsigned char* p, int n, int original_chunk_size) {
	return align_chunk_by_condition(p, n, original_chunk_size, iseightbyte);
}
