#include "rankstore.h"

#include <map>
#include <db.h>
#include <cstring>

#include "destor.h"

static std::map<std::string, uint32_t> fpToTokenNum; 

static DB *db;
static constexpr unsigned BUFFER_SIZE = 4096;
static char buffer[BUFFER_SIZE];

void init_rankstore() {
    if (db_create(&db, NULL, 0) != 0) {
        printf("error creating db handle: %d\n", errno);
        exit(1);
    }
    if (db->open(db, NULL, RANKSTORE_DB_PATH, NULL, DB_HASH, DB_CREATE, 0) != 0) {
        printf("error opening db: %d\n", errno);
        exit(1);
    }
}

void close_rankstore() {
    if (db->close(db, 0) != 0) {
        printf("error closing db: %d\n", errno);
        exit(1);
    }
}

void insert_chunk_term_count(const std::string& fp, uint32_t numTokens) {
    fpToTokenNum.emplace(fp, numTokens);
}
uint32_t num_tokens_chunk(const std::string& fp) {
    auto it = fpToTokenNum.find(fp);
    if (it == fpToTokenNum.end()) return 0;
    return it->second;
}

void setGlobalFileCount(uint64_t fileCount) {
    FILE* rankstore = fopen(RANKSTORE_PATH, "wb");
    fwrite(&fileCount, sizeof(fileCount), 1, rankstore);
    fclose(rankstore);
}

uint64_t getGlobalFileCount() {
    uint64_t fileCount = 0;
    FILE* rankstore = fopen(RANKSTORE_PATH, "rb");
    fread(&fileCount, sizeof(fileCount), 1, rankstore);
    fclose(rankstore);
    return fileCount;
}

void insert_filename_term_count(const std::string& filename, uint32_t numTokens) {
    DBT key;
    DBT data;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = filename.data();
    key.size = filename.size();
    data.data = &numTokens;
    data.size = sizeof(numTokens);
    db->put(db, NULL, &key, &data, 0);
}

uint32_t num_tokens_filename(const std::string& filename) {
    DBT key, data;
    std::memset(&key, 0, sizeof(key));
    std::memset(&data, 0, sizeof(data));
    key.data = filename.data();
    key.size = filename.size();
    data.data = buffer;
    data.ulen = BUFFER_SIZE;
    data.flags = DB_DBT_USERMEM;
    if (db->get(db, NULL, &key, &data, 0)) {
        printf("error in db->get: %d\n", errno);
        exit(1);
    }

    return *(uint32_t*)buffer;
}