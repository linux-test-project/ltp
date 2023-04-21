// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * x86-specific KVM helper functions
 */

#include "kvm_x86.h"

struct kvm_interrupt_frame {
	uintptr_t eip, cs, eflags, esp, ss;
};

const char *tst_interrupt_names[INTERRUPT_COUNT] = {
	"Division by zero",
	"Debug interrupt",
	"Non-maskable interrupt",
	"Breakpoint",
	"Arithmetic overflow",
	"Bound range exception",
	"Illegal instruction error",
	"Device not available error",
	"Double fault",
	NULL,
	"Invalid TSS error",
	"Segment not present error",
	"Stack segment fault",
	"General protection fault",
	"Page fault",
	NULL,
	"Floating point exception",
	"Alignment error",
	"Machine check exception",
	"SIMD floating point exception",
	"Virtualization exception",
	"Control protection exception",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Hypervisor injection exception",
	"VMM communication exception",
	"Security exception",
	NULL
};

static uintptr_t intr_handlers[] = {
	(uintptr_t)kvm_handle_zerodiv,
	(uintptr_t)kvm_handle_debug,
	(uintptr_t)kvm_handle_nmi,
	(uintptr_t)kvm_handle_breakpoint,
	(uintptr_t)kvm_handle_overflow,
	(uintptr_t)kvm_handle_bound_range_exc,
	(uintptr_t)kvm_handle_bad_opcode,
	(uintptr_t)kvm_handle_device_error,
	(uintptr_t)kvm_handle_double_fault,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_invalid_tss,
	(uintptr_t)kvm_handle_segfault,
	(uintptr_t)kvm_handle_stack_fault,
	(uintptr_t)kvm_handle_gpf,
	(uintptr_t)kvm_handle_page_fault,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_fpu_error,
	(uintptr_t)kvm_handle_alignment_error,
	(uintptr_t)kvm_handle_machine_check,
	(uintptr_t)kvm_handle_simd_error,
	(uintptr_t)kvm_handle_virt_error,
	(uintptr_t)kvm_handle_cpe,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_bad_exception,
	(uintptr_t)kvm_handle_hv_injection,
	(uintptr_t)kvm_handle_vmm_comm,
	(uintptr_t)kvm_handle_security_error,
	(uintptr_t)kvm_handle_bad_exception,
	0
};

static void kvm_set_intr_handler(unsigned int id, uintptr_t func)
{
	memset(kvm_idt + id, 0, sizeof(kvm_idt[0]));
	kvm_idt[id].offset_lo = func & 0xffff;
	kvm_idt[id].offset_hi = func >> 16;
	kvm_idt[id].selector = 8;
	kvm_idt[id].flags = 0x8f;	/* type = 0xf, P = 1 */
}

void kvm_init_interrupts(void)
{
	int i;

	for (i = 0; intr_handlers[i]; i++)
		kvm_set_intr_handler(i, intr_handlers[i]);

	for (; i < X86_INTR_COUNT; i++)
		kvm_set_intr_handler(i, (uintptr_t)kvm_handle_bad_exception);
}

uintptr_t kvm_get_page_address_pae(const struct page_table_entry_pae *entry)
{
	if (!entry->present)
		return 0;

	return entry->address << 12;
}

#ifdef __x86_64__
static void kvm_set_segment_descriptor64(struct segment_descriptor64 *dst,
	uint64_t baseaddr, uint32_t limit, unsigned int flags)
{

	dst->baseaddr_lo = baseaddr & 0xffffff;
	dst->baseaddr_hi = baseaddr >> 24;
	dst->limit_lo = limit & 0xffff;
	dst->limit_hi = limit >> 16;
	dst->flags_lo = flags & 0xff;
	dst->flags_hi = (flags >> 8) & 0xf;
	dst->reserved = 0;
}
#endif

void kvm_set_segment_descriptor(struct segment_descriptor *dst,
	uint64_t baseaddr, uint32_t limit, unsigned int flags)
{
	if (limit >> 20)
		tst_brk(TBROK, "Segment limit out of range");

#ifdef __x86_64__
	/* System descriptors have double size in 64bit mode */
	if (!(flags & SEGFLAG_NSYSTEM)) {
		kvm_set_segment_descriptor64((struct segment_descriptor64 *)dst,
			baseaddr, limit, flags);
		return;
	}
#endif

	if (baseaddr >> 32)
		tst_brk(TBROK, "Segment base address out of range");

	dst->baseaddr_lo = baseaddr & 0xffffff;
	dst->baseaddr_hi = baseaddr >> 24;
	dst->limit_lo = limit & 0xffff;
	dst->limit_hi = limit >> 16;
	dst->flags_lo = flags & 0xff;
	dst->flags_hi = (flags >> 8) & 0xf;
}

void kvm_parse_segment_descriptor(struct segment_descriptor *src,
	uint64_t *baseaddr, uint32_t *limit, unsigned int *flags)
{
	if (baseaddr) {
		*baseaddr = (((uint64_t)src->baseaddr_hi) << 24) |
			src->baseaddr_lo;
	}

	if (limit)
		*limit = (((uint32_t)src->limit_hi) << 16) | src->limit_lo;

	if (flags)
		*flags = (((uint32_t)src->flags_hi) << 8) | src->flags_lo;
}

int kvm_find_free_descriptor(const struct segment_descriptor *table,
	size_t size)
{
	const struct segment_descriptor *ptr;
	size_t i;

	for (i = 0, ptr = table; i < size; i++, ptr++) {
		if (!(ptr->flags_lo & SEGFLAG_PRESENT))
			return i;

#ifdef __x86_64__
		/* System descriptors have double size in 64bit mode */
		if (!(ptr->flags_lo & SEGFLAG_NSYSTEM)) {
			ptr++;
			i++;
		}
#endif
	}

	return -1;
}

unsigned int kvm_create_stack_descriptor(struct segment_descriptor *table,
	size_t tabsize, void *stack_base)
{
	int ret = kvm_find_free_descriptor(table, tabsize);

	if (ret < 0)
		tst_brk(TBROK, "Descriptor table is full");

	kvm_set_segment_descriptor(table + ret, 0,
		(((uintptr_t)stack_base) - 1) >> 12, SEGTYPE_STACK |
		SEGFLAG_PRESENT | SEGFLAG_32BIT | SEGFLAG_PAGE_LIMIT);
	return ret;
}

void kvm_get_cpuid(unsigned int eax, unsigned int ecx, struct kvm_cpuid *buf)
{
	asm (
		"cpuid\n"
		: "=a" (buf->eax), "=b" (buf->ebx), "=c" (buf->ecx),
			"=d" (buf->edx)
		: "0" (eax), "2" (ecx)
	);
}

uint64_t kvm_rdmsr(unsigned int msr)
{
	unsigned int ret_lo, ret_hi;

	asm (
		"rdmsr\n"
		: "=a" (ret_lo), "=d" (ret_hi)
		: "c" (msr)
	);

	return (((uint64_t)ret_hi) << 32) | ret_lo;
}

void kvm_wrmsr(unsigned int msr, uint64_t value)
{
	uint32_t val_lo = value & 0xffffffff, val_hi = value >> 32;

	asm (
		"wrmsr\n"
		:
		: "a" (val_lo), "d" (val_hi), "c" (msr)
	);
}

uintptr_t kvm_get_interrupt_ip(const struct kvm_interrupt_frame *ifrm)
{
	return ifrm->eip;
}
