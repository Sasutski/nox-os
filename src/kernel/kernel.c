/* kernel.c - Main kernel entry point */
#include "keyboard.h"

/* Video memory address */
#define VIDEO_MEMORY 0xB8000
/* Color: white on black */
#define COLOR 0x0F

/* Function prototypes - declare these before using them */
void update_cursor();
void outb(unsigned short port, unsigned char value);
void move_cursor_left();
void move_cursor_right();
void move_cursor_to_start();
void move_cursor_to_end();
void handle_special_key(char key, char* command_buffer, int* buffer_pos);
int strcmp(const char* str1, const char* str2);
void execute_command(char* command);

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
    
    // Update the hardware cursor position
    update_cursor();
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

// Implement cursor movement functions
void move_cursor_left() {
    if (cursor_x > 0) {
        cursor_x--;
        update_cursor();
    } else if (cursor_y > 15) { // Command area starts at line 15
        cursor_y--;
        cursor_x = 79;
        update_cursor();
    }
}

void move_cursor_right() {
    // This needs to be aware of the current command length
    if (cursor_x < 79) {
        cursor_x++;
        update_cursor();
    } else {
        cursor_x = 0;
        cursor_y++;
        update_cursor();
    }
}

void move_cursor_to_start() {
    // Move to start of current command line
    cursor_x = 0;
    update_cursor();
}

void move_cursor_to_end(int command_length) {
    // Move to end of current command
    cursor_x = command_length % 80;
    cursor_y = 15 + (command_length / 80);
    update_cursor();
}

// Special key handler for command editing
void handle_special_key(char key, char* command_buffer, int* buffer_pos) {
    switch(key) {
        case KEY_LEFT:
            if (*buffer_pos > 0) {
                (*buffer_pos)--;
                move_cursor_left();
            }
            break;
            
        case KEY_RIGHT:
            if (command_buffer[*buffer_pos] != '\0') {
                (*buffer_pos)++;
                move_cursor_right();
            }
            break;
            
        case KEY_HOME:
            *buffer_pos = 0;
            move_cursor_to_start();
            break;
            
        case KEY_END:
            while (command_buffer[*buffer_pos] != '\0')
                (*buffer_pos)++;
            move_cursor_to_end(*buffer_pos);
            break;
    }
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
                // Handle backspace
                if (buffer_pos > 0) {
                    buffer_pos--;
                    move_cursor_left();
                    
                    // Shift characters to the left
                    int i = buffer_pos;
                    while(command_buffer[i]) {
                        command_buffer[i] = command_buffer[i+1];
                        putchar(command_buffer[i] ? command_buffer[i] : ' ', cursor_x + (i - buffer_pos), cursor_y);
                        i++;
                    }
                }
            }
            else if (key >= 32 && key <= 126) {
                // Insert character at current position
                int i = buffer_pos;
                while(command_buffer[i]) i++;
                while(i > buffer_pos) {
                    command_buffer[i] = command_buffer[i-1];
                    i--;
                }
                command_buffer[buffer_pos] = key;
                
                // Redraw the line from cursor position
                i = buffer_pos;
                while(command_buffer[i]) {
                    putchar(command_buffer[i], cursor_x + (i - buffer_pos), cursor_y);
                    i++;
                }
                
                buffer_pos++;
                move_cursor_right();
            }
            // Handle special keys (arrows, etc.)
            else if (key == KEY_LEFT || key == KEY_RIGHT || 
                     key == KEY_HOME || key == KEY_END) {
                handle_special_key(key, command_buffer, &buffer_pos);
            }
        }
    }
}
