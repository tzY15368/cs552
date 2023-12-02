#define FS_H 1
#define RAMDISK_SIZE 2 * 1048576 //2MB
#define RAMDISK_BLK_SIZE 256 // 256B

#define N_BLOCKS 8192 // 2MB/256B
#define N_INODES 1024 // each inode=64B, total 256blocks, each block=256B, total 1024 inodes

#define SUPERBLOCK_BLOCKS 1 // 1 block for superblock
#define BITMAP_BLOCKS 4 // 4 blocks for bitmap

#define INODE_TYPE_DIR 0
#define INODE_TYPE_REG 1

#ifndef UTILS_H
#include "utils.h"
#endif

static uint8_t ramdisk[RAMDISK_SIZE];

typedef struct block{
	uint8_t data_byte[RAMDISK_BLK_SIZE];
}block_t;

typedef struct super_block{
    int free_blocks;
    int free_inodes;
    uint8_t pad[248];
} __attribute__((packed)) superblock_t;

typedef struct inode{
    uint32_t type;   		// 4B, 0 for dir, 1 for reg
    uint32_t size; 			// 4B
    block_t* location[10]; 	// 40B, first 8: direct, 9th: single indirect, 10th: double indirect
    uint8_t pad[16]              // 16B
} __attribute__((packed)) inode_t; // must be 64B

typedef struct dir_entry{
	uint16_t inode_num; 		//Index into the inode array
	char filename[14]; 		//Directory name
} __attribute__((packed)) dir_entry_t;

typedef struct partition{
	superblock_t superblock; 	//256 bytes
	inode_t inode[N_INODES]; 		//256 blocks * 256 bytes/block / 64 bytes/inode = 1024 inodes
	block_t blocks[N_BLOCKS - SUPERBLOCK_BLOCKS - N_INODES - BITMAP_BLOCKS]; 		//(2MB - (256 + 256*256 + 1024)) / 256
	uint8_t bitmap[N_BLOCKS / 8]; 		    //1 bit per block
}  __attribute__((packed)) partition_t;

static partition_t* ramfs = (partition_t*)ramdisk; 				//Our partition
static inode_t* root_inode;

// TODO: test this
void __bitmap_set(uint8_t* bitmap, int block_num, int bit_value){
    int byte_num = block_num / 8;
    int bit_num = block_num % 8;
    if(bit_value == 1){
        bitmap[byte_num] |= (1 << bit_num);
    }else{
        bitmap[byte_num] &= ~(1 << bit_num);
    }
}

void ramdisk_init(){
    for(int i = 0; i < RAMDISK_SIZE; i++){
        ramdisk[i] = 0;
    }

    // first 256B (first block) is superblock
    // next 256 blocks: inode[]
    // 4 blocks for bitmap
    ramfs->superblock.free_inodes = N_INODES;
    ramfs->superblock.free_blocks = N_BLOCKS - SUPERBLOCK_BLOCKS - N_INODES - BITMAP_BLOCKS;

    // setup root inode
    root_inode = &ramfs->inode[0];
    root_inode->type = INODE_TYPE_DIR;
    root_inode->size = 0;
    root_inode->location[0] = &ramfs->blocks[0];
    // setup bitmap for block 0 to "using"
    __bitmap_set(ramfs->bitmap, 0, 1);

    tprintf("rd init @ %d = %dKB\n", ramdisk, RAMDISK_SIZE / 1024);
}



int rd_creat(char *pathname);
int rd_mkdir(char *pathname);
int rd_open(char *pathname, int flags);
int rd_close(int fd);
int rd_read(int fd, char *address, int num_bytes);
int rd_write(int fd, char *address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char *pathname);
int rd_readdir(int fd, char *address);