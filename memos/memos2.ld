OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
phys = 0x00100000;
SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}
  
	/* Read-write data (uninitialized) and stack */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.data)
	}
}
