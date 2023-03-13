OUTPUT_FORMAT(elf64-x86-64)

PHDRS
{
	headers PT_PHDR PHDRS ;
	text PT_LOAD FILEHDR PHDRS ;
	data PT_LOAD ;
	bss PT_LOAD ;
}

SECTIONS
{
	/DISCARD/ :
	{
		  *(.note.gnu.* .comment)
	}

	. = 0x1000;
	.init.boot :
	{
		*(.init.protected_mode)
		*(.init.gdt32)
		*(.init.memlayout)
	} :text

	.text :
	{
		*(.rodata .rodata.*)
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
	}

	.init :
	{
		KEEP (*(SORT_NONE(.init)))
	}

	.data :
	{
		*(.data.strings)
		*(.data)
	} :data

	.preinit_array :
	{
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	}

	.init_array :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	}

	.bss.pgtables : ALIGN(4096)
	{
		*(.bss.pgtables)
	} :bss

	.bss : ALIGN(4096)
	{
		*(.bss.stack)
		*(.bss.tss)
		*(.bss)

		. = ALIGN(4096);
		kvm_heap_begin = .;
	}
}
