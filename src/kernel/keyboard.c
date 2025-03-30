#include "keyboard.h"

// Define keyboard I/O ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Keyboard modifier states */
static int shift_pressed = 0;
static int capslock_enabled = 0;

/* Special scan codes */
#define SCAN_LEFT_SHIFT  0x2A
#define SCAN_RIGHT_SHIFT 0x36
#define SCAN_CAPS_LOCK   0x3A

/* This table maps scan codes to ASCII characters (unshifted) */
static unsigned char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* This table maps scan codes to ASCII characters (shifted) */
static unsigned char scancode_to_ascii_shifted[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Get a key from the keyboard */
char get_key() {
    if (inb(KEYBOARD_STATUS_PORT) & 0x01) {
        unsigned char scan_code = inb(KEYBOARD_DATA_PORT);
        
        // Handle special keys (shift, caps lock)
        if (scan_code == SCAN_LEFT_SHIFT || scan_code == SCAN_RIGHT_SHIFT) {
            shift_pressed = 1;
            return 0; // Don't return shift as a character
        }
        else if ((scan_code == (SCAN_LEFT_SHIFT + 0x80)) || (scan_code == (SCAN_RIGHT_SHIFT + 0x80))) {
            shift_pressed = 0;
            return 0; // Don't return shift release as a character
        }
        else if (scan_code == SCAN_CAPS_LOCK) {
            capslock_enabled = !capslock_enabled; // Toggle caps lock state
            return 0; // Don't return caps lock as a character
        }
        
        // Handle extended scan codes (arrow keys, etc.)
        if (scan_code == 0xE0) {
            // Wait for the next byte
            while (!(inb(KEYBOARD_STATUS_PORT) & 0x01));
            scan_code = inb(KEYBOARD_DATA_PORT);
            
            // Map extended scan codes to our defined values
            switch (scan_code) {
                case 0x48: return KEY_UP;
                case 0x50: return KEY_DOWN;
                case 0x4B: return KEY_LEFT;
                case 0x4D: return KEY_RIGHT;
                case 0x47: return KEY_HOME;
                case 0x4F: return KEY_END;
                case 0x53: return KEY_DELETE; // Delete key
                default: return 0;
            }
        }
        
        // Regular key processing
        if (scan_code < 0x80) { // Key press
            // Handle alphabetic keys (a-z, A-Z) based on caps lock and shift
            if ((scan_code >= 0x10 && scan_code <= 0x19) ||   // q-p
                (scan_code >= 0x1E && scan_code <= 0x26) ||   // a-l
                (scan_code >= 0x2C && scan_code <= 0x32)) {   // z-m
                
                // Apply shift XOR capslock for determining case
                // If only one of them is active, use uppercase
                if (shift_pressed ^ capslock_enabled) {
                    return scancode_to_ascii_shifted[scan_code];
                } else {
                    return scancode_to_ascii[scan_code];
                }
            }
            // For non-alphabetic characters, just check shift
            else if (shift_pressed) {
                return scancode_to_ascii_shifted[scan_code];
            } else {
                return scancode_to_ascii[scan_code];
            }
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