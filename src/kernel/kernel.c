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
void init_vga_cursor();

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
        print("  clear - Clear the screen\n");
        print("  help  - Display this help message\n");
        print("  quit  - Shutdown the system\n");
        print("NOX OS> ");
    }
    else if (strcmp(command, "quit") == 0) {
        print("\nShutting down...\n");
        // Halt the CPU - this will stop execution in most emulators
        __asm__ volatile("cli");  // Disable interrupts
        __asm__ volatile("hlt");  // Halt the CPU
        // If we get here, the halt didn't work
        while(1) { /* Infinite loop as fallback */ }
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

/* Function to handle command input without arrow keys */
void kernel_main() {
    init_vga_cursor();
    clear_screen();
    print("Welcome to NOX OS!\n");
    print("Type 'help' for a list of commands\n\n");
    print("NOX OS> ");
    
    char command_buffer[256];
    int buffer_pos = 0;
    for (int i = 0; i < 256; i++)
        command_buffer[i] = '\0';
    
    while(1) {
        char key = get_key();
        if (key != 0) {
            if (key == '\n') {
                print_char('\n');
                
                // Don't truncate the command at the cursor position
                // command_buffer[buffer_pos] = '\0'; <- This line is the problem
                
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
            else if (key == KEY_LEFT) {
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
            }
            else if (key == KEY_END) {
                // Go to end of current text
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
