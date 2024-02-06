/**
 * @file utils.c
 * @author Asaf Levi
 * 
 * ADDED FOR IDEA
 */
#include "utils.h"

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned long long size = 0;

unsigned long long file_size(const char* path) {
	struct stat st;
	if (stat(path, &st)) {
		perror("error in stat because");
		exit(1);
	}
	return st.st_size;
}

int sum_dir_size(const char*, const struct stat *stat, int) {
	size += stat->st_size;
	return 0;
}

int join_thread_with_timeout(pthread_t thread, int timeout) {
	struct timespec timeout_s = { 0 };
	clock_gettime(CLOCK_REALTIME, &timeout_s);
	timeout_s.tv_sec += timeout;
	return pthread_timedjoin_np(thread, NULL, &timeout_s);
}

unsigned long long dir_size(const char* path) {
	size = 0;

	if (ftw(path, &sum_dir_size, 1)) {
		perror("error in ftw because");
		exit(1);
	}
	
	return size;
}

std::string hex(const unsigned char* bytes, size_t length) {
	static const char digit_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', 
									 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	
	std::string ret;
	for (size_t i = 0; i < length; i++)
	{
		ret += digit_map[bytes[i] >> 4];
		ret += digit_map[bytes[i] & 0xf];
	}
	
	return ret;
}

std::string hex_H_prefix(const unsigned char* bytes, size_t length) {
	return std::string("h") + hex(bytes, length);
}