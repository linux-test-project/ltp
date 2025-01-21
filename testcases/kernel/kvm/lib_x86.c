// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * x86-specific KVM helper functions
 */

#include "kvm_x86_svm.h"

void kvm_svm_guest_entry(void);

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

	for (i = 1, ptr = table + 1; i < size; i++, ptr++) {
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

void kvm_set_cr0(unsigned long val)
{
	asm (
		"mov %0, %%cr0\n"
		:
		: "r" (val)
	);
}

void kvm_set_cr3(unsigned long val)
{
	asm (
		"mov %0, %%cr3\n"
		:
		: "r" (val)
	);
}

void kvm_set_cr4(unsigned long val)
{
	asm (
		"mov %0, %%cr4\n"
		:
		: "r" (val)
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

int kvm_is_svm_supported(void)
{
	struct kvm_cpuid buf;

	kvm_get_cpuid(CPUID_GET_INPUT_RANGE, 0, &buf);

	if (buf.eax < CPUID_GET_EXT_FEATURES)
		return 0;

	kvm_get_cpuid(CPUID_GET_EXT_FEATURES, 0, &buf);
	return buf.ecx & 0x4;
}

int kvm_get_svm_state(void)
{
	return kvm_rdmsr(MSR_EFER) & EFER_SVME;
}

void kvm_set_svm_state(int enabled)
{
	uint64_t value;

	if (!kvm_is_svm_supported())
		tst_brk(TCONF, "CPU does not support SVM");

	if (kvm_rdmsr(MSR_VM_CR) & VM_CR_SVMDIS)
		tst_brk(TCONF, "SVM is supported but disabled");

	value = kvm_rdmsr(MSR_EFER);

	if (enabled)
		value |= EFER_SVME;
	else
		value &= ~EFER_SVME;

	kvm_wrmsr(MSR_EFER, value);
}

struct kvm_vmcb *kvm_alloc_vmcb(void)
{
	struct kvm_vmcb *ret;

	ret = tst_heap_alloc_aligned(sizeof(struct kvm_vmcb), PAGESIZE);
	memset(ret, 0, sizeof(struct kvm_vmcb));
	return ret;
}

void kvm_init_svm(void)
{
	kvm_set_svm_state(1);
	kvm_wrmsr(MSR_VM_HSAVE_PA, (uintptr_t)kvm_alloc_vmcb());
}

void kvm_vmcb_copy_gdt_descriptor(struct kvm_vmcb_descriptor *dst,
	unsigned int gdt_id)
{
	uint64_t baseaddr;
	uint32_t limit;
	unsigned int flags;

	if (gdt_id >= KVM_GDT_SIZE)
		tst_brk(TBROK, "GDT descriptor ID out of range");

	kvm_parse_segment_descriptor(kvm_gdt + gdt_id, &baseaddr, &limit,
		&flags);

	if (!(flags & SEGFLAG_PRESENT)) {
		memset(dst, 0, sizeof(struct kvm_vmcb_descriptor));
		return;
	}

	if (flags & SEGFLAG_PAGE_LIMIT)
		limit = (limit << 12) | 0xfff;

	dst->selector = gdt_id << 3;
	dst->attrib = flags;
	dst->limit = limit;
	dst->base = baseaddr;
}

void kvm_vmcb_set_intercept(struct kvm_vmcb *vmcb, unsigned int id,
	unsigned int state)
{
	unsigned int addr = id / 8, bit = 1 << (id % 8);

	if (id >= SVM_INTERCEPT_MAX)
		tst_brk(TBROK, "Invalid SVM intercept ID");

	if (state)
		vmcb->intercepts[addr] |= bit;
	else
		vmcb->intercepts[addr] &= ~bit;
}

void kvm_init_guest_vmcb(struct kvm_vmcb *vmcb, uint32_t asid, uint16_t ss,
	void *rsp, int (*guest_main)(void))
{
	struct kvm_cregs cregs;
	struct kvm_sregs sregs;

	kvm_read_cregs(&cregs);
	kvm_read_sregs(&sregs);

	kvm_vmcb_set_intercept(vmcb, SVM_INTERCEPT_VMRUN, 1);
	kvm_vmcb_set_intercept(vmcb, SVM_INTERCEPT_HLT, 1);

	kvm_vmcb_copy_gdt_descriptor(&vmcb->es, sregs.es >> 3);
	kvm_vmcb_copy_gdt_descriptor(&vmcb->cs, sregs.cs >> 3);
	kvm_vmcb_copy_gdt_descriptor(&vmcb->ss, ss);
	kvm_vmcb_copy_gdt_descriptor(&vmcb->ds, sregs.ds >> 3);
	kvm_vmcb_copy_gdt_descriptor(&vmcb->fs, sregs.fs >> 3);
	kvm_vmcb_copy_gdt_descriptor(&vmcb->gs, sregs.gs >> 3);
	vmcb->gdtr.base = (uintptr_t)kvm_gdt;
	vmcb->gdtr.limit = (KVM_GDT_SIZE*sizeof(struct segment_descriptor)) - 1;
	vmcb->idtr.base = (uintptr_t)kvm_idt;
	vmcb->idtr.limit = (X86_INTR_COUNT*sizeof(struct intr_descriptor)) - 1;

	vmcb->guest_asid = asid;
	vmcb->efer = kvm_rdmsr(MSR_EFER);
	vmcb->cr0 = cregs.cr0;
	vmcb->cr3 = cregs.cr3;
	vmcb->cr4 = cregs.cr4;
	vmcb->rip = (uintptr_t)kvm_svm_guest_entry;
	vmcb->rax = (uintptr_t)guest_main;
	vmcb->rsp = (uintptr_t)rsp;
	vmcb->rflags = 0x200;	/* Interrupts enabled */
}

struct kvm_svm_vcpu *kvm_create_svm_vcpu(int (*guest_main)(void),
	int alloc_stack)
{
	uint16_t ss = 0;
	char *stack = NULL;
	struct kvm_vmcb *vmcb;
	struct kvm_svm_vcpu *ret;

	vmcb = kvm_alloc_vmcb();

	if (alloc_stack) {
		stack = tst_heap_alloc_aligned(2 * PAGESIZE, PAGESIZE);
		ss = kvm_create_stack_descriptor(kvm_gdt, KVM_GDT_SIZE, stack);
		stack += 2 * PAGESIZE;
	}

	kvm_init_guest_vmcb(vmcb, 1, ss, stack, guest_main);
	ret = tst_heap_alloc(sizeof(struct kvm_svm_vcpu));
	memset(ret, 0, sizeof(struct kvm_svm_vcpu));
	ret->vmcb = vmcb;
	return ret;
}

void kvm_svm_vmload(struct kvm_vmcb *buf)
{
	asm (
		"vmload %0\n"
		:
		: "a" (buf)
	);
}

void kvm_svm_vmsave(struct kvm_vmcb *buf)
{
	asm (
		"vmsave %0\n"
		:
		: "a" (buf)
	);
}
