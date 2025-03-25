#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

/* Special key definitions */
#define KEY_DELETE  0x53

/* Function prototypes */
char get_key();
unsigned char inb(unsigned short port);

#endif /* KEYBOARD_H */