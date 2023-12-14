#include "pathstore.h"

#include <db.h>
#include <cstring>

#include "destor.h"

static DB *db;
static constexpr unsigned BUFFER_SIZE = 4096;
static char buffer[BUFFER_SIZE];


void init_pathstore() {
    if (db_create(&db, NULL, 0) != 0) {
        printf("error creating db handle: %d\n", errno);
        exit(1);
    }
    if (db->open(db, NULL, PATHSTORE_PATH, NULL, DB_RECNO, DB_CREATE, 0) != 0) {
        printf("error opening db: %d\n", errno);
        exit(1);
    }
}

unsigned int add_path(const std::string& path) {
    unsigned int counter;
    DBT key, data;
    key.data = &counter;
    key.ulen = sizeof(counter);
    key.flags = DB_DBT_USERMEM;
    data.data = path.data();
    data.size = path.size();
    data.flags = 0;
    if (db->put(db, NULL, &key, &data, DB_APPEND)) {
        printf("error in db->put: %d\n", errno);
        exit(1);
    }
    return counter;
}

std::string get_path(unsigned int keyCounter) {
    DBT key, data;
    std::memset(&key, 0, sizeof(key));
    std::memset(&data, 0, sizeof(data));
    key.data = &keyCounter;
    key.size = sizeof(keyCounter);
    data.data = buffer;
    data.ulen = BUFFER_SIZE;
    data.flags = DB_DBT_USERMEM;
    if (db->get(db, NULL, &key, &data, 0)) {
        printf("error in db->get: %d\n", errno);
        exit(1);
    }

    return std::string(buffer, data.size);
}

void close_pathstore() {
    if (db->close(db, 0) != 0) {
        printf("error closing db: %d\n", errno);
        exit(1);
    }
}