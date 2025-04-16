#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

/* Special key definitions - use values above normal ASCII range */
#define KEY_DELETE  0x7F    /* Above normal ASCII */
#define KEY_UP      0x80    /* Above normal ASCII */
#define KEY_DOWN    0x81    /* Above normal ASCII */
#define KEY_LEFT    0x82    /* Above normal ASCII */
#define KEY_RIGHT   0x83    /* Above normal ASCII */
#define KEY_HOME    0x84    /* Above normal ASCII */
#define KEY_END     0x85    /* Above normal ASCII */

/* Function prototypes */
unsigned char get_key();
unsigned char inb(unsigned short port);

#endif /* KEYBOARD_H */