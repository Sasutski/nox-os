#include "memory.h"

/* Memory bitmap - each bit represents a page
   0 = free page, 1 = used page */
static unsigned char mem_bitmap[BITMAP_SIZE];

/* Forward declaration of print function */
void print(const char *str);

// Define our own print_int function so we don't need to rely on external one
static void print_int(int num) {
    char buffer[12]; // Enough for 32-bit integers
    int i = 0;
    
    // Handle 0 specially
    if (num == 0) {
        print("0");
        return;
    }
    
    // Handle negative numbers
    if (num < 0) {
        print("-");
        num = -num;
    }
    
    // Convert to string (reversed)
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Print in correct order
    while (i > 0) {
        char c[2] = {buffer[--i], '\0'};
        print(c);
    }
}

/* Set a bit in the bitmap */
static void bitmap_set(int bit) {
    mem_bitmap[bit / 8] |= (1 << (bit % 8));
}

/* Clear a bit in the bitmap */
static void bitmap_clear(int bit) {
    mem_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

/* Test if a bit is set */
static int bitmap_test(int bit) {
    return mem_bitmap[bit / 8] & (1 << (bit % 8));
}

/* Find first free page (first clear bit) */
static int bitmap_first_free() {
    for (unsigned int i = 0; i < BITMAP_SIZE; i++) {
        if (mem_bitmap[i] != 0xFF) { // If not all bits are set
            for (int j = 0; j < 8; j++) {
                if (!(mem_bitmap[i] & (1 << j))) {
                    return i * 8 + j;
                }
            }
        }
    }
    return -1; // No free pages
}

/* Find first n contiguous free pages */
static int bitmap_first_free_s(int n) {
    int start = -1;
    int count = 0;
    
    for (unsigned int i = 0; i < BITMAP_SIZE * 8; i++) {
        if (!bitmap_test(i)) {
            // This bit is clear
            if (start == -1) {
                start = i;
            }
            count++;
            if (count == n) {
                return start;
            }
        } else {
            // This bit is set, reset our counters
            start = -1;
            count = 0;
        }
    }
    
    return -1; // Not enough contiguous free pages
}

/* Initialize memory management */
void init_memory() {
    // Clear the bitmap - all memory is free
    for (unsigned int i = 0; i < BITMAP_SIZE; i++) {
        mem_bitmap[i] = 0;
    }
    
    // Reserve the first page (NULL pointer protection)
    bitmap_set(0);
    
    print("Memory initialized: ");
    print_int((HEAP_INITIAL_SIZE / 1024));
    print(" KB available\n");
}

/* Allocate memory (in bytes) - returns pointer to allocated memory */
void* kmalloc(size_t size) {
    if (size == 0) return 0;
    
    // Calculate pages needed (round up)
    int pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Use our page allocation function
    return page_alloc_multiple(pages);
}

/* Free allocated memory */
void kfree(void* ptr) {
    if (ptr == 0) return;
    
    // Use our page free function and check for errors
    int result = page_free(ptr);
    if (result != MEM_OK) {
        print("ERROR: Memory free failed with code ");
        print_int(result);
        print("\n");
    }
}

/* Reallocate memory - simple version */
void* krealloc(void* ptr, size_t size) {
    if (ptr == 0) {
        return kmalloc(size);
    }
    if (size == 0) {
        kfree(ptr);
        return 0;
    }
    
    // Allocate new block
    void* new_ptr = kmalloc(size);
    if (new_ptr == 0) {
        return 0;
    }
    
    // This is highly simplified - we don't know the original size
    // In a real implementation, we would store size metadata
    
    // Copy original data (assuming size is smaller than original)
    unsigned char* src = (unsigned char*)ptr;
    unsigned char* dst = (unsigned char*)new_ptr;
    
    // Copy conservatively (we don't know the exact original size)
    // This assumes the new allocation is smaller than or equal to the original
    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    
    // Free the old block
    kfree(ptr);
    
    return new_ptr;
}

/* Print memory statistics */
void print_memory_stats() {
    int used_pages = 0;
    int total_pages = HEAP_INITIAL_SIZE / PAGE_SIZE;
    
    // Fix signedness comparison (cast total_pages to unsigned int)
    for (unsigned int i = 0; i < (unsigned int)total_pages; i++) {
        if (bitmap_test(i)) {
            used_pages++;
        }
    }
    
    print("\nMemory Statistics:\n");
    print("  Total memory: ");
    print_int(HEAP_INITIAL_SIZE / 1024);
    print(" KB\n");
    
    print("  Used memory: ");
    print_int((used_pages * PAGE_SIZE) / 1024);
    print(" KB (");
    print_int(used_pages);
    print(" pages)\n");
    
    print("  Free memory: ");
    print_int(((total_pages - used_pages) * PAGE_SIZE) / 1024);
    print(" KB (");
    print_int(total_pages - used_pages);
    print(" pages)\n");
}

/* Print a visual map of memory usage */
void print_memory_map() {
    int total_pages = HEAP_INITIAL_SIZE / PAGE_SIZE;
    int pages_per_line = 64;
    int lines = (total_pages + pages_per_line - 1) / pages_per_line;
    
    print("\nMemory Map (each character represents 1 page):\n");
    print("  [.] free   [#] used\n\n  ");
    
    for (int i = 0; i < total_pages; i++) {
        // Print a character for each page
        if (bitmap_test(i)) {
            print("#");  // Used page
        } else {
            print(".");  // Free page
        }
        
        // Add line breaks for readability
        if ((i + 1) % pages_per_line == 0 && i < total_pages - 1) {
            print("\n  ");
        }
    }
    print("\n");
    
    // Find largest contiguous free block
    int max_free = 0;
    int current_free = 0;
    
    for (int i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            current_free++;
            if (current_free > max_free) {
                max_free = current_free;
            }
        } else {
            current_free = 0;
        }
    }
    
    print("\nLargest contiguous free block: ");
    print_int(max_free * PAGE_SIZE / 1024);
    print(" KB (");
    print_int(max_free);
    print(" pages)\n");
    
    // Calculate fragmentation
    int free_blocks = 0;
    int free_pages = 0;
    int in_free_block = 0;
    
    for (int i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            free_pages++;
            if (!in_free_block) {
                free_blocks++;
                in_free_block = 1;
            }
        } else {
            in_free_block = 0;
        }
    }
    
    print("Memory fragmentation: ");
    if (free_pages > 0) {
        print_int(free_blocks);
        print(" free blocks across ");
        print_int(free_pages);
        print(" pages\n");
    } else {
        print("N/A (no free memory)\n");
    }
}

/* Page allocation functions */

/* Allocate a single page */
void* page_alloc() {
    int page_index = bitmap_first_free();
    if (page_index == -1) {
        print("ERROR: Out of memory in page_alloc()\n");
        return 0;
    }
    
    bitmap_set(page_index);
    void* addr = (void*)(HEAP_START + page_index * PAGE_SIZE);
    
    // Zero out the page for security
    unsigned char* page = (unsigned char*)addr;
    for (size_t i = 0; i < PAGE_SIZE; i++) {
        page[i] = 0;
    }
    
    return addr;
}

/* Allocate multiple contiguous pages */
void* page_alloc_multiple(int count) {
    if (count <= 0) {
        return 0;
    }
    
    int page_index = bitmap_first_free_s(count);
    if (page_index == -1) {
        print("ERROR: Cannot allocate ");
        print_int(count);
        print(" contiguous pages\n");
        return 0;
    }
    
    // Mark pages as used
    for (int i = 0; i < count; i++) {
        bitmap_set(page_index + i);
    }
    
    void* addr = (void*)(HEAP_START + page_index * PAGE_SIZE);
    
    // Zero out the pages
    unsigned char* pages = (unsigned char*)addr;
    for (size_t i = 0; i < (PAGE_SIZE * count); i++) {
        pages[i] = 0;
    }
    
    return addr;
}

/* Free a page or pages */
int page_free(void* addr) {
    if (addr == 0) return MEM_ERR_INVALID_ADDR;
    
    unsigned int address = (unsigned int)addr;
    if (address < HEAP_START) {
        print("ERROR: Invalid free - address below heap start\n");
        return MEM_ERR_INVALID_ADDR;
    }
    
    // Calculate page index
    int page_index = (address - HEAP_START) / PAGE_SIZE;
    
    // Check if the page is allocated
    if (!bitmap_test(page_index)) {
        print("ERROR: Double free detected in page_free()\n");
        return MEM_ERR_DOUBLE_FREE;
    }
    
    // Free this page and any contiguous pages allocated together
    bitmap_clear(page_index);
    
    // Free subsequent pages that were part of this allocation
    int i = page_index + 1;
    while (bitmap_test(i) && i < BITMAP_SIZE * 8) {
        bitmap_clear(i);
        i++;
    }
    
    return MEM_OK;
}

/* Check if a page is allocated */
int page_is_allocated(void* addr) {
    if (addr == 0) return 0;
    
    unsigned int address = (unsigned int)addr;
    if (address < HEAP_START) {
        return 0;
    }
    
    int page_index = (address - HEAP_START) / PAGE_SIZE;
    return bitmap_test(page_index);
}

/* Get number of pages for an allocation */
int get_page_count(void* addr) {
    if (addr == 0 || !page_is_allocated(addr)) {
        return 0;
    }
    
    unsigned int address = (unsigned int)addr;
    int page_index = (address - HEAP_START) / PAGE_SIZE;
    int count = 1;
    
    // Count contiguous allocated pages
    int i = page_index + 1;
    while (bitmap_test(i) && i < BITMAP_SIZE * 8) {
        count++;
        i++;
    }
    
    return count;
}