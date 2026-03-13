MEMORY
{
    FLASH(rx)   : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM(rwx)    : ORIGIN = 0x20000000, LENGTH = 640K
}

SECTIONS
{
    _estack = ORIGIN(RAM) + LENGTH(RAM);

    .isr_vector :
    {
        *(.isr_vector)
    } > FLASH

    .text :
    {
        *(.text)
    } > FLASH

    .data :
    {
        _sdata = .;
        *(.data)
        _edata = .;
    } > RAM AT> FLASH

    _sidata = LOADADDR(.data);

    /* BSS = Block Started by Symbol, stores all uninitialized global variables */
    .bss :
    {
        _sbss = .;
        *(.bss)
        _ebss = .;
    } > RAM
}
