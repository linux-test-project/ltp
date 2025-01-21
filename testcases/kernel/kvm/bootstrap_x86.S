// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

.set KVM_TEXIT, 0xff
.set RESULT_ADDRESS, 0xfffff000
.set KVM_GDT_SIZE, 32

.set MSR_VM_HSAVE_PA, 0xc0010117

.set VMX_VMCS_HOST_RSP, 0x6c14
.set VMX_VMCS_HOST_RIP, 0x6c16

/*
 * This section will be allocated at address 0x1000 and
 * jumped to from the reset stub provided by kvm_run.
 */
.code16
.section .init.protected_mode, "ax"
real_mode_entry:
	cli

	lgdt kvm_gdt_desc

	mov $0x11, %eax
	mov %eax, %cr0

	jmp $1 * 8, $protected_mode_entry

.code32
protected_mode_entry:
	mov $2 * 8, %eax
	mov %eax, %ds
	mov %eax, %es
	jmp init_memlayout

.section .init.gdt32, "a", @progbits

.macro gdt32_entry type:req l=0 d=0 dpl=0 limit=0xfffff g=1 p=1
	.4byte \limit & 0xffff
	.2byte (\type << 8) | (\dpl << 13) | (\p << 15)
	.2byte (\limit >> 16) | (\l << 5) | (\d << 6) | (\g << 7)
.endm
.align 8
.global kvm_gdt
kvm_gdt:
	.8byte 0
	gdt32_entry type=0x1a l=0 d=1 /* Code segment protected_mode, 32bits */
	gdt32_entry type=0x12 /* Data segment, writable */
	.skip (KVM_GDT_SIZE-3)*8 /* Stack, TSS and other segment descriptors */

.Lgdt_end:
.global kvm_gdt_desc
kvm_gdt_desc:
	.2byte .Lgdt_end - kvm_gdt - 1
	.4byte kvm_gdt

.code32
.section .init.memlayout, "ax"
init_memlayout:
	/*
	 * Identity-map the first 2GB of virtual address space.
	 */
	lea kvm_pagetable, %edi
	lea kvm_pgtable_l2, %esi
	movl %esi, %eax
	mov $1024, %ecx

1:	movl %eax, %ebx
	orl $0x3, %ebx		/* Flags: present, writable */
	movl %ebx, (%edi)
	addl $4, %edi
	addl $4096, %eax
	dec %ecx
	jnz 1b

	/* Fill kvm_pgtable_l2 with identity map of the first 2GB. */
	movl %esi, %edi
	movl $512 * 1024, %ecx
	xor %eax, %eax

1:	movl %eax, %ebx
	orl $0x3, %ebx		/* Flags: present, writable */
	movl %ebx, (%edi)
	addl $4, %edi
	addl $4096, %eax
	dec %ecx
	jnz 1b

	/* Mark the upper 2GB as unmapped except for the last page. */
	movl $512 * 1024 - 1, %ecx
	xor %eax, %eax
	rep stosl
	movl $0xfffff003, (%edi)

	/*
	 * Install new pagetable to CR3 and enable memory paging by setting
	 * CR0.WP and CR0.PG
	 */
	lea kvm_pagetable, %eax
	movl %eax, %cr3
	movl %cr0, %eax
	btsl $31, %eax
	btsl $16, %eax
	movl %eax, %cr0

	/* Init TSS */
	lea kvm_tss, %edx
	movl %edx, %edi
	movl $.Ltss_end - kvm_tss, %ecx
	xor %eax, %eax
	rep stosb
	movl %edx, %edi
	lea kvm_stack_top, %edx
	movl %edx, 4(%edi)

	/* Create a stack descriptor in the 4th GDT slot */
	/* Base address: 0x0, Limit: kvm_stack_bottom */
	xor %eax, %eax
	movl $0xc09600, %ebx /* flags + access bits */
	movl $kvm_stack_bottom - 1, %edx
	shr $12, %edx
	movw %dx, %ax
	andl $0xf0000, %edx
	orl %edx, %ebx

	lea kvm_gdt + 3*8, %edi
	mov %eax, (%edi)
	mov %ebx, 4(%edi)
	mov $3 * 8, %eax
	mov %ax, %ss
	lea kvm_stack_top, %esp

	/* Create a TSS descriptor in the 5th GDT slot */
	lea kvm_tss, %edx
	movl %edx, %ebx
	andl $0xff000000, %ebx
	movl %edx, %eax
	shr $16, %eax
	movb %al, %bl
	orl $0x408900, %ebx /* flags + access bits */

	movl %edx, %eax
	movl $.Ltss_end - kvm_tss - 1, %edx
	shl $16, %eax
	movw %dx, %ax
	andl $0xf0000, %edx
	orl %edx, %ebx

	lea kvm_gdt + 4*8, %edi
	mov %eax, (%edi)
	mov %ebx, 4(%edi)
	mov $4 * 8, %ax
	ltr %ax

	/* Configure and enable interrupts */
	call kvm_init_interrupts
	lidt kvm_idt_desc
	sti

	/*
	 * Do just enough of initialization to get to a working
	 * -ffreestanding environment and call tst_main(void).
	 */
	lea __preinit_array_start, %edi
	lea __preinit_array_end, %esi
1:
	cmp %edi, %esi
	je 2f
	call *(%edi)
	add $4, %edi
	jmp 1b
2:

	lea __init_array_start, %edi
	lea __init_array_end, %esi
1:
	cmp %edi, %esi
	je 2f
	call *(%edi)
	add $4, %edi
	jmp 1b
2:
	call main
	jmp kvm_exit

.global kvm_read_cregs
kvm_read_cregs:
	push %edi
	mov 8(%esp), %edi
	mov %cr0, %eax
	mov %eax, (%edi)
	mov %cr2, %eax
	mov %eax, 4(%edi)
	mov %cr3, %eax
	mov %eax, 8(%edi)
	mov %cr4, %eax
	mov %eax, 12(%edi)
	pop %edi
	ret

.global kvm_read_sregs
kvm_read_sregs:
	push %edi
	mov 8(%esp), %edi
	mov %cs, %ax
	movw %ax, (%edi)
	mov %ds, %ax
	movw %ax, 2(%edi)
	mov %es, %ax
	movw %ax, 4(%edi)
	mov %fs, %ax
	movw %ax, 6(%edi)
	mov %gs, %ax
	movw %ax, 8(%edi)
	mov %ss, %ax
	movw %ax, 10(%edi)
	str %ax
	movw %ax, 12(%edi)
	pop %edi
	ret

handle_interrupt:
	/* save CPU state */
	push %ebp
	mov %esp, %ebp
	addl $12, %ebp
	pushal

	/* call handler */
	push -4(%ebp)
	push -8(%ebp)
	push %ebp
	cld
	call tst_handle_interrupt
	addl $12, %esp
	popal
	pop %ebp
	addl $8, %esp
	iret

.macro create_intr_handler vector:req padargs=0
.if \padargs
	pushl $0	/* push dummy error code */
.endif
	pushl $\vector
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
	movl $RESULT_ADDRESS, %edi
	movl $KVM_TEXIT, (%edi)
	hlt
	jmp kvm_exit

.global kvm_yield
kvm_yield:
	hlt
	ret

.global kvm_svm_guest_entry
kvm_svm_guest_entry:
	call *%eax
1:	hlt
	jmp 1b

/* vcpu structure address must be in %rdi */
.macro load_vcpu_regs
	movl 0x04(%edi), %eax
	movl 0x0c(%edi), %ebx
	movl 0x14(%edi), %ecx
	movl 0x1c(%edi), %edx
	/* save %edi last */
	movl 0x2c(%edi), %esi
	movl 0x34(%edi), %ebp
	/* skip %esp */
	movl 0x24(%edi), %edi
.endm

/* vcpu structure address must be on top of the stack */
.macro save_vcpu_regs
	push %edi
	movl 4(%esp), %edi
	movl %eax, 0x04(%edi)
	movl %ebx, 0x0c(%edi)
	movl %ecx, 0x14(%edi)
	movl %edx, 0x1c(%edi)
	pop %eax
	movl %eax, 0x24(%edi)
	movl %esi, 0x2c(%edi)
	movl %ebp, 0x34(%edi)
	/* skip %esp */
.endm

.global kvm_svm_vmrun
kvm_svm_vmrun:
	push %edi
	mov 8(%esp), %edi
	push %ebx
	push %esi
	push %ebp

	clgi

	/* Save full host state */
	movl $MSR_VM_HSAVE_PA, %ecx
	rdmsr
	vmsave
	push %eax

	push %edi
	load_vcpu_regs
	/* %eax = vcpu->vmcb; */
	movl (%esp), %eax
	movl (%eax), %eax

	vmload
	vmrun
	vmsave

	/* Clear guest register buffer */
	push %edi
	push %eax
	push %ecx
	movl 12(%esp), %edi
	addl $4, %edi
	xorl %eax, %eax
	mov $32, %ecx
	pushfl
	cld
	rep stosl
	popfl
	pop %ecx
	pop %eax
	pop %edi

	save_vcpu_regs
	pop %edi

	/* Copy %eax and %esp from VMCB */
	movl (%edi), %esi
	movl 0x5f8(%esi), %eax
	movl %eax, 0x04(%edi)
	movl 0x5d8(%esi), %eax
	movl %eax, 0x3c(%edi)

	pop %eax
	vmload
	stgi

	pop %ebp
	pop %esi
	pop %ebx
	pop %edi
	ret

.global kvm_vmx_vmlaunch
kvm_vmx_vmlaunch:
	push %edi
	mov 8(%esp), %edi
	push %ebx
	push %esi
	push %ebp
	push %edi

	mov $VMX_VMCS_HOST_RSP, %eax
	vmwrite %esp, %eax
	jna vmx_vmwrite_error
	mov $VMX_VMCS_HOST_RIP, %eax
	lea vmx_vm_exit, %ebx
	vmwrite %ebx, %eax
	jna vmx_vmwrite_error

	load_vcpu_regs
	vmlaunch
	jmp vmx_vm_exit

.global kvm_vmx_vmresume
kvm_vmx_vmresume:
	push %edi
	mov 8(%esp), %edi
	push %ebx
	push %esi
	push %ebp
	push %edi

	mov $VMX_VMCS_HOST_RSP, %eax
	vmwrite %esp, %eax
	jna vmx_vmwrite_error
	mov $VMX_VMCS_HOST_RIP, %eax
	lea vmx_vm_exit, %ebx
	vmwrite %ebx, %eax
	jna vmx_vmwrite_error

	load_vcpu_regs
	vmresume

vmx_vm_exit:
	jna vmx_vmentry_error
	save_vcpu_regs
	xorl %eax, %eax

vmx_vm_ret:
	pop %edi
	pop %ebp
	pop %esi
	pop %ebx
	pop %edi
	ret

vmx_vmwrite_error:
	movl $2, %eax
	jmp vmx_vm_ret

vmx_vmentry_error:
	movl $1, %eax
	jmp vmx_vm_ret

.section .bss.pgtables, "aw", @nobits
.global kvm_pagetable
kvm_pagetable:
	.skip 4096

kvm_pgtable_l2:
	.skip 1024 * 4096

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
	.skip 8 * 256
.Lidt_end:

.section .data
.align 8
.global kvm_idt_desc
kvm_idt_desc:
	.2byte .Lidt_end - kvm_idt - 1
	.4byte kvm_idt
