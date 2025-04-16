/* kernel.c - Main kernel entry point */
#include "keyboard.h"
#include "memory.h"  // Add this line

/* Video memory address */
#define VIDEO_MEMORY 0xB8000
/* Color: white on black */
#define COLOR 0x0F

/* Function prototypes - declare these before using them */
void update_cursor();
void outb(unsigned short port, unsigned char value);
int strcmp(const char* str1, const char* str2);
void execute_command(char* command);
void scroll_screen();
void init_vga_cursor();
void print_int(int num);  // Add this for the integer printing function

/* Debug function prototypes */
void* page_alloc_debug();
void* page_alloc_multiple_debug(int count);
int page_free_debug(void* addr);
void print_memory_debug_info();

/* Current cursor position */
int cursor_x = 0;
int cursor_y = 0;

/* History buffer */
#define HISTORY_SIZE 5
static char history[HISTORY_SIZE][256];
static int history_count = 0;
static int history_index = 0;

/* Function to write a character to video memory */
void putchar(char c, int x, int y) {
    unsigned char *video_memory = (unsigned char*)VIDEO_MEMORY;
    int offset = (y * 80 + x) * 2;
    video_memory[offset] = c;
    video_memory[offset + 1] = COLOR;
}

/* Function to clear the screen */
void clear_screen() {
    unsigned char *video_memory = (unsigned char*)VIDEO_MEMORY;
    for(int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = COLOR;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();  // Update the hardware cursor
}

/* Function to print a string at specific position */
void print_string(const char *str, int x, int y) {
    int i = 0;
    while(str[i] != '\0') {
        putchar(str[i], x + i, y);
        i++;
    }
}

/* Print a character at the current cursor position and update cursor */
void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        // Handle backspace - we need to handle it more simply
        if (cursor_x > 0) {
            // Move cursor back one position
            cursor_x--;
            
            // Erase the character (replace with space)
            putchar(' ', cursor_x, cursor_y);
        }
    } else {
        putchar(c, cursor_x, cursor_y);
        cursor_x++;
        
        // Handle line wrapping
        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    
    // Check if we need to scroll the screen
    if (cursor_y >= 25) {
        scroll_screen();
        cursor_y = 24; // Keep cursor at the last line
    }
    
    // Update the hardware cursor position
    update_cursor();
}

/* Scroll the screen up by one line */
void scroll_screen() {
    unsigned char *video_memory = (unsigned char*)VIDEO_MEMORY;
    
    // Move each line up one position
    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 80; x++) {
            int src_offset = ((y + 1) * 80 + x) * 2;
            int dest_offset = (y * 80 + x) * 2;
            
            video_memory[dest_offset] = video_memory[src_offset];
            video_memory[dest_offset + 1] = video_memory[src_offset + 1];
        }
    }
    
    // Clear the last line
    for (int x = 0; x < 80; x++) {
        int offset = (24 * 80 + x) * 2;
        video_memory[offset] = ' ';
        video_memory[offset + 1] = COLOR;
    }
}

/* Print a string at the current cursor position */
void print(const char *str) {
    int i = 0;
    while(str[i] != '\0') {
        print_char(str[i]);
        i++;
    }
}

/* Update the hardware cursor position to match our software cursor */
void update_cursor() {
    unsigned short position = (cursor_y * 80) + cursor_x;
    
    // Tell the VGA hardware the position
    outb(0x3D4, 0x0F);  // Low byte register
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);  // High byte register
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

/* Write a byte to an I/O port */
void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "dN"(port));
}

/* Read a byte from an I/O port */
unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

// Command comparison function
int strcmp(const char* str1, const char* str2) {
    while(*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// Execute commands
void execute_command(char* command) {
    if (strcmp(command, "clear") == 0) {
        clear_screen();
        print("NOX OS> ");
    }
    else if (strcmp(command, "help") == 0) {
        print("\nAvailable commands:\n");
        print("  clear    - Clear the screen\n");
        print("  help     - Display this help message\n");
        print("  memory   - Display memory statistics\n");
        print("  memcheck - Show detailed memory map\n");
        print("  pagetest - Test page allocation system\n");
        print("  quit     - Shutdown the system\n");
        print("  memprotect - Test memory protection system\n");
        print("  memdebug - Test memory debugging system\n");
        print("NOX OS> ");
    }
    else if (strcmp(command, "memory") == 0) {
        print_memory_stats();
        print("\nNOX OS> ");
    }
    else if (strcmp(command, "memcheck") == 0) {
        print_memory_stats();
        print_memory_map();
        print("\nNOX OS> ");
    }
    else if (strcmp(command, "pagetest") == 0) {
        print("\nTesting page allocation system...\n");
        
        // Allocate single pages
        print("Allocating 3 individual pages...\n");
        void* page1 = page_alloc();
        void* page2 = page_alloc();
        void* page3 = page_alloc();
        
        print("Page 1: ");
        print_int((unsigned int)page1);
        print("\nPage 2: ");
        print_int((unsigned int)page2);
        print("\nPage 3: ");
        print_int((unsigned int)page3);
        print("\n");
        
        // Allocate multiple pages
        print("\nAllocating 5 contiguous pages...\n");
        void* multi_page = page_alloc_multiple(5);
        print("Multi-page address: ");
        print_int((unsigned int)multi_page);
        print("\nPage count: ");
        print_int(get_page_count(multi_page));
        print("\n");
        
        // Free pages
        print("\nFreeing allocated pages...\n");
        page_free(page2);
        page_free(multi_page);
        
        // Display memory map after allocations and frees
        print_memory_map();
        
        print("\nNOX OS> ");
    }
    else if (strcmp(command, "memprotect") == 0) {
        print("\nTesting memory protection...\n");
        
        // Allocate a test page
        void* test_page = page_alloc();
        print("Allocated test page at: ");
        print_int((unsigned int)test_page);
        print("\n");
        
        // Set different permissions for regions within the page
        unsigned char* addr = (unsigned char*)test_page;
        
        // First 1KB: Read-only
        set_memory_permissions(addr, 1024, MEM_PERM_READ);
        print("Set first 1KB to read-only\n");
        
        // Next 1KB: Read-write
        set_memory_permissions(addr + 1024, 1024, MEM_PERM_RW);
        print("Set next 1KB to read-write\n");
        
        // Next 1KB: Read-execute
        set_memory_permissions(addr + 2048, 1024, MEM_PERM_RX);
        print("Set next 1KB to read-execute\n");
        
        // Last 1KB: No permissions
        set_memory_permissions(addr + 3072, 1024, 0);
        print("Set last 1KB to no-access\n");
        
        // Print region information
        print_memory_protection_info();
        
        // Test reading from each region
        print("\nTesting memory access:\n");
        
        print("Read from read-only region: ");
        int result = validate_memory_access(addr, 4, MEM_PERM_READ);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        print("Write to read-only region: ");
        result = validate_memory_access(addr, 4, MEM_PERM_WRITE);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        print("Execute from read-only region: ");
        result = validate_memory_access(addr, 4, MEM_PERM_EXEC);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        print("Read from read-write region: ");
        result = validate_memory_access(addr + 1024, 4, MEM_PERM_READ);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        print("Write to read-write region: ");
        result = validate_memory_access(addr + 1024, 4, MEM_PERM_WRITE);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        print("Read from no-access region: ");
        result = validate_memory_access(addr + 3072, 4, MEM_PERM_READ);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        // Test out-of-bounds access
        print("Access beyond region boundary: ");
        result = validate_memory_access(addr + 1020, 8, MEM_PERM_READ);
        if (result == MEM_PROT_OK) print("Allowed\n"); else print("Denied\n");
        
        // Free the test page
        page_free(test_page);
        
        print("\nNOX OS> ");
    }
    else if (strcmp(command, "memdebug") == 0) {
        print("\nTesting memory debugging...\n");
        // Allocate a page
        void* test_page = page_alloc_debug();
        print("Allocated debug page at: ");
        print_int((unsigned int)test_page);
        print("\n");
        
        // Print debug info
        print_memory_debug_info();

        // Free the page
        page_free_debug(test_page);
        
        // Print debug info again
        print_memory_debug_info();
        
        print("\nNOX OS> ");
    }
    else if (strcmp(command, "quit") == 0) {
        print("\nShutting down...\n");
        // Tell QEMU to power off
        __asm__ volatile("outw %%ax, %%dx" : : "a"((unsigned short)0x2000), "d"((unsigned short)0x604));
        // Backup halt if that fails
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
        while(1) { }
    }
    else if (command[0] != '\0') {
        print("\nUnknown command: ");
        print(command);
        print("\nNOX OS> ");
    }
    else {
        print("\nNOX OS> ");
    }
}

/* Initialize VGA cursor */
void init_vga_cursor() {
    // Enable cursor, set scanline start/end
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 0x0F);
}

/* Function to print an integer at the current cursor position */
void print_int(int num) {
    char buffer[16];
    int i = 0;
    int is_negative = 0;
    
    // Handle 0 as a special case
    if (num == 0) {
        print("0");
        return;
    }
    
    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Convert integer to string (in reverse)
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    // Print in correct order (reversing the buffer)
    while (i > 0) {
        print_char(buffer[--i]);
    }
}

/* Function to handle command input without arrow keys */
void kernel_main() {
    init_vga_cursor();
    clear_screen();
    
    print("Welcome to NOX OS!\n");
    
    // Initialize memory system
    init_memory();
    init_memory_protection();
    
    print("Type 'help' for a list of commands\n\n");
    print("NOX OS> ");
    
    char command_buffer[256];
    int buffer_pos = 0;
    for (int i = 0; i < 256; i++)
        command_buffer[i] = '\0';
    
    while(1) {
        unsigned char key = get_key();
        if (key != 0) {
            if (key == KEY_LEFT) {
                if (buffer_pos > 0) {
                    buffer_pos--;
                    cursor_x--;
                    update_cursor();
                }
            }
            else if (key == KEY_RIGHT) {
                if (command_buffer[buffer_pos] != '\0') {
                    buffer_pos++;
                    cursor_x++;
                    update_cursor();
                }
            }
            else if (key == KEY_HOME) {
                // Go to start of line
                buffer_pos = 0;
                cursor_x = 8; // After the prompt "NOX OS> "
                update_cursor();
            }
            else if (key == KEY_END) {
                // Go to end of current text
                buffer_pos = 0;
                while (command_buffer[buffer_pos] != '\0')
                    buffer_pos++;
                cursor_x = 8 + buffer_pos;
                update_cursor();
            }
            else if (key == KEY_UP) {
                // Show previous command in history
                if (history_index > 0) {
                    history_index--;
                    
                    // Clear entire current line first
                    // Move cursor to beginning of line
                    cursor_x = 0;
                    update_cursor();
                    
                    // Clear the entire line
                    for (int i = 0; i < 80; i++) {
                        putchar(' ', i, cursor_y);
                    }
                    
                    // Redraw the prompt
                    print_string("NOX OS> ", 0, cursor_y);
                    
                    // Reset cursor position
                    cursor_x = 8; // After the prompt (including the space)
                    update_cursor();
                    
                    // Clear command buffer
                    for (int i = 0; i < 256; i++) 
                        command_buffer[i] = '\0';
                    
                    // Copy from history
                    for (int i = 0; i < 256; i++)
                        command_buffer[i] = history[history_index % HISTORY_SIZE][i];
                    
                    // Print it
                    buffer_pos = 0;
                    while (command_buffer[buffer_pos] != '\0') {
                        print_char(command_buffer[buffer_pos]);
                        buffer_pos++;
                    }
                }
            }
            else if (key == KEY_DOWN) {
                // Show next command if available
                if (history_index < history_count) {
                    history_index++;
                    
                    // Clear entire current line first
                    // Move cursor to beginning of line
                    cursor_x = 0;
                    update_cursor();
                    
                    // Clear the entire line
                    for (int i = 0; i < 80; i++) {
                        putchar(' ', i, cursor_y);
                    }
                    
                    // Redraw the prompt
                    print_string("NOX OS> ", 0, cursor_y);
                    
                    // Reset cursor position
                    cursor_x = 8; // After the prompt (including the space)
                    update_cursor();
                    
                    // Clear command buffer
                    for (int i = 0; i < 256; i++)
                        command_buffer[i] = '\0';
                    
                    if (history_index < history_count) {
                        // Copy from history
                        for (int i = 0; i < 256; i++)
                            command_buffer[i] = history[history_index % HISTORY_SIZE][i];
                    }
                    
                    // Print it (might be empty if we went past the end of history)
                    buffer_pos = 0;
                    while (command_buffer[buffer_pos] != '\0') {
                        print_char(command_buffer[buffer_pos]);
                        buffer_pos++;
                    }
                }
            }
            else if (key == KEY_DELETE) {
                // Delete the character at the current cursor position
                if (command_buffer[buffer_pos] != '\0') {
                    // Shift all characters after cursor to the left
                    for (int i = buffer_pos; i < 255; i++) {
                        command_buffer[i] = command_buffer[i+1];
                    }
                    
                    // Clear the rest of the line
                    int current_x = cursor_x;
                    for (int i = current_x; i < 80; i++) {
                        putchar(' ', i, cursor_y);
                    }
                    
                    // Redraw from current position
                    int temp_pos = buffer_pos;
                    cursor_x = current_x;
                    update_cursor();
                    
                    while (command_buffer[temp_pos] != '\0') {
                        putchar(command_buffer[temp_pos], cursor_x, cursor_y);
                        temp_pos++;
                        cursor_x++;
                    }
                    
                    // Reset cursor to the correct position
                    cursor_x = current_x;
                    update_cursor();
                }
            }
            else if (key == '\n') {
                print_char('\n');
                
                // Store in history - we'll use the full command
                if (buffer_pos > 0 || command_buffer[0] != '\0') {
                    for(int i=0; i<256; i++)
                        history[history_count % HISTORY_SIZE][i] = command_buffer[i];
                    history_count++;
                    history_index = history_count;
                }
                
                execute_command(command_buffer);
                buffer_pos = 0;
                for (int i = 0; i < 256; i++)
                    command_buffer[i] = '\0';
            }
            else if (key == '\b') {
                if (buffer_pos > 0) {
                    // Remove the character at cursor position - 1
                    buffer_pos--;
                    
                    // Shift all characters to the left
                    for (int i = buffer_pos; i < 255; i++) {
                        command_buffer[i] = command_buffer[i+1];
                    }
                    
                    // Move cursor back visually
                    cursor_x--;
                    update_cursor();
                    
                    // Clear the rest of the line
                    int current_x = cursor_x;
                    for (int i = current_x; i < 80; i++) {
                        putchar(' ', i, cursor_y);
                    }
                    
                    // Redraw from current position
                    int temp_pos = buffer_pos;
                    cursor_x = current_x;
                    update_cursor();
                    
                    while (command_buffer[temp_pos] != '\0') {
                        putchar(command_buffer[temp_pos], cursor_x, cursor_y);
                        temp_pos++;
                        cursor_x++;
                    }
                    
                    // Reset cursor to the correct position
                    cursor_x = current_x;
                    update_cursor();
                }
            }
            // Only handle normal ASCII characters (32-126) for typing
            else if (key >= 32 && key <= 126) {
                if (buffer_pos < 255) {
                    // Make room for the new character by shifting everything right
                    for (int i = 255; i > buffer_pos; i--) {
                        command_buffer[i] = command_buffer[i-1];
                    }
                    
                    // Insert the character
                    command_buffer[buffer_pos] = key;
                    buffer_pos++;
                    
                    // Redraw the line from current position
                    print_char(key);
                    
                    // Print the rest of the line
                    int temp_pos = buffer_pos;
                    while (command_buffer[temp_pos] != '\0') {
                        print_char(command_buffer[temp_pos]);
                        temp_pos++;
                    }
                    
                    // Move cursor back to correct position
                    for (int i = temp_pos; i > buffer_pos; i--) {
                        cursor_x--;
                        update_cursor();
                    }
                }
            }
        }
    }
}
