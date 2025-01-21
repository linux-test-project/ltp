// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * x86-specific KVM helper functions
 */

#include "kvm_x86_svm.h"
#include "kvm_x86_vmx.h"

#define VMX_VMINST_ERR_COUNT 29

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

static const char *vmx_error_description[VMX_VMINST_ERR_COUNT] = {
	"Success",
	"VMCALL executed in VMX root",
	"VMCLEAR on invalid pointer",
	"VMCLEAR on VMXON pointer",
	"VMLAUNCH with non-clear VMCS",
	"VMRESUME with non-launched VMCS",
	"VMRESUME after VMXOFF",
	"VM entry with invalid VMCS control fields",
	"VM entry with invalid VMCS host state",
	"VMPTRLD with invalid pointer",
	"VMPTRLD with VMXON pointer",
	"VMPTRLD with incorrect VMCS version field",
	"Invalid VMCS field code",
	"VMWRITE to read-only VMCS field",
	"Unknown error",
	"VMXON called twice",
	"VM entry with invalid executive VMCS pointer",
	"VM entry with non-launched executive VMCS",
	"VM entry with executive VMCS pointer != VMXON pointer",
	"VMCALL with non-clear VMCS",
	"VMCALL with invalid VMCS exit control fields",
	"Unknown error",
	"VMCALL with incorrect MSEG revision ID",
	"VMXOFF under dual-monitor SMIs and SMM",
	"VMCALL with invalid SMM-monitor features",
	"VM entry with invalid executive VMCS execution control fields",
	"VM entry with events blocked by MOV SS",
	"Unknown error",
	"Invalid operand to INVEPT/INVVPID"
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

int kvm_is_vmx_supported(void)
{
	struct kvm_cpuid buf;

	kvm_get_cpuid(CPUID_GET_MODEL_INFO, 0, &buf);
	return buf.ecx & CPUID_MODEL_VMX;
}

void kvm_vmx_vmclear(struct kvm_vmcs *buf)
{
	uint64_t tmp = (uintptr_t)buf;

	asm goto(
		"vmclear (%0)\n"
		"jna %l[error]\n"
		:
		: "r" (&tmp)
		: "cc", "memory"
		: error
	);

	return;

error:
	tst_brk(TBROK, "VMCLEAR(%p) failed", buf);
}

void kvm_vmx_vmptrld(struct kvm_vmcs *buf)
{
	uint64_t tmp = (uintptr_t)buf;

	asm goto(
		"vmptrld (%0)\n"
		"jna %l[error]\n"
		:
		: "r" (&tmp)
		: "cc"
		: error
	);

	return;

error:
	tst_brk(TBROK, "VMPTRLD(%p) failed", buf);
}

uint64_t kvm_vmx_vmptrst(void)
{
	uint64_t ret;

	asm (
		"vmptrst (%0)\n"
		:
		: "r" (&ret)
		: "cc", "memory"
	);

	return ret;
}

uint64_t kvm_vmx_vmread(unsigned long var_id)
{
	uint64_t ret = 0;
	unsigned long tmp;

#ifndef __x86_64__
	if ((var_id & VMX_VMCSFIELD_SIZE_MASK) == VMX_VMCSFIELD_64BIT) {
		asm goto(
			"vmread %1, (%0)\n"
			"jna %l[error]\n"
			:
			: "r" (&tmp), "r" (var_id + 1)
			: "cc", "memory"
			: error
		);

		ret = tmp;
		ret <<= 32;
	}
#endif /* __x86_64__ */

	asm goto(
		"vmread %1, (%0)\n"
		"jna %l[error]\n"
		:
		: "r" (&tmp), "r" (var_id)
		: "cc", "memory"
		: error
	);

	ret |= tmp;
	return ret;

error:
	tst_brk(TBROK, "VMREAD(%lx) failed", var_id);
}

void kvm_vmx_vmwrite(unsigned long var_id, uint64_t value)
{
	unsigned long tmp = value;

	asm goto(
		"vmwrite %0, %1\n"
		"jna %l[error]\n"
		:
		: "r" (tmp), "r" (var_id)
		: "cc"
		: error
	);

#ifndef __x86_64__
	if ((var_id & VMX_VMCSFIELD_SIZE_MASK) == VMX_VMCSFIELD_64BIT) {
		tmp = value >> 32;

		asm goto(
			"vmwrite %0, %1\n"
			"jna %l[error]\n"
			:
			: "r" (tmp), "r" (var_id + 1)
			: "cc"
			: error
		);

	}
#endif /* __x86_64__ */

	return;

error:
	tst_brk(TBROK, "VMWRITE(%lx, %llx) failed", var_id, value);
}

static void kvm_vmx_vmxon(struct kvm_vmcs *buf)
{
	uint64_t tmp = (uintptr_t)buf;

	asm goto(
		"vmxon (%0)\n"
		"jna %l[error]\n"
		:
		: "r" (&tmp)
		: "cc"
		: error
	);

	return;

error:
	tst_brk(TBROK, "VMXON(%p) failed", buf);
}

static void kvm_vmx_vmxoff(void)
{
	asm goto(
		"vmxoff\n"
		"jna %l[error]\n"
		:
		:
		: "cc"
		: error
	);

	return;

error:
	tst_brk(TBROK, "VMXOFF failed");
}

struct kvm_vmcs *kvm_alloc_vmcs(void)
{
	struct kvm_vmcs *ret;

	ret = tst_heap_alloc_aligned(sizeof(struct kvm_vmcs), PAGESIZE);
	memset(ret, 0, sizeof(struct kvm_vmcs));
	ret->version = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_BASIC);
	return ret;
}

void kvm_set_vmx_state(int enabled)
{
	static struct kvm_vmcs *vmm_buf;
	uint64_t value;
	struct kvm_cregs cregs;

	if (!kvm_is_vmx_supported())
		tst_brk(TCONF, "CPU does not support VMX");

	kvm_read_cregs(&cregs);
	kvm_set_cr0(cregs.cr0 | CR0_NE);
	kvm_set_cr4(cregs.cr4 | CR4_VMXE);
	value = kvm_rdmsr(MSR_IA32_FEATURE_CONTROL);
	value |= IA32FC_LOCK | IA32FC_VMXON_NORMAL;
	kvm_wrmsr(MSR_IA32_FEATURE_CONTROL, value);

	if (!vmm_buf)
		vmm_buf = kvm_alloc_vmcs();

	if (enabled)
		kvm_vmx_vmxon(vmm_buf);
	else
		kvm_vmx_vmxoff();
}

void kvm_vmcs_copy_gdt_descriptor(unsigned int gdt_id,
	unsigned long vmcs_selector, unsigned long vmcs_flags,
	unsigned long vmcs_limit, unsigned long vmcs_baseaddr)
{
	uint64_t baseaddr;
	uint32_t limit;
	unsigned int flags;

	if (gdt_id >= KVM_GDT_SIZE)
		tst_brk(TBROK, "GDT descriptor ID out of range");

	kvm_parse_segment_descriptor(kvm_gdt + gdt_id, &baseaddr, &limit,
		&flags);

	if (!(flags & SEGFLAG_PRESENT)) {
		gdt_id = 0;
		baseaddr = 0;
		flags = 0x10000;
		limit = 0;
	} else if (flags & SEGFLAG_PAGE_LIMIT) {
		limit = (limit << 12) | 0xfff;
	}

	if (!(flags & 0x10000)) {
		// insert the reserved limit bits and force accessed bit to 1
		flags = ((flags & 0xf00) << 4) | (flags & 0xff) | 0x1;
	}

	kvm_vmx_vmwrite(vmcs_selector, gdt_id << 3);
	kvm_vmx_vmwrite(vmcs_flags, flags);
	kvm_vmx_vmwrite(vmcs_limit, limit);
	kvm_vmx_vmwrite(vmcs_baseaddr, baseaddr);
}

void kvm_init_vmx_vcpu(struct kvm_vmx_vcpu *cpu, uint16_t ss, void *rsp,
	int (*guest_main)(void))
{
	uint64_t old_vmcs, pinxctl, execctl, entryctl, exitctl;
	unsigned long crx;
	struct kvm_cregs cregs;
	struct kvm_sregs sregs;

	kvm_read_cregs(&cregs);
	kvm_read_sregs(&sregs);

	/* Clear cpu->vmcs first in case it's the current VMCS */
	kvm_vmx_vmclear(cpu->vmcs);
	memset(&cpu->regs, 0, sizeof(struct kvm_regs64));
	cpu->launched = 0;
	old_vmcs = kvm_vmx_vmptrst();
	kvm_vmx_vmptrld(cpu->vmcs);

	/* Configure VM execution control fields */
	if (kvm_rdmsr(MSR_IA32_VMX_BASIC) & IA32_VMXBASIC_USELESS_CTL_MASKS) {
		pinxctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_PINX_MASK2);
		execctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_EXECCTL_MASK2);
		exitctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_EXITCTL_MASK2);
		entryctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_ENTRYCTL_MASK2);
	} else {
		pinxctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_PINX_MASK);
		execctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_EXECCTL_MASK);
		exitctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_EXITCTL_MASK);
		entryctl = (uint32_t)kvm_rdmsr(MSR_IA32_VMX_ENTRYCTL_MASK);
	}

	execctl |= VMX_INTERCEPT_HLT;

	if (kvm_rdmsr(MSR_EFER) & EFER_LME) {
		entryctl |= VMX_ENTRYCTL_X64;
		exitctl |= VMX_EXITCTL_X64;
	}

	kvm_vmx_vmwrite(VMX_VMCS_VMPINX_CTL, pinxctl);
	kvm_vmx_vmwrite(VMX_VMCS_VMEXEC_CTL, execctl);
	kvm_vmx_vmwrite(VMX_VMCS_VMENTRY_CTL, entryctl);
	kvm_vmx_vmwrite(VMX_VMCS_VMEXIT_CTL, exitctl);
	kvm_vmx_vmwrite(VMX_VMCS_LINK_POINTER, VMX_INVALID_VMCS);
	kvm_vmcs_copy_gdt_descriptor(sregs.es >> 3, VMX_VMCS_GUEST_ES,
		VMX_VMCS_GUEST_ES_ACCESS, VMX_VMCS_GUEST_ES_LIMIT,
		VMX_VMCS_GUEST_ES_BASE);
	kvm_vmcs_copy_gdt_descriptor(sregs.cs >> 3, VMX_VMCS_GUEST_CS,
		VMX_VMCS_GUEST_CS_ACCESS, VMX_VMCS_GUEST_CS_LIMIT,
		VMX_VMCS_GUEST_CS_BASE);
	kvm_vmcs_copy_gdt_descriptor(ss, VMX_VMCS_GUEST_SS,
		VMX_VMCS_GUEST_SS_ACCESS, VMX_VMCS_GUEST_SS_LIMIT,
		VMX_VMCS_GUEST_SS_BASE);
	kvm_vmcs_copy_gdt_descriptor(sregs.ds >> 3, VMX_VMCS_GUEST_DS,
		VMX_VMCS_GUEST_DS_ACCESS, VMX_VMCS_GUEST_DS_LIMIT,
		VMX_VMCS_GUEST_DS_BASE);
	kvm_vmcs_copy_gdt_descriptor(sregs.fs >> 3, VMX_VMCS_GUEST_FS,
		VMX_VMCS_GUEST_FS_ACCESS, VMX_VMCS_GUEST_FS_LIMIT,
		VMX_VMCS_GUEST_FS_BASE);
	kvm_vmcs_copy_gdt_descriptor(sregs.gs >> 3, VMX_VMCS_GUEST_GS,
		VMX_VMCS_GUEST_GS_ACCESS, VMX_VMCS_GUEST_GS_LIMIT,
		VMX_VMCS_GUEST_GS_BASE);
	kvm_vmcs_copy_gdt_descriptor(sregs.tr >> 3, VMX_VMCS_GUEST_TR,
		VMX_VMCS_GUEST_TR_ACCESS, VMX_VMCS_GUEST_TR_LIMIT,
		VMX_VMCS_GUEST_TR_BASE);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_LDTR, 0);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_LDTR_ACCESS, 0x10000);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_LDTR_LIMIT, 0);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_LDTR_BASE, 0);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_GDTR_BASE, (uintptr_t)kvm_gdt);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_GDTR_LIMIT,
		(KVM_GDT_SIZE * sizeof(struct segment_descriptor)) - 1);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_IDTR_BASE, (uintptr_t)kvm_idt);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_IDTR_LIMIT,
		(X86_INTR_COUNT * sizeof(struct intr_descriptor)) - 1);

	crx = cregs.cr0 & kvm_rdmsr(MSR_IA32_VMX_CR0_FIXED1);
	crx |= kvm_rdmsr(MSR_IA32_VMX_CR0_FIXED0);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_CR0, crx);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_CR3, cregs.cr3);
	crx = cregs.cr4 & kvm_rdmsr(MSR_IA32_VMX_CR4_FIXED1);
	crx |= kvm_rdmsr(MSR_IA32_VMX_CR4_FIXED0);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_CR4, crx);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_RSP, (uintptr_t)rsp);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_RIP, (uintptr_t)kvm_svm_guest_entry);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_RFLAGS, 0x202); /* Interrupts enabled */
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_SYSENTER_ESP, 0);
	kvm_vmx_vmwrite(VMX_VMCS_GUEST_SYSENTER_EIP, 0);
	cpu->regs.rax = (uintptr_t)guest_main;

	/* Reactivate previous VMCS (if any) */
	if (old_vmcs != VMX_INVALID_VMCS)
		kvm_vmx_vmptrld((struct kvm_vmcs *)(uintptr_t)old_vmcs);
}

struct kvm_vmx_vcpu *kvm_create_vmx_vcpu(int (*guest_main)(void),
	int alloc_stack)
{
	uint16_t ss = 0;
	char *stack = NULL;
	struct kvm_vmcs *vmcs;
	struct kvm_vmx_vcpu *ret;

	vmcs = kvm_alloc_vmcs();

	if (alloc_stack) {
		stack = tst_heap_alloc_aligned(2 * PAGESIZE, PAGESIZE);
		ss = kvm_create_stack_descriptor(kvm_gdt, KVM_GDT_SIZE, stack);
		stack += 2 * PAGESIZE;
	}

	ret = tst_heap_alloc(sizeof(struct kvm_vmx_vcpu));
	memset(ret, 0, sizeof(struct kvm_vmx_vcpu));
	ret->vmcs = vmcs;
	kvm_init_vmx_vcpu(ret, ss, stack, guest_main);
	return ret;
}

void kvm_vmx_activate_vcpu(struct kvm_vmx_vcpu *cpu)
{
	struct kvm_cregs cregs;
	struct kvm_sregs sregs;
	uint64_t baseaddr;

	kvm_read_cregs(&cregs);
	kvm_read_sregs(&sregs);

	kvm_vmx_vmptrld(cpu->vmcs);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_ES, sregs.es);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_CS, sregs.cs);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_SS, sregs.ss);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_DS, sregs.ds);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_FS, sregs.fs);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_GS, sregs.gs);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_TR, sregs.tr);

	kvm_vmx_vmwrite(VMX_VMCS_HOST_CR0, cregs.cr0);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_CR3, cregs.cr3);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_CR4, cregs.cr4);
	kvm_parse_segment_descriptor(kvm_gdt + (sregs.fs >> 3), &baseaddr,
		NULL, NULL);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_FS_BASE, baseaddr);
	kvm_parse_segment_descriptor(kvm_gdt + (sregs.gs >> 3), &baseaddr,
		NULL, NULL);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_GS_BASE, baseaddr);
	kvm_parse_segment_descriptor(kvm_gdt + (sregs.tr >> 3), &baseaddr,
		NULL, NULL);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_TR_BASE, baseaddr);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_GDTR_BASE, (uintptr_t)kvm_gdt);
	kvm_vmx_vmwrite(VMX_VMCS_HOST_IDTR_BASE, (uintptr_t)kvm_idt);
}

void kvm_vmx_vmrun(struct kvm_vmx_vcpu *cpu)
{
	int ret, err;
	uint64_t reason;

	kvm_vmx_activate_vcpu(cpu);

	if (cpu->launched) {
		ret = kvm_vmx_vmresume(cpu);
	} else {
		ret = kvm_vmx_vmlaunch(cpu);
		cpu->launched = 1;
	}

	if (ret) {
		err = kvm_vmx_inst_errno();
		tst_brk(TBROK, "VMLAUNCH/VMRESUME failed: %s (%d)",
			kvm_vmx_inst_strerr(err), err);
	}

	reason = kvm_vmx_vmread(VMX_VMCS_EXIT_REASON);

	if (reason & VMX_EXIT_FAILED_ENTRY) {
		tst_brk(TBROK, "VM entry failed. Reason: %llu, qualif.: %llu",
			reason & 0xffff,
			kvm_vmx_vmread(VMX_VMCS_EXIT_QUALIFICATION));
	}
}

int kvm_vmx_inst_errno(void)
{
	unsigned long ret, var_id = VMX_VMCS_VMINST_ERROR;

	/* Do not use kvm_vmx_vmread() to avoid tst_brk() on failure */
	asm goto(
		"vmread %1, (%0)\n"
		"jna %l[error]\n"
		:
		: "r" (&ret), "r" (var_id)
		: "cc", "memory"
		: error
	);

	return ret;

error:
	return -1;
}

const char *kvm_vmx_inst_strerr(int vmx_errno)
{
	if (vmx_errno < 0)
		return "Cannot read VM errno - invalid current VMCS?";

	if (vmx_errno >= VMX_VMINST_ERR_COUNT)
		return "Unknown error";

	return vmx_error_description[vmx_errno];
}

const char *kvm_vmx_inst_err(void)
{
	return kvm_vmx_inst_strerr(kvm_vmx_inst_errno());
}
