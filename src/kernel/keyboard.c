#include "keyboard.h"

// Define keyboard I/O ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64


#define KEY_DELETE  0xF6

/* This table maps scan codes to ASCII characters */
static unsigned char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Read a byte from an I/O port */
unsigned char inb(unsigned short port) {
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

/* Get a key from the keyboard */
char get_key() {
    if (inb(KEYBOARD_STATUS_PORT) & 0x01) {
        unsigned char scan_code = inb(KEYBOARD_DATA_PORT);
        
        // Handle extended scan codes (arrow keys, etc.)
        if (scan_code == 0xE0) {
            // Wait for the next byte
            while (!(inb(KEYBOARD_STATUS_PORT) & 0x01));
            scan_code = inb(KEYBOARD_DATA_PORT);
            
            // Map extended scan codes to our defined values
            switch (scan_code) {
                case 0x53: return KEY_DELETE;// Delete key
                default: return 0;
            }
        }
        
        // Regular key processing
        if (scan_code < 0x80) { // Key press
            return scancode_to_ascii[scan_code];
        }
    }
    return 0; // No key
}

// Wait until a key is pressed and return its ASCII value
char wait_for_key() {
    char c = 0;
    while (c == 0) {
        c = get_key();
    }
    return c;
}