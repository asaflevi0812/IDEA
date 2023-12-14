#ifndef _EXTERNAL_REVERSE_MAPPING_H_
#define _EXTERNAL_REVERSE_MAPPING_H_

#include "../destor.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <set>


void init_external_reverse_mapping();
void close_external_reverse_mapping(bool is_read_optimized);

void insert_external_reverse_mapping(const std::string& fp, const std::string& file_path);
std::set<std::string> get_external_reverse_mapping_multiple(const std::vector<std::string>& fps, bool trace_new_results=false);
std::vector<std::wstring> get_external_reverse_mapping(const std::string& fp);
std::unordered_map<std::string, uint32_t>& get_external_reverse_mapping_keys();

#endif //_EXTERNAL_REVERSE_MAPPING_H_