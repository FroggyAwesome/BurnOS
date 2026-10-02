ENTRY(_start)

PHDRS
{
	text PT_LOAD FLAGS(5);
	data PT_LOAD FLAGS(6);
}

SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	} :text

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	} :text

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	} :data

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	} :data
}
