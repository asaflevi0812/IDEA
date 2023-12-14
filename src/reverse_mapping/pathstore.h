#ifndef __PATHSTORE_H__
#define __PATHSTORE_H__

#include <string>

void init_pathstore();

unsigned int add_path(const std::string& path);

std::string get_path(unsigned int key);

void close_pathstore();


#endif // __PATHSTORE_H__