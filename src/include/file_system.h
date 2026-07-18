#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../include/ata_pio.h"
#include "../include/kstring.h"
enum fs_error {
    FS_SUCCESS = 0,
    FS_FILE_NOT_FOUND = -1,
    FS_READ_ERROR = -2,
    FS_INVALID_PATH = -3
};

enum FileType {
    FILE_TYPE_REGULAR = 0,
    FILE_TYPE_DIRECTORY = 1
};
typedef struct fat32 {
    char data[16]; // filename (null-terminated)
    uint32_t size; // file size in bytes
    uint32_t data_sector; // starting sector on disk
    uint8_t type; // FILE_TYPE_REGULAR or FILE_TYPE_DIRECTORY

    struct fat32* parent;
    struct fat32* root;
    struct fat32* children; // only for directories
    uint32_t child_count;
} fat32;
void fs_save();
void fs_init();
int fs_read_file(const char* path, uint8_t* buffer, uint32_t max_size);
int fs_write_file(const char* path, const uint8_t* data, uint32_t size);
int fs_list_directory(const char* path, fat32* entries, uint32_t max_entries);
#endif // FILE_SYSTEM_H