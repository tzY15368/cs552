#define TYPES_H 1


#define STACK_SIZE 4096 // 4kb stack
#define FDS_PER_THREAD 1024
#define N_THREADS 2

#define FALSE 0
#define TRUE 1

#define NULL 0
typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;

#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

typedef struct context{
	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
//	uint32_t flags;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
} context_t;

enum thread_state {
    IDLE,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};




// FSTYPES
#define RAMDISK_SIZE 2 * 1048576 //2MB
#define RAMDISK_BLK_SIZE 256 // 256B

#define N_BLOCKS 8192 // 2MB/256B
#define N_INODES 1024 // each inode=64B,  each block=256B, total 256blocks, total 1024 inodes

#define INODE_BLOCKS 256 // 256 blocks for inode
#define SUPERBLOCK_BLOCKS 1 // 1 block for superblock
#define BITMAP_BLOCKS 4 // 4 blocks for bitmap

#define INODE_TYPE_DIR 0
#define INODE_TYPE_REG 1

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
    uint8_t in_use;         // 8B, 0 for not in use, 1 for in use
    uint8_t ref_cnt;        // 8B
} __attribute__((packed)) inode_t; // must be 64B

typedef struct dir_entry{
	uint16_t inode_num; 		//Index into the inode array
	char filename[14]; 		//Directory name
} __attribute__((packed)) dir_entry_t;

typedef struct partition{
	superblock_t superblock; 	//256 bytes
	inode_t inode[N_INODES]; 		//256 blocks * 256 bytes/block / 64 bytes/inode = 1024 inodes
	block_t blocks[N_BLOCKS - SUPERBLOCK_BLOCKS - INODE_BLOCKS - BITMAP_BLOCKS]; 		//(2MB - (256 + 256*256 + 1024)) / 256
	uint8_t bitmap[N_BLOCKS / 8]; 		    //1 bit per block
}  __attribute__((packed)) partition_t;


typedef struct file_descriptor{
    inode_t* inode;
    bool in_use;
    int fd_num;
    int offset;
} file_descriptor_t;

static partition_t* ramfs = (partition_t*)ramdisk; 				//Our partition
static inode_t* root_inode;















// Example state information includes copies of machine register values
// such as the stack and instruction pointers (possibly others too), and a thread ID (TID).
typedef struct thread_ctl_blk {
    size_t tp_idx;
    size_t id;
    enum thread_state state;
    uint32_t stack[STACK_SIZE];
    uint32_t bp;
    uint32_t esp;
    uint32_t func;
    context_t* ctx; // points to the context on the stack, not an actual context
    file_descriptor_t fds[FDS_PER_THREAD];
} thread_ctl_blk_t;

typedef struct thread_pool{
    size_t size;
    size_t idle_cnt;
    thread_ctl_blk_t threads[N_THREADS];
    int next_tid;
} thread_pool_t;

typedef struct ready_queue{
    size_t size;
    thread_ctl_blk_t* queue[N_THREADS];
    int queue_head;
    int queue_tail;
} ready_queue_t;

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

