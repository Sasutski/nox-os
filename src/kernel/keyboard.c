#include "keyboard.h"

// Define keyboard I/O ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// A US keyboard layout mapping from scan codes to ASCII characters
static unsigned char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Read a byte from an I/O port
unsigned char inb(unsigned short port) {
    unsigned char data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "d"(port));
    return data;
}

// Check if a key is pressed and return its ASCII value
char get_key() {
    // Check if the keyboard has data available
    if (inb(KEYBOARD_STATUS_PORT) & 0x01) {
        // Read the scan code from the keyboard data port
        unsigned char scan_code = inb(KEYBOARD_DATA_PORT);
        
        // Check if it's a key press (not a key release)
        if (scan_code < 0x80) {
            // Convert scan code to ASCII and return it
            return scancode_to_ascii[scan_code];
        }
    }
    
    // No key press detected
    return 0;
}

// Wait until a key is pressed and return its ASCII value
char wait_for_key() {
    char c = 0;
    while (c == 0) {
        c = get_key();
    }
    return c;
}