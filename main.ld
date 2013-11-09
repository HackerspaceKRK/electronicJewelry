MEMORY 
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 16k
  SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 4k
}
SECTIONS
{
  .text :
  {
    _text = .;
    *(.isr_vector)
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH
  
  _sidata = .;

  .data : 
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > SRAM AT>FLASH
  
  .bss :
  {
    _bss = .;
    *(.bss)
    *(.bss*)
  } > SRAM AT>FLASH
    _ebss = .;
  
  end = .;
}
