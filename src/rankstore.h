#ifndef _RANKSTORE_H_
#define _RANKSTORE_H_

#include <string>

void init_rankstore();
void close_rankstore();

void insert_chunk_term_count(const std::string& fp, uint32_t numTokens);
uint32_t num_tokens_chunk(const std::string& fp);

void insert_filename_term_count(const std::string& filename, uint32_t numTokens);
uint32_t num_tokens_filename(const std::string& fp);

void setGlobalFileCount(uint64_t fileCount);
uint64_t getGlobalFileCount();

#endif _RANKSTORE_H_