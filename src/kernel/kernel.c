/* kernel.c - Main kernel entry point */
#include "keyboard.h"

/* Video memory address */
#define VIDEO_MEMORY 0xB8000
/* Color: white on black */
#define COLOR 0x0F

/* Function prototypes - declare these before using them */
void update_cursor();
void outb(unsigned short port, unsigned char value);

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

void kernel_main() {
    // Clear the screen first
    clear_screen();
    
    // Display a welcome message
    print_string("Welcome to NOX OS!", 25, 10);
    print_string("My first operating system", 22, 12);
    
    // Set cursor to a position for keyboard input testing
    cursor_x = 0;
    cursor_y = 14;
    
    print("Keyboard test: Type any key (hex values shown)\n");
    
    // Main keyboard testing loop
    while(1) {
        char key = get_key();
        
        // If a key was pressed, display it
        if (key != 0) {
            // Show the character itself if printable
            if (key >= 32 && key <= 126) {
                print_char(key);
                print_char(' ');
            } 
            // Show special keys as hex values
            else {
                char hex[6];
                hex[0] = '0';
                hex[1] = 'x';
                
                // Convert key value to hex
                unsigned char nibble;
                nibble = (key >> 4) & 0xF;
                hex[2] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
                
                nibble = key & 0xF;
                hex[3] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
                
                hex[4] = ' ';
                hex[5] = '\0';
                
                print(hex);
            }
        }
    }
}
