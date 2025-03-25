OUTPUT_FORMAT(binary)

ENTRY(_start)

SECTIONS {
    . = 0x1000;

    .text : {
        *(.text)
        *(.rodata)  /* Ensure rodata is inside text */
    }

    .data : {
        *(.data)
    }
    
    .bss : {
        *(.bss)
    }
}
