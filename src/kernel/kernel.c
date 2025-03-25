/* kernel.c - Main kernel entry point */
#include "keyboard.h"

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

/* Current cursor position */
int cursor_x = 0;
int cursor_y = 0;

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
        // Handle backspace
        // First check if we're in a user-editable area (below line 14 or at line 15+)
        if (cursor_y > 14 || (cursor_y == 14 && cursor_x > 0)) {
            if (cursor_x > 0) {
                // Move cursor back one position
                cursor_x--;
                
                // Erase the character (replace with space)
                putchar(' ', cursor_x, cursor_y);
            } else if (cursor_y > 15) {  // Only allow line wrapping within user input area
                // If at beginning of line and not first input line, go to end of previous line
                cursor_y--;
                cursor_x = 79;  // Last position on previous line
                putchar(' ', cursor_x, cursor_y);
            }
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
        print("  clear - Clear the screen\n");
        print("  help  - Display this help message\n");
        print("NOX OS> ");
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

/* Function to handle command input without arrow keys */
void kernel_main() {
    clear_screen();
    print("Welcome to NOX OS!\n");
    print("Type 'help' for a list of commands\n\n");
    print("NOX OS> ");
    
    char command_buffer[256];
    int buffer_pos = 0;
    
    // Initialize command buffer
    for (int i = 0; i < 256; i++)
        command_buffer[i] = '\0';
    
    while(1) {
        char key = get_key();
        
        if (key != 0) {
            if (key == '\n') {
                // Execute command
                print_char('\n');
                command_buffer[buffer_pos] = '\0';
                execute_command(command_buffer);
                
                // Reset buffer for new command
                buffer_pos = 0;
                for (int i = 0; i < 256; i++)
                    command_buffer[i] = '\0';
            }
            else if (key == '\b') {
                // Handle backspace - simpler approach
                if (buffer_pos > 0) {
                    buffer_pos--;
                    command_buffer[buffer_pos] = '\0';
                    print_char('\b'); // Let print_char handle cursor movement
                }
            }
            else if (key >= 32 && key <= 126) {
                // Only append to the end - no complex insertion
                if (buffer_pos < 255) { // Prevent buffer overflow
                    command_buffer[buffer_pos] = key;
                    buffer_pos++;
                    command_buffer[buffer_pos] = '\0';
                    print_char(key); // Let print_char handle cursor movement
                }
            }
        }
    }
}
