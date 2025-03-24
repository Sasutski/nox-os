; boot.asm - A simple bootloader
[bits 16]
[org 0x7c00]

%define COM1_BASE 0x3F8

; Set up segments
cli
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00
sti

; Initialize COM1 (0x3F8) for 9600 baud, 8-N-1
mov dx, COM1_BASE
mov al, 0x80            ; Enable DLAB
add dx, 3
out dx, al              ; out 0x3FB, al

mov dx, COM1_BASE       ; Divisor low byte
mov al, 0x0C
out dx, al              ; out 0x3F8, al

inc dx                   ; Divisor high byte (0x3F9)
mov al, 0x00
out dx, al

mov dx, COM1_BASE
mov al, 0x03            ; 8 data bits, etc.
add dx, 3
out dx, al              ; out 0x3FB, al

mov dx, COM1_BASE
mov al, 0xC7            ; Enable FIFO
add dx, 2
out dx, al              ; out 0x3FA, al

mov dx, COM1_BASE
mov al, 0x0B            ; IRQs enabled, RTS/DSR set
add dx, 4
out dx, al              ; out 0x3FC, al

; Write a test character
mov al, 'H'
out dx, al

; Display a message
mov si, boot_message
call print_string

; Print a multi-line welcome message
mov si, welcome_msg
call print_string

; Load the kernel from disk
mov ah, 0x02    ; BIOS read sector function
mov al, 32      ; Increase sectors read
mov ch, 0       ; Cylinder number
mov cl, 2       ; Sector number (sectors start from 1, bootloader is at 1)
mov dh, 0       ; Head number
mov dl, 0       ; Drive number (0 = floppy disk)
mov bx, 0x1000  ; Memory location to load the kernel
int 0x13        ; Call BIOS interrupt
jc disk_error   ; Jump if error (carry flag set)

; Switch to protected mode
cli                    ; Disable interrupts
lgdt [gdt_descriptor]  ; Load GDT

; Set protected mode bit
mov eax, cr0
or eax, 0x1
mov cr0, eax

; Far jump to 32-bit code
jmp CODE_SEG:protected_mode_entry

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp hang

; Infinite loop for when we're done
hang:
    jmp hang

; Print string routine
print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string
done:
    ret

; Global Descriptor Table
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0
    
    ; Code segment descriptor
    dw 0xffff    ; Limit (bits 0-15)
    dw 0x0000    ; Base (bits 0-15)
    db 0x00      ; Base (bits 16-23)
    db 10011010b ; Access byte
    db 11001111b ; Flags and Limit (bits 16-19)
    db 0x0       ; Base (bits 24-31)
    
    ; Data segment descriptor
    dw 0xffff    ; Limit 
    dw 0x0000    ; Base (bits 0-15)
    db 0x00      ; Base (bits 16-23)
    db 10010010b ; Access byte
    db 11001111b ; Flags and Limit
    db 0x0       ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT address

; Constants
CODE_SEG equ 0x08
DATA_SEG equ 0x10

; Messages
boot_message db 'NOX OS Booting...', 0
disk_error_msg db 'Error loading kernel!', 0
welcome_msg db 13,10, "Welcome to NOX OS!", 13,10
db "This is line 2.", 13,10
db "Enjoy your stay.", 0

[bits 32]
protected_mode_entry:
    ; Set up segment registers for protected mode
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up a stack
    mov esp, 0x90000
    
    ; Far jump to the kernel using a code segment selector
    jmp CODE_SEG:0x1000

; Padding and boot signature
times 510-($-$$) db 0
dw 0xAA55