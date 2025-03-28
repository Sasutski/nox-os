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

#endif /* MEMORY_H */