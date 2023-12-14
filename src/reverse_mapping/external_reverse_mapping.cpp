#include "reverse_mapping/external_reverse_mapping.h"
#include "clucene-wrapper/libclucene-wrapper.h"

#include "jcr.h"

#include <algorithm>

#define READ_EXACT(fd, buf, size) do { \
    if (read(fd, buf, size) != size) { \
        perror("could not read exactly because"); exit(1); \
    } \
} while (0);
#define READ_EXACT_INTO_VAR(fd, var) READ_EXACT(fd, &var, sizeof(var));

#define WRITE_EXACT(fd, buf, size) do { \
    if (write(fd, buf, size) != size) { \
        perror("could not write exactly because"); exit(1); \
    } \
} while (0);
#define WRITE_EXACT_VAR(fd, var) WRITE_EXACT(fd, &var, sizeof(var));

static std::unordered_multimap<std::string, std::string> reverse_mapping;
static std::unordered_map<std::string, uint32_t> keys;
static bool is_changed = false;
static bool is_initialized = false;

std::unordered_map<std::string, uint32_t>& get_external_reverse_mapping_keys() {
    return keys;
}

static uint32_t read_uint32(int fd) {
    uint32_t ret;
    READ_EXACT_INTO_VAR(fd, ret);
    return ret;
}

static uint64_t read_uint64(int fd) {
    uint64_t ret;
    READ_EXACT_INTO_VAR(fd, ret);
    return ret;
}

static void write_uint32(int fd, uint32_t val) {
    WRITE_EXACT_VAR(fd, val);
}

static void write_uint64(int fd, uint64_t val) {
    WRITE_EXACT_VAR(fd, val);
}

static std::string read_string(int fd) {
    uint32_t size;
    READ_EXACT_INTO_VAR(fd, size);
    std::string ret(size, '\0');
    READ_EXACT(fd, ret.data(), size);
    return ret;
}

static void write_string(int fd, const std::string& str) {
    uint32_t size = str.length();
    WRITE_EXACT_VAR(fd, size);
    WRITE_EXACT(fd, str.data(), size);
}

static std::string read_fingerprint(int fd) {
    return read_string(fd);
}

static void write_fingerprint(int fd, const std::string& fp) {
    return write_string(fd, fp);
}

static std::vector<uint64_t> get_fingerprints_offsets(const std::vector<std::string>& fps) {
    int fd_index = open(EXTERNAL_REVERSE_MAPPING_MINI_INDEX_PATH, O_RDONLY);
    if (fd_index < 0) {
        perror("could not open reverse mapping because");
        exit(1);
    }
    
    std::unordered_map<std::string, uint64_t> offsets;
    std::vector<uint64_t> results;

    uint32_t keys = read_uint32(fd_index);
    for (uint32_t i = 0; i < keys; i++) {
        std::string fp = read_fingerprint(fd_index);
        uint64_t offset = read_uint64(fd_index);
        offsets.insert(std::pair<std::string, uint64_t>(fp, offset));
    }

    for (const auto& fp : fps) {
        results.push_back(offsets.find(fp)->second);
    }
    
    if (fps.size() != results.size()) {
        printf("ASSERTION ERROR: didn't find all fds in mini index\n");
        exit(1);
    }

    close(fd_index);

    return results;
}

static std::vector<std::string> get_paths_from_offset(int fd, uint64_t offset) {
    if (lseek(fd, offset, SEEK_SET) < 0) {
        perror("could not seek in reverse mapping because");
    }

    std::vector<std::string> results;
    std::string fp = read_fingerprint(fd);
    uint32_t count = read_uint32(fd);

    for (uint32_t i = 0; i < count; i++) {
        results.push_back(read_string(fd));
    }
    
    return results;
}

void init_external_reverse_mapping() {
    TIMER_DECLARE(1);
    TIMER_BEGIN(1);

    is_changed = false;
    is_initialized = true;

    if (access(EXTERNAL_REVERSE_MAPPING_PATH, 0) != 0) {
        mkdir(EXTERNAL_REVERSE_MAPPING_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
        return;
    }

    int fd = open(EXTERNAL_REVERSE_MAPPING_PATH, O_RDONLY);
    if (fd < 0) {
        perror("could not open reverse mapping because");
        exit(1);
    }

    uint32_t key_num = read_uint32(fd);
    for (uint32_t i = 0; i < key_num; i++)
    {
        std::string fp = read_fingerprint(fd);
        uint32_t value_num = read_uint32(fd);
        keys.insert(std::pair<std::string, uint32_t>(fp, value_num));
        for (uint32_t j = 0; j < value_num; j++)
        {
            reverse_mapping.insert(std::pair<std::string, std::string>(fp, read_string(fd)));
        }
    }

    close(fd);

    TIMER_END(1, jcr.external_reverse_mapping_load_time);
}

void close_external_reverse_mapping(bool is_read_optimized) {
    if (!is_changed) {
        return;
    }

    TIMER_DECLARE(1);
    TIMER_BEGIN(1);

    if (access(EXTERNAL_REVERSE_MAPPING_PATH, 0) != 0) {
        mkdir(EXTERNAL_REVERSE_MAPPING_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    int fd = open(EXTERNAL_REVERSE_MAPPING_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("could not open reverse mapping because");
        exit(1);
    }
    int fd_index;
    if (is_read_optimized) {
        fd_index = open(EXTERNAL_REVERSE_MAPPING_MINI_INDEX_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_index < 0) {
            perror("could not open reverse mapping because");
            exit(1);
        }
        write_uint32(fd_index, keys.size());
    }

    write_uint32(fd, keys.size());
    for (auto key_count_it = keys.begin(); key_count_it != keys.end(); ++key_count_it)
    {
        const std::string& fp = key_count_it->first;
        const uint32_t count = key_count_it->second;
        if (is_read_optimized) {
            write_fingerprint(fd_index, fp);
            write_uint64(fd_index, lseek(fd, 0, SEEK_CUR));
        }
        write_fingerprint(fd, fp);
        write_uint32(fd, count);
        auto range_it = reverse_mapping.equal_range(fp);
        auto fp_begin = range_it.first;
        auto fp_end = range_it.second;
        for (; fp_begin != fp_end; ++(fp_begin))
        {
            write_string(fd, fp_begin->second);
        }
    }

    close(fd);
    if (is_read_optimized) {
        close(fd_index);
    }

    TIMER_END(1, jcr.external_reverse_mapping_save_time);
}

void insert_external_reverse_mapping(const std::string& fp, const std::string& file_path) {
    is_changed = true;
    auto it = keys.find(fp);
    if (it == keys.end()) {
        keys.insert(std::pair<std::string, uint32_t>(fp, 1));
    } else {
        it->second++;
    }

    reverse_mapping.insert(std::pair<std::string, std::string>(fp, file_path));
}

std::vector<std::wstring> get_external_reverse_mapping(const std::string& fp) {
    std::vector<std::wstring> ret;
    auto range_it = reverse_mapping.equal_range(fp);
        for (; range_it.first != range_it.second; 
                ++(range_it.first)) {
            ret.push_back(toWstring(range_it.first->second));
        }
    return ret;
}

std::set<std::string> get_external_reverse_mapping_multiple(const std::vector<std::string>& fps, bool trace_new_results) {
    FILE* trace_fp;
    struct timeval begin, now;

    if (trace_new_results) {
        trace_fp = fopen("new_results_trace.out", "w");
        gettimeofday(&begin, NULL);
    }
    std::set<std::string> ret;
    
    if (is_initialized) {
        for (const auto& fp : fps) {
            auto range_it = reverse_mapping.equal_range(fp);
            for (; range_it.first != range_it.second; 
                    ++(range_it.first)) {
                auto result = range_it.first->second;
                bool new_result = ret.insert(result).second;
                if (trace_new_results && new_result) {
                    gettimeofday(&now, NULL);
                    double diff = now.tv_usec - begin.tv_usec + 1000000 * (now.tv_sec - begin.tv_sec);
                    diff /= 1000000;
                    fprintf(trace_fp, "%.6f\n", diff);
                }
            }
        }
    } else {
        int fd = open(EXTERNAL_REVERSE_MAPPING_PATH, O_RDONLY);
        if (fd < 0) {
            perror("could not open reverse mapping because");
            exit(1);
        }
        double get_offsets_time = 0;
        double get_files_time = 0;
        TIMER_DECLARE(1);
        TIMER_DECLARE(2);
        TIMER_BEGIN(1);
        auto offsets = get_fingerprints_offsets(fps);
        TIMER_END(1, get_offsets_time);
        TIMER_BEGIN(2);
        for (auto offset : offsets) {
            auto results = get_paths_from_offset(fd, offset);
            for (auto result : results) {
                bool new_result = ret.insert(result).second;
                if (trace_new_results && new_result) {
                    gettimeofday(&now, NULL);
                    double diff = now.tv_usec - begin.tv_usec + 1000000 * (now.tv_sec - begin.tv_sec);
                    diff /= 1000000;
                    fprintf(trace_fp, "%.6f\n", diff);
                }
            }
        }
        TIMER_END(2, get_files_time);
        close(fd);
        
        jcr.reverse_mapping_get_offsets_time = get_offsets_time / 1000000;
        jcr.reverse_mapping_get_files_time = get_files_time / 1000000;
    }
    if (trace_new_results) {
        fclose(trace_fp);
    }
    return ret;
}