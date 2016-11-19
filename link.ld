ENTRY (start)
SECTIONS
{
    . = 0x00100000;

    .text :
    {
        *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .bss ALIGN (0x1000):
    {
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }
	. = ALIGN (0x1000);
	__bss_end = .;
	. = ALIGN (0x1000);
    memstackTrueBottom = .;
    PROVIDE (memstackTrueBottom = .);
}