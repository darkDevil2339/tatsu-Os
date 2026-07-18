#include "../include/ata_pio.h"
#include "../include/file_system.h"
#include "../include/kstring.h"
#define FS_SIGNATURE "SACHINFS" // "SACHINFS" in hex
struct superblock {
    char signature[8]; // "SACHINFS"
    uint32_t total_blocks;
    uint32_t block_size;
    uint32_t data_start_block;
};
typedef enum {
    Node_FILE,
    Node_DIRECTORY
} NodeType;
typedef struct File{
    char name[25];
    int permission;
    int size;
    char identifier;
    int block;
    char extension[5];
} File;

typedef struct Folder Folder;
typedef struct Node {
    NodeType type;

    union {
        File file;
        Folder *folder;
    } data;

    struct Node *next;
} Node;
typedef struct Folder Folder;
struct Folder {
    char name[32];
    int permission;
    int identifier;

    Node *children;
};
void format_disk() {
    struct superblock sb;
    memcpy(sb.signature, FS_SIGNATURE, 8);
    sb.total_blocks = 1024; // Example size
    sb.block_size = 512; // Standard sector size
    sb.data_start_block = 10; // Superblock is block 0

    uint8_t buffer[512];
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &sb, sizeof(sb));

    ata_write_sector(1, buffer); // Write superblock to disk
}
uint32_t read_superblock() {
    uint8_t buffer[512];
    ata_read_sector(1, buffer);
    struct superblock* sb = (struct superblock*)buffer;
    if (memcmp(sb->signature, FS_SIGNATURE, 8) == 0) {
        return sb->data_start_block;
    } else {
        return 0; // Invalid superblock
    }
}

void fs_init() {
    if((int)read_superblock() == 0) {
        format_disk();
    }

    
}