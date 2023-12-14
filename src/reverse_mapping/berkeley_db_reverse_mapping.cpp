#include "berkeley_db_reverse_mapping.h"

static DB *db;

typedef unsigned char byte;

#define BUFFER_SIZE (10 * (1 << 10))

struct PathWithOffsets {
    std::string path;
    std::vector<uint64_t> offsets;
};

struct PathWithCounter {
    std::string path;
    uint32_t count;
};

static byte* memcpy_ser(byte* dst, const byte* src, uint64_t length) {
    memcpy(dst, src, length);
    return dst + length;
}

static PathWithOffsets deserializePathWithOffsets(byte* data) {
    uint64_t pathSize = *(uint64_t*)data;
    std::string path(pathSize, '\0');
    memcpy(path.data(), (data + sizeof(uint64_t)), pathSize);
    uint64_t offsetsNum = *(uint64_t*)(data + sizeof(uint64_t) + pathSize);
    std::vector<uint64_t> offsets(offsetsNum);
    for (size_t i = 0; i < offsetsNum; i++)
    {
        offsets[i] = *(uint64_t*)(data + sizeof(uint64_t) + pathSize + sizeof(uint64_t) + sizeof(uint64_t) * i);
    }
    
    return {path, offsets};
}
static PathWithCounter deserializePathWithCounter(byte* data) {
    uint64_t pathSize = *(uint64_t*)data;
    std::string path(pathSize, '\0');
    memcpy(path.data(), (data + sizeof(uint64_t)), pathSize);
    uint32_t count = *(uint32_t*)(data + sizeof(uint64_t) + pathSize);
    
    return {path, count};
}

static std::vector<byte> serializePathWithOffsets(const std::string& path, const std::vector<uint64_t> offsets) {
    
    uint64_t pathSize = path.size();
    uint64_t offsetsNum = offsets.size();
    std::vector<byte> ret(sizeof(pathSize) + pathSize + sizeof(offsetsNum) + offsetsNum * sizeof(offsets[0]));
    byte* buf = ret.data();

    buf = memcpy_ser(buf, (byte*)(&pathSize), sizeof(pathSize));
    buf = memcpy_ser(buf, path.data(), path.size());
    buf = memcpy_ser(buf, (byte*)(&offsetsNum), sizeof(offsetsNum));
    for (size_t i = 0; i < offsetsNum; i++) {
        buf = memcpy_ser(buf, (const byte*)(&offsets[i]), sizeof(offsets[i]));
    }
    
    return ret;
}

static std::vector<byte> serializePathWithCount(const std::string& path, uint32_t count) {
    
    uint64_t pathSize = path.size();
    std::vector<byte> ret(sizeof(pathSize) + pathSize + sizeof(count));
    byte* buf = ret.data();

    buf = memcpy_ser(buf, (byte*)(&pathSize), sizeof(pathSize));
    buf = memcpy_ser(buf, path.data(), path.size());
    buf = memcpy_ser(buf, (byte*)(&count), sizeof(count));
    
    return ret;
}


void init_db_reverse_mapping() {
    if (db_create(&db, NULL, 0) != 0) {
        printf("error creating db handle: %d\n", errno);
        exit(1);
    }
    if (mkdir(DB_REVERSE_MAPPING_DIR, 0755) != 0 && errno != EEXIST) {
        printf("error creating directory: %d\n", errno);
        exit(1);
    }
    if (db->set_flags(db, DB_DUP) != 0) {
        printf("unable to set flags: %d\n", errno);
        exit(1);
    }
    if (db->open(db, NULL, DB_REVERSE_MAPPING_PATH, NULL, DB_HASH, DB_CREATE, 0) != 0) {
        printf("error opening db: %d\n", errno);
        exit(1);
    }
}

void close_db_reverse_mapping() {
    if (db->close(db, 0) != 0) {
        printf("error closing db: %d\n", errno);
        exit(1);
    }
}

void insert_db_reverse_mapping(const std::string& fp, const std::string& file_path) {
    DBT key;
    DBT data;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = fp.data();
    key.size = fp.size();
    data.data = file_path.data();
    data.size = file_path.size();
    db->put(db, NULL, &key, &data, 0);
}

void insert_db_reverse_mapping_offsets(const std::string& fp, const std::string& file_path, const std::vector<uint64_t>& offsets) {
    DBT key;
    DBT data;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = fp.data();
    key.size = fp.size();
    std::vector<byte> serializedData = serializePathWithOffsets(file_path, offsets);
    data.data = serializedData.data();
    data.size = serializedData.size();
    db->put(db, NULL, &key, &data, 0);
}

void insert_db_reverse_mapping_counter(const std::string& fp, const std::string& file_path, uint32_t count) {
    DBT key;
    DBT data;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = fp.data();
    key.size = fp.size();
    std::vector<byte> serializedData = serializePathWithCount(file_path, count);
    data.data = serializedData.data();
    data.size = serializedData.size();
    db->put(db, NULL, &key, &data, 0);
}

std::set<std::string> get_db_reverse_mapping_multiple(const std::vector<std::string>& fps, 
                                                      SyncQueue* queue) {
    std::set<std::string> results;

    DBC *dbc;
    DBT key, data;
    size_t retklen, retdlen;
    void *retkey, *retdata;
    int ret, t_ret;
    uint32_t buffer_size = 1 << 20;
    void *p;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    if ((data.data = malloc(buffer_size)) == NULL) {
        printf("couldn't malloc!\n");
        exit(1);
    }
    data.ulen = buffer_size;
    data.flags = DB_DBT_USERMEM;

    /* Acquire a cursor for the database. */
    if ((ret = db->cursor(db, NULL, &dbc, DB_CURSOR_BULK)) 
        != 0) {
            db->err(db, ret, "DB->cursor");
            free(data.data);
            exit(1);
    }

    for (auto& fp : fps) {
        key.data = fp.data();
        key.size = fp.size();
        
        if ((ret = dbc->get(dbc, &key, &data, DB_SET | DB_MULTIPLE)) != 0) {
            if (ret != DB_NOTFOUND) {
                db->err(db, ret, "DBcursor->get");
                exit(1);
            }
        }
        while (true) {
            for (DB_MULTIPLE_INIT(p, &data);;) {
                DB_MULTIPLE_NEXT(p, &data, retdata, retdlen);
                if (p == NULL)
                    break;
                std::string to_insert{(const char*)retdata, retdlen};
                auto ret = results.insert(to_insert);
                if (queue && ret.second) { // if actually inserted
                    sync_queue_push(queue, new std::string(to_insert));
                }
            }

            if ((ret = dbc->get(dbc,
                &key, &data, DB_MULTIPLE | DB_NEXT_DUP)) != 0) {
                if (ret != DB_NOTFOUND) {
                    db->err(db, ret, "DBcursor->get");
                    exit(1);
                }
                break;
            }
        }
    }

    if ((t_ret = dbc->close(dbc)) != 0) {
        db->err(db, ret, "DBcursor->close");
        if (ret == 0)
            ret = t_ret;
    }

    return results;
}


std::map<std::string, uint32_t> get_db_reverse_mapping_multiple_counter(const std::map<std::string, uint32_t>& fps) {
    std::map<std::string, uint32_t> results;

    DBC *dbc;
    DBT key, data;
    size_t retklen, retdlen;
    void *retkey, *retdata;
    int ret, t_ret;
    uint32_t buffer_size = 1 << 20;
    void *p;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    if ((data.data = malloc(buffer_size)) == NULL) {
        printf("couldn't malloc!\n");
        exit(1);
    }
    data.ulen = buffer_size;
    data.flags = DB_DBT_USERMEM;

    /* Acquire a cursor for the database. */
    if ((ret = db->cursor(db, NULL, &dbc, DB_CURSOR_BULK)) 
        != 0) {
            db->err(db, ret, "DB->cursor");
            free(data.data);
            exit(1);
    }

    for (auto& fp_and_counter : fps) {
        auto& fp = fp_and_counter.first;
        auto fp_counter = fp_and_counter.second;
        key.data = fp.data();
        key.size = fp.size();
        
        if ((ret = dbc->get(dbc, &key, &data, DB_SET | DB_MULTIPLE)) != 0) {
            if (ret != DB_NOTFOUND) {
                db->err(db, ret, "DBcursor->get");
                exit(1);
            }
        }
        while (true) {
            for (DB_MULTIPLE_INIT(p, &data);;) {
                DB_MULTIPLE_NEXT(p, &data, retdata, retdlen);
                if (p == NULL)
                    break;
                PathWithCounter to_insert{deserializePathWithCounter((byte*)retdata)};
                auto it = results.find(to_insert.path);
                if (it != results.end()) {
                    it->second += to_insert.count * fp_counter;
                } else {
                    results.emplace_hint(it, to_insert.path, to_insert.count * fp_counter);
                }
            }

            if ((ret = dbc->get(dbc,
                &key, &data, DB_MULTIPLE | DB_NEXT_DUP)) != 0) {
                if (ret != DB_NOTFOUND) {
                    db->err(db, ret, "DBcursor->get");
                    exit(1);
                }
                break;
            }
        }
    }

    if ((t_ret = dbc->close(dbc)) != 0) {
        db->err(db, ret, "DBcursor->close");
        if (ret == 0)
            ret = t_ret;
    }

    return results;
}

std::map<std::string, std::vector<uint64_t>> get_db_reverse_mapping_multiple_with_offsets(const std::map<std::string, std::vector<uint64_t>>& fps) {
    std::map<std::string, std::vector<uint64_t>> results;

    DBC *dbc;
    DBT key, data;
    size_t retklen, retdlen;
    void *retkey, *retdata;
    int ret, t_ret;
    uint32_t buffer_size = 1 << 20;
    void *p;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    if ((data.data = malloc(buffer_size)) == NULL) {
        printf("couldn't malloc!\n");
        exit(1);
    }
    data.ulen = buffer_size;
    data.flags = DB_DBT_USERMEM;

    /* Acquire a cursor for the database. */
    if ((ret = db->cursor(db, NULL, &dbc, DB_CURSOR_BULK)) 
        != 0) {
            db->err(db, ret, "DB->cursor");
            free(data.data);
            exit(1);
    }

    for (auto& fp_and_offsets : fps) {
        auto& fp = fp_and_offsets.first;
        auto& fp_offsets = fp_and_offsets.second;

        key.data = fp.data();
        key.size = fp.size();
        
        if ((ret = dbc->get(dbc, &key, &data, DB_SET | DB_MULTIPLE)) != 0) {
            if (ret != DB_NOTFOUND) {
                db->err(db, ret, "DBcursor->get");
                exit(1);
            }
        }
        while (true) {
            for (DB_MULTIPLE_INIT(p, &data);;) {
                DB_MULTIPLE_NEXT(p, &data, retdata, retdlen);
                if (p == NULL)
                    break;
                PathWithOffsets to_insert(deserializePathWithOffsets((byte*)retdata));
                std::vector<uint64_t> final_offsets;
                for (auto& offset : to_insert.offsets) {
                    for (auto& fp_offset : fp_offsets) {
                        final_offsets.push_back(offset + fp_offset);
                    }
                }

                auto iterator = results.find(to_insert.path);
                if (iterator != results.end()) {
                    iterator->second.insert(iterator->second.end(), final_offsets.begin(), final_offsets.end());
                } else {
                    results.emplace(to_insert.path, final_offsets);
                }
            }

            if ((ret = dbc->get(dbc,
                &key, &data, DB_MULTIPLE | DB_NEXT_DUP)) != 0) {
                if (ret != DB_NOTFOUND) {
                    db->err(db, ret, "DBcursor->get");
                    exit(1);
                }
                break;
            }
        }
    }

    if ((t_ret = dbc->close(dbc)) != 0) {
        db->err(db, ret, "DBcursor->close");
        if (ret == 0)
            ret = t_ret;
    }

    return results;
}

void get_db_reverse_mapping_fetch(const std::string& fp, 
                                  SyncQueue* queue) {

    DBC *dbc;
    DBT key, *data;
    int ret, t_ret;
    uint32_t buffer_size = BUFFER_SIZE;

    memset(&key, 0, sizeof(key));

    /* Acquire a cursor for the database. */
    if ((ret = db->cursor(db, NULL, &dbc, DB_CURSOR_BULK)) 
        != 0) {
            db->err(db, ret, "DB->cursor");
            exit(1);
    }

    key.data = fp.data();
    key.size = fp.size();


    data = new DBT;
    memset(data, 0, sizeof(*data));
    if ((data->data = malloc(buffer_size)) == NULL) {
        printf("couldn't malloc!\n");
        exit(1);
    }
    data->ulen = buffer_size;
    data->flags = DB_DBT_USERMEM;
    if ((ret = dbc->get(dbc, &key, data, DB_SET | DB_MULTIPLE)) != 0) {
        if (ret != DB_NOTFOUND) {
            db->err(db, ret, "DBcursor->get");
            exit(1);
        }
    }

    while (true) {
        sync_queue_push(queue, data);
        
        data = new DBT;
        memset(data, 0, sizeof(*data));
        if ((data->data = malloc(buffer_size)) == NULL) {
            printf("couldn't malloc!\n");
            exit(1);
        }
        data->ulen = buffer_size;
        data->flags = DB_DBT_USERMEM;  
        if ((ret = dbc->get(dbc, &key, data, DB_MULTIPLE | DB_NEXT_DUP)) != 0) {
            if (ret != DB_NOTFOUND) {
                db->err(db, ret, "DBcursor->get");
                exit(1);
            }
            break;
        }
    }
    
    if ((t_ret = dbc->close(dbc)) != 0) {
        db->err(db, ret, "DBcursor->close");
        if (ret == 0)
            ret = t_ret;
    }
}

void get_db_reverse_mapping_iterate(DBT* data, SyncQueue* queue) {
    
    size_t retklen, retdlen;
    void *retkey, *retdata;
    void *p;
    
    for (DB_MULTIPLE_INIT(p, data);;) {
        DB_MULTIPLE_NEXT(p, data, retdata, retdlen);
        if (p == NULL)
            break;
        std::string to_insert{(const char*)retdata, retdlen};
        sync_queue_push(queue, new std::string(to_insert));
    }
}