#ifndef KEYBOARD_H
#define KEYBOARD_H

// Read a byte from an I/O port
unsigned char inb(unsigned short port);

// Check if a key has been pressed and return its ASCII value
char get_key();

#endif