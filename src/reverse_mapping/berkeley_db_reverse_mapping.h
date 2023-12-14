#ifndef _BERKELEY_DB_REVERSE_MAPPING_H_
#define _BERKELEY_DB_REVERSE_MAPPING_H_

#include "../destor.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <db.h>

#include "sync_queue.h"


void init_db_reverse_mapping();
void close_db_reverse_mapping();

void insert_db_reverse_mapping(const std::string& fp, const std::string& file_path);
void insert_db_reverse_mapping_offsets(const std::string& fp, const std::string& file_path, const std::vector<uint64_t>& offsets);
void insert_db_reverse_mapping_counter(const std::string& fp, const std::string& file_path, uint32_t count);
std::set<std::string> get_db_reverse_mapping_multiple(const std::vector<std::string>& fps, SyncQueue* queue = nullptr);
std::map<std::string, std::vector<uint64_t>> get_db_reverse_mapping_multiple_with_offsets(const std::map<std::string, std::vector<uint64_t>>& fps);
std::map<std::string, uint32_t> get_db_reverse_mapping_multiple_counter(const std::map<std::string, uint32_t>& fps);
void get_db_reverse_mapping_fetch(const std::string& fp, SyncQueue* queue);
void get_db_reverse_mapping_iterate(DBT* data, SyncQueue* queue);

#endif //_BERKELEY_DB_REVERSE_MAPPING_H_