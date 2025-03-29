#ifndef MEMORY_H
#define MEMORY_H

/* Define our own size_t since we don't have stddef.h */
typedef unsigned int size_t;

/* Memory constants */
#define PAGE_SIZE 4096                  /* 4KB pages */
#define HEAP_START 0x100000             /* Start at 1MB */
#define HEAP_INITIAL_SIZE 0x100000      /* 1MB initial heap */
#define BITMAP_SIZE (HEAP_INITIAL_SIZE / PAGE_SIZE / 8)  /* Bitmap size in bytes */

/* Memory allocation error codes */
#define MEM_OK 0
#define MEM_ERR_NO_MEM 1
#define MEM_ERR_INVALID_ADDR 2
#define MEM_ERR_DOUBLE_FREE 3

/* Memory protection flags */
#define MEM_PERM_READ    0x01
#define MEM_PERM_WRITE   0x02
#define MEM_PERM_EXEC    0x04
#define MEM_PERM_RW      (MEM_PERM_READ | MEM_PERM_WRITE)
#define MEM_PERM_RX      (MEM_PERM_READ | MEM_PERM_EXEC)
#define MEM_PERM_RWX     (MEM_PERM_READ | MEM_PERM_WRITE | MEM_PERM_EXEC)

/* Memory protection error codes */
#define MEM_PROT_OK            0
#define MEM_PROT_INVALID_ADDR  1
#define MEM_PROT_PERM_DENIED   2
#define MEM_PROT_OUT_OF_BOUNDS 3

/* Memory region structure */
typedef struct {
    void*  start;       // Start address of region
    void*  end;         // End address of region
    unsigned char perm; // Permissions (read/write/exec)
} mem_region_t;

/* Function prototypes */
void init_memory();
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);
void print_memory_stats();
void print_memory_map();

/* Page allocation functions */
void* page_alloc();                  /* Allocate a single page */
void* page_alloc_multiple(int count);/* Allocate multiple contiguous pages */
int page_free(void* addr);           /* Free a page or pages */
int page_is_allocated(void* addr);   /* Check if a page is allocated */
int get_page_count(void* addr);      /* Get number of pages for an allocation */

/* Memory protection function prototypes */
void init_memory_protection();
int set_memory_permissions(void* addr, size_t size, unsigned char perm);
int check_memory_access(void* addr, size_t size, unsigned char access_type);
int validate_memory_access(void* addr, size_t size, unsigned char access_type);
void print_memory_protection_info();

#endif /* MEMORY_H */