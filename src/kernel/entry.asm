; entry.asm - Assembly entry point that calls our C kernel
[bits 32]
[global _start]
[extern kernel_main]  ; Make sure this matches your C function name

section .text
_start:
    ; We're already in protected mode, skip trying to use BIOS interrupts
    
    ; Show a debug character at position 3
    mov byte [0xB8004], 'E'
    mov byte [0xB8005], 0x07
    
    ; Set up kernel stack
    mov esp, kernel_stack_top
    
    ; Call the C kernel main function
    call kernel_main
    
    ; Kernel should never return, but if it does:
    cli                 ; Disable interrupts
    hlt                 ; Halt the CPU
    jmp $               ; Infinite loop
    
; Reserve space for the kernel stack
section .bss
align 16
kernel_stack_bottom:
    resb 16384  ; 16 KB for kernel stack
kernel_stack_top: