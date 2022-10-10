// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

.set KVM_TCONF, 32
.set KVM_TEXIT, 0xff
.set RESULT_ADDRESS, 0xfffff000

/*
 * This section will be allocated at address 0x1000 and
 * jumped to from the reset stub provided by kvm_run.
 */
.code16
.section .init.protected_mode, "ax"
real_mode_entry:
	cli

	lgdt kvm_gdt32_desc

	mov $0x11, %eax
	mov %eax, %cr0

	jmp $3 * 8, $protected_mode_entry

.code32
protected_mode_entry:
	mov $2 * 8, %eax
	mov %eax, %ds
	mov %eax, %es
	jmp init_memlayout

.section .data.gdt32, "a", @progbits

.macro gdt32_entry type:req l=0 d=0 dpl=0 limit=0xfffff g=1 p=1
	.4byte \limit & 0xffff
	.2byte (\type << 8) | (\dpl << 13) | (\p << 15)
	.2byte (\limit >> 16) | (\l << 5) | (\d << 6) | (\g << 7)
.endm
.align 8
kvm_gdt32:
	.8byte 0
	gdt32_entry type=0x1a l=1 /* Code segment long mode */
	gdt32_entry type=0x12 /* Data segment, writable */
	gdt32_entry type=0x1a l=0 d=1 /* Code segment protected_mode, 32bits */

.Lgdt32_end:
kvm_gdt32_desc:
	.2byte .Lgdt32_end - kvm_gdt32 - 1
	.4byte kvm_gdt32

.section .data.strings, "aS", @progbits
source_filename:
	.ascii "bootstrap_x86_64.S\0"

long_mode_err:
	.ascii "Virtual CPU does not support 64bit mode\0"

.code32
.section .init.memlayout, "ax"
init_memlayout:
	/*
	 * Identity-map the first 2GB of virtual address space.
	 */
	lea kvm_pagetable, %edi

	/*
	 * Set the first entry of kvm_pagetable (level 1) and fill the rest
	 * of the page with zeroes.
	 */
	lea kvm_pgtable_l2, %esi
	movl %esi, %ebx
	orl $0x3, %ebx		/* Flags: present, writable */
	movl %ebx, (%edi)
	addl $4, %edi
	movl $1023, %ecx
	xor %eax, %eax
	rep stosl

	/*
	 * Set the first four entries of kvm_pgtable_l2 and fill the rest
	 * of the page with zeroes.
	 */
	mov %esi, %edi
	lea kvm_pgtable_l3, %esi
	movl %esi, %eax
	mov $4, %ecx

1:	movl %eax, %ebx
	orl $0x3, %ebx		/* Flags: present, writable */
	movl %ebx, (%edi)
	movl $0, 4(%edi)
	addl $8, %edi
	addl $4096, %eax
	dec %ecx
	jnz 1b

	movl $1016, %ecx
	xor %eax, %eax
	rep stosl

	/* Fill kvm_pgtable_l3 with pointers to kvm_pgtable_l4 */
	mov %esi, %edi
	lea kvm_pgtable_l4, %esi
	movl %esi, %eax
	mov $4 * 512, %ecx

1:	movl %eax, %ebx
	orl $0x3, %ebx		/* Flags: present, writable */
	movl %ebx, (%edi)
	movl $0, 4(%edi)
	addl $8, %edi
	addl $4096, %eax
	dec %ecx
	jnz 1b

	/* Fill kvm_pgtable_l4 with identity map of the first 2GB. */
	movl %esi, %edi
	movl $2 * 512 * 512, %ecx
	xor %eax, %eax

1:	movl %eax, %ebx
	orl $0x3, %ebx		/* Flags: present, writable */
	movl %ebx, (%edi)
	movl $0, 4(%edi)
	addl $8, %edi
	addl $4096, %eax
	dec %ecx
	jnz 1b

	/* Mark the upper 2GB as unmapped except for the last page. */
	movl $4 * 512 * 512 - 2, %ecx
	xor %eax, %eax
	rep stosl
	movl $0xfffff003, (%edi)
	movl $0, 4(%edi)

	/*
	 * Now that the identity-map pagestables have been populated,
	 * we're ready to install them at CR3 and switch to long mode.
	 */
	/* Enable CR4.PAE */
	movl %cr4, %eax
	btsl $5, %eax
	movl %eax, %cr4

	lea kvm_pagetable, %eax
	movl %eax, %cr3

	/* Check if the CPU supports long mode. */
	movl $0x80000000, %eax
	cpuid
	cmpl $0x80000000, %eax
	jg 1f
	movl $KVM_TCONF, %edi
	lea long_mode_err, %esi
	jmp init_error
1:
	movl $0x80000001, %eax
	cpuid
	bt $29, %edx
	jc 1f
	movl $KVM_TCONF, %edi
	lea long_mode_err, %esi
	jmp init_error
1:

	/* Activate EFER.LME to enable long mode. */
	movl $0xc0000080, %ecx
	rdmsr
	btsl $8, %eax
	wrmsr

	/* Enable CR0.PG and CR0.WP */
	movl %cr0, %eax
	btsl $31, %eax
	btsl $16, %eax
	movl %eax, %cr0

	/* Long jmp to load the long mode %cs. */
	jmp $1 * 8, $long_mode_entry

init_error:
	/* Write error info to test result structure and exit VM */
	/* Equivalent to tst_brk() but using only 32bit instructions */
	movl %edi, RESULT_ADDRESS
	movl $RESULT_ADDRESS+4, %edi
	movl $0, (%edi)
	lea source_filename, %eax
	movl %eax, 4(%edi)
	movl $0, 8(%edi)
	addl $12, %edi
	xor %edx, %edx

1:	movzbl (%esi,%edx,1), %eax
	movb %al, (%edi,%edx,1)
	inc %edx
	test %al, %al
	jne 1b
	hlt
	jmp kvm_exit

.code64
long_mode_entry:
	lgdt kvm_gdt_desc

	/*
	 * Reset data segment selectors to NULL selector and
	 * initialize stack.
	 */
	xor %eax, %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %ss
	lea kvm_stack_top, %rsp

	/*
	 * Strictly speaking a TSS should not be required
	 * and experiments confirm that. However, we
	 * might perhaps want to play games with the
	 * interrupt/exception stacks in the future, so
	 * install a minimal one now.
	 */
	lea kvm_tss, %rdx
	movq %rdx, %rdi
	movq $.Ltss_end - kvm_tss, %rsi
	call memzero

	movq %rsp, 4(%rdx)

	/*
	 * Create a 16 byte descriptor starting at the
	 * 3rd 8-byte GDT slot.xs
	 */
	movq %rdx, %rax
	shl $40, %rax
	shr $24, %rax
	movq %rdx, %rbx
	shr $24, %rbx
	shl $56, %rbx
	or %rbx, %rax
	movq $0x89, %rbx
	shl $40, %rbx
	or $.Ltss_end - kvm_tss - 1, %rbx
	or %rbx, %rax
	shr $32, %rdx

	lea kvm_gdt + 2*8, %rdi
	mov %rax, (%rdi)
	mov %rdx, 8(%rdi)

	mov $2 * 8, %ax
	ltr %ax


	/* Configure and enable interrupts */
	call kvm_init_interrupts
	lidt kvm_idt_desc
	sti

	/*
	 * Do just enough of initialization to get to a working
	 * -ffreestanding environment and call tst_main(void).
	 */
	lea __preinit_array_start, %rdi
1:
	lea __preinit_array_end, %rsi
	cmp %rdi, %rsi
	je 2f
	push %rdi
	call *(%rdi)
	pop %rdi
	add $8, %rdi
	jmp 1b
2:

	lea __init_array_start, %rdi
1:
	lea __init_array_end, %rsi
	cmp %rdi, %rsi
	je 2f
	push %rdi
	call *(%rdi)
	pop %rdi
	add $8, %rdi
	jmp 1b
2:
	call main
	jmp kvm_exit

.global kvm_read_cregs
kvm_read_cregs:
	mov %cr0, %rax
	mov %rax, (%rdi)
	mov %cr2, %rax
	mov %rax, 8(%rdi)
	mov %cr3, %rax
	mov %rax, 16(%rdi)
	mov %cr4, %rax
	mov %rax, 24(%rdi)
	retq

handle_interrupt:
	/* push CPU state */
	push %rbp
	mov %rsp, %rbp
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rdi
	push %rsi
	push %r8
	push %r9
	push %r10
	push %r11

	/* load handler arguments from the stack and call handler */
	movq %rbp, %rdi
	addq $24, %rdi
	movq 8(%rbp), %rsi
	movq 16(%rbp), %rdx
	cld
	call tst_handle_interrupt

	/* restore CPU state and return */
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rsi
	pop %rdi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
	pop %rbp
	add $16, %rsp
	iretq

.macro create_intr_handler vector:req padargs=0
.if \padargs
	pushq $0	/* push dummy error code */
.endif
	pushq $\vector
	jmp handle_interrupt
.endm

.global kvm_handle_zerodiv
kvm_handle_zerodiv:
	create_intr_handler 0, padargs=1

.global kvm_handle_debug
kvm_handle_debug:
	create_intr_handler 1, padargs=1

.global kvm_handle_nmi
kvm_handle_nmi:
	create_intr_handler 2, padargs=1

.global kvm_handle_breakpoint
kvm_handle_breakpoint:
	create_intr_handler 3, padargs=1

.global kvm_handle_overflow
kvm_handle_overflow:
	create_intr_handler 4, padargs=1

.global kvm_handle_bound_range_exc
kvm_handle_bound_range_exc:
	create_intr_handler 5, padargs=1

.global kvm_handle_bad_opcode
kvm_handle_bad_opcode:
	create_intr_handler 6, padargs=1

.global kvm_handle_device_error
kvm_handle_device_error:
	create_intr_handler 7, padargs=1

.global kvm_handle_double_fault
kvm_handle_double_fault:
	create_intr_handler 8

.global kvm_handle_invalid_tss
kvm_handle_invalid_tss:
	create_intr_handler 10

.global kvm_handle_segfault
kvm_handle_segfault:
	create_intr_handler 11

.global kvm_handle_stack_fault
kvm_handle_stack_fault:
	create_intr_handler 12

.global kvm_handle_gpf
kvm_handle_gpf:
	create_intr_handler 13

.global kvm_handle_page_fault
kvm_handle_page_fault:
	create_intr_handler 14

.global kvm_handle_fpu_error
kvm_handle_fpu_error:
	create_intr_handler 16, padargs=1

.global kvm_handle_alignment_error
kvm_handle_alignment_error:
	create_intr_handler 17

.global kvm_handle_machine_check
kvm_handle_machine_check:
	create_intr_handler 18, padargs=1

.global kvm_handle_simd_error
kvm_handle_simd_error:
	create_intr_handler 19, padargs=1

.global kvm_handle_virt_error
kvm_handle_virt_error:
	create_intr_handler 20, padargs=1

.global kvm_handle_cpe
kvm_handle_cpe:
	create_intr_handler 21

.global kvm_handle_hv_injection
kvm_handle_hv_injection:
	create_intr_handler 28, padargs=1

.global kvm_handle_vmm_comm
kvm_handle_vmm_comm:
	create_intr_handler 29

.global kvm_handle_security_error
kvm_handle_security_error:
	create_intr_handler 30

.global kvm_handle_bad_exception
kvm_handle_bad_exception:
	create_intr_handler -1, padargs=1


.global kvm_exit
kvm_exit:
	movq $RESULT_ADDRESS, %rdi
	movl $KVM_TEXIT, (%rdi)
	hlt
	jmp kvm_exit

.global kvm_yield
kvm_yield:
	hlt
	ret


.section .bss.pgtables, "aw", @nobits
.global kvm_pagetable
kvm_pagetable:
	.skip 4096

kvm_pgtable_l2:
	.skip 4096

kvm_pgtable_l3:
	.skip 4 * 4096

kvm_pgtable_l4:
	.skip 4 * 512 * 4096

.section .data
.align 8
.global kvm_gdt
kvm_gdt:
	.8byte 0
	gdt32_entry type=0x1a l=1 limit=0 g=0 /* Code segment long mode */
	.skip 16 /* TSS segment descriptor */

.Lgdt_end:
.global kvm_gdt_desc
kvm_gdt_desc:
	.2byte .Lgdt_end - kvm_gdt - 1
	.8byte kvm_gdt


.section .bss.stack, "aw", @nobits
.global kvm_stack_bottom
kvm_stack_bottom:
	.skip 2 * 4096
.global kvm_stack_top
kvm_stack_top:

.section .bss.tss
.global kvm_tss
kvm_tss:
	.skip 0x6C
.Ltss_end:

.section .bss
.align 8
.global kvm_idt
kvm_idt:
	.skip 16 * 256
.Lidt_end:

.section .data
.align 8
.global kvm_idt_desc
kvm_idt_desc:
	.2byte .Lidt_end - kvm_idt - 1
	.8byte kvm_idt
