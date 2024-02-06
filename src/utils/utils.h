/**
 * @file utils.c
 * @author Asaf Levi
 * 
 * ADDED FOR IDEA
 */
#ifndef _UTILS_H_
#define _UTILS_H_

#define _GNU_SOURCE
#include <pthread.h>
#include <string>

unsigned long long file_size(const char* path);
int join_thread_with_timeout(pthread_t thread, int timeout);

/// not thread safe
unsigned long long dir_size(const char* path);

std::string hex(const unsigned char* bytes, size_t length);
std::string hex_H_prefix(const unsigned char* bytes, size_t length);

#endif