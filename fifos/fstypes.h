// #define RAMDISK_SIZE 2 * 1048576 //2MB
// #define RAMDISK_BLK_SIZE 256 // 256B

// #define N_BLOCKS 8192 // 2MB/256B
// #define N_INODES 1024 // each inode=64B,  each block=256B, total 256blocks, total 1024 inodes

// #define INODE_BLOCKS 256 // 256 blocks for inode
// #define SUPERBLOCK_BLOCKS 1 // 1 block for superblock
// #define BITMAP_BLOCKS 4 // 4 blocks for bitmap

// #define INODE_TYPE_DIR 0
// #define INODE_TYPE_REG 1

// #define FSTYPES_H 1

// #ifndef TYPES_H
// #include "types.h"
// #endif

// static uint8_t ramdisk[RAMDISK_SIZE];

// typedef struct block{
// 	uint8_t data_byte[RAMDISK_BLK_SIZE];
// }block_t;

// typedef struct super_block{
//     int free_blocks;
//     int free_inodes;
//     uint8_t pad[248];
// } __attribute__((packed)) superblock_t;

// typedef struct inode{
//     uint32_t type;   		// 4B, 0 for dir, 1 for reg
//     uint32_t size; 			// 4B
//     block_t* location[10]; 	// 40B, first 8: direct, 9th: single indirect, 10th: double indirect
//     uint8_t in_use;         // 8B, 0 for not in use, 1 for in use
//     uint8_t pad[8]              // 8B
// } __attribute__((packed)) inode_t; // must be 64B

// typedef struct dir_entry{
// 	uint16_t inode_num; 		//Index into the inode array
// 	char filename[14]; 		//Directory name
// } __attribute__((packed)) dir_entry_t;

// typedef struct partition{
// 	superblock_t superblock; 	//256 bytes
// 	inode_t inode[N_INODES]; 		//256 blocks * 256 bytes/block / 64 bytes/inode = 1024 inodes
// 	block_t blocks[N_BLOCKS - SUPERBLOCK_BLOCKS - INODE_BLOCKS - BITMAP_BLOCKS]; 		//(2MB - (256 + 256*256 + 1024)) / 256
// 	uint8_t bitmap[N_BLOCKS / 8]; 		    //1 bit per block
// }  __attribute__((packed)) partition_t;


// typedef struct file_descriptor{
//     inode_t* inode;
//     bool in_use;
//     int fd_num;
//     int offset;
// } file_descriptor_t;

// static partition_t* ramfs = (partition_t*)ramdisk; 				//Our partition
// static inode_t* root_inode;