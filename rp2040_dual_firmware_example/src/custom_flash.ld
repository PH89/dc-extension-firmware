MEMORY
{
  FLASH(rx) : ORIGIN = 0x100000, LENGTH = 0xF0000
  RAM(rwx)  : ORIGIN = 0x20000000, LENGTH = 256K
}
INCLUDE "pico_standard_link/memmap_default.ld"