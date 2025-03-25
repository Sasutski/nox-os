#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

/* Special key codes */
#define KEY_UP      0x48
#define KEY_DOWN    0x50
#define KEY_LEFT    0x4B
#define KEY_RIGHT   0x4D
#define KEY_HOME    0x47
#define KEY_END     0x4F
#define KEY_DELETE  0x53

/* Function prototypes */
char get_key();
unsigned char inb(unsigned short port);

#endif /* KEYBOARD_H */