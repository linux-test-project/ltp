/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * x86-specific KVM helper functions and structures
 */

#ifndef KVM_X86_H_
#define KVM_X86_H_

#include "kvm_test.h"

#define PAGESIZE 0x1000
#define KVM_GDT_SIZE 32

/* Interrupts */
#define X86_INTR_COUNT 256

#define INTR_ZERODIV 0
#define INTR_DEBUG 1
#define INTR_NMI 2
#define INTR_BREAKPOINT 3
#define INTR_OVERFLOW 4
#define INTR_BOUND_RANGE_EXC 5
#define INTR_BAD_OPCODE 6
#define INTR_DEVICE_ERROR 7
#define INTR_DOUBLE_FAULT 8
#define INTR_INVALID_TSS 10
#define INTR_SEGFAULT 11
#define INTR_STACK_FAULT 12
#define INTR_GPF 13
#define INTR_PAGE_FAULT 14
#define INTR_FPU_ERROR 16
#define INTR_ALIGNMENT_ERROR 17
#define INTR_MACHINE_CHECK 18
#define INTR_SIMD_ERROR 19
#define INTR_VIRT_ERROR 20
#define INTR_CPE 21
#define INTR_HV_INJECTION 28
#define INTR_VMM_COMM 29
#define INTR_SECURITY_ERROR 30


/* Segment descriptor flags */
#define SEGTYPE_LDT 0x02
#define SEGTYPE_TSS 0x09
#define SEGTYPE_TSS_BUSY 0x0b
#define SEGTYPE_CALL_GATE 0x0c
#define SEGTYPE_INTR_GATE 0x0e
#define SEGTYPE_TRAP_GATE 0x0f
#define SEGTYPE_RODATA 0x10
#define SEGTYPE_RWDATA 0x12
#define SEGTYPE_STACK 0x16
#define SEGTYPE_CODE 0x1a
#define SEGTYPE_MASK 0x1f

#define SEGFLAG_NSYSTEM 0x10
#define SEGFLAG_PRESENT 0x80
#define SEGFLAG_CODE64 0x200
#define SEGFLAG_32BIT 0x400
#define SEGFLAG_PAGE_LIMIT 0x800


/* CPUID constants */
#define CPUID_GET_MODEL_INFO 0x1
#define CPUID_GET_INPUT_RANGE 0x80000000
#define CPUID_GET_EXT_FEATURES 0x80000001
#define CPUID_GET_SVM_FEATURES 0x8000000a


/* Model-specific CPU register constants */
#define MSR_IA32_FEATURE_CONTROL 0x3a
#define MSR_SYSENTER_CS 0x174
#define MSR_SYSENTER_ESP 0x175
#define MSR_SYSENTER_EIP 0x176
#define MSR_EFER 0xc0000080
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_CSTAR 0xc0000083
#define MSR_SFMASK 0xc0000084
#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101
#define MSR_KERNEL_GS_BASE 0xc0000102
#define MSR_VM_CR 0xc0010114
#define MSR_VM_HSAVE_PA 0xc0010117

#define EFER_SCE (1 << 0)	/* SYSCALL/SYSRET instructions enabled */
#define EFER_LME (1 << 8)	/* CPU is running in 64bit mode */
#define EFER_LMA (1 << 10)	/* CPU uses 64bit memory paging (read-only) */
#define EFER_NXE (1 << 11)	/* Execute disable bit active */
#define EFER_SVME (1 << 12)	/* AMD SVM instructions enabled */

#define VM_CR_DPD (1 << 0)
#define VM_CR_R_INIT (1 << 1)
#define VM_CR_DIS_A20M (1 << 2)
#define VM_CR_LOCK (1 << 3)
#define VM_CR_SVMDIS (1 << 4)

/* Control register constants */
#define CR0_PE (1 << 0)
#define CR0_MP (1 << 1)
#define CR0_EM (1 << 2)
#define CR0_TS (1 << 3)
#define CR0_ET (1 << 4)
#define CR0_NE (1 << 5)
#define CR0_WP (1 << 16)
#define CR0_AM (1 << 18)
#define CR0_NW (1 << 29)
#define CR0_CD (1 << 30)
#define CR0_PG (1 << 31)

#define CR4_VME (1 << 0)
#define CR4_PVI (1 << 1)
#define CR4_TSD (1 << 2)
#define CR4_DE (1 << 3)
#define CR4_PSE (1 << 4)
#define CR4_PAE (1 << 5)
#define CR4_MCE (1 << 6)
#define CR4_PGE (1 << 7)
#define CR4_PCE (1 << 8)
#define CR4_OSFXSR (1 << 9)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_UMIP (1 << 11)
#define CR4_LA57 (1 << 12)
#define CR4_VMXE (1 << 13)
#define CR4_SMXE (1 << 14)
#define CR4_FSGSBASE (1 << 16)
#define CR4_PCIDE (1 << 17)
#define CR4_OSXSAVE (1 << 18)
#define CR4_KL (1 << 19)
#define CR4_SMEP (1 << 20)
#define CR4_SMAP (1 << 21)
#define CR4_PKE (1 << 22)
#define CR4_CET (1 << 23)
#define CR4_PKS (1 << 24)

struct intr_descriptor {
	uint16_t offset_lo;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
#if defined(__x86_64__)
	uint64_t offset_hi; /* top 16 bits must be set to 0 */
	uint16_t padding;
#else /* defined(__x86_64__) */
	uint16_t offset_hi;
#endif /* defined(__x86_64__) */
} __attribute__((__packed__));

struct segment_descriptor {
	unsigned int limit_lo : 16;
	unsigned int baseaddr_lo : 24;
	unsigned int flags_lo : 8;
	unsigned int limit_hi : 4;
	unsigned int flags_hi : 4;
	unsigned int baseaddr_hi : 8;
} __attribute__((__packed__));

struct segment_descriptor64 {
	unsigned int limit_lo : 16;
	unsigned int baseaddr_lo : 24;
	unsigned int flags_lo : 8;
	unsigned int limit_hi : 4;
	unsigned int flags_hi : 4;
	uint64_t baseaddr_hi : 40;
	uint32_t reserved;
} __attribute__((__packed__));

struct page_table_entry_pae {
	unsigned int present: 1;
	unsigned int writable: 1;
	unsigned int user_access: 1;
	unsigned int write_through: 1;
	unsigned int disable_cache: 1;
	unsigned int accessed: 1;
	unsigned int dirty: 1;
	unsigned int page_type: 1;
	unsigned int global: 1;
	unsigned int padding: 3;
	uint64_t address: 40;
	unsigned int padding2: 7;
	unsigned int prot_key: 4;
	unsigned int noexec: 1;
} __attribute__((__packed__));

struct kvm_cpuid {
	unsigned int eax, ebx, ecx, edx;
};

struct kvm_cregs {
	unsigned long cr0, cr2, cr3, cr4;
};

struct kvm_sregs {
	uint16_t cs, ds, es, fs, gs, ss, tr;
};

struct kvm_regs64 {
	uint64_t rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
};

extern struct page_table_entry_pae kvm_pagetable[];
extern struct intr_descriptor kvm_idt[X86_INTR_COUNT];
extern struct segment_descriptor kvm_gdt[KVM_GDT_SIZE];

/* Page table helper functions */
uintptr_t kvm_get_page_address_pae(const struct page_table_entry_pae *entry);

/* Segment descriptor table functions */
void kvm_set_segment_descriptor(struct segment_descriptor *dst,
	uint64_t baseaddr, uint32_t limit, unsigned int flags);
void kvm_parse_segment_descriptor(struct segment_descriptor *src,
	uint64_t *baseaddr, uint32_t *limit, unsigned int *flags);
int kvm_find_free_descriptor(const struct segment_descriptor *table,
	size_t size);
unsigned int kvm_create_stack_descriptor(struct segment_descriptor *table,
	size_t tabsize, void *stack_base);

/* Functions for querying CPU info and status */
void kvm_get_cpuid(unsigned int eax, unsigned int ecx, struct kvm_cpuid *buf);
void kvm_read_cregs(struct kvm_cregs *buf);
void kvm_read_sregs(struct kvm_sregs *buf);
uint64_t kvm_rdmsr(unsigned int msr);
void kvm_wrmsr(unsigned int msr, uint64_t value);

/* Low-level interrupt handlers, DO NOT call directly */
void kvm_handle_bad_exception(void);
void kvm_handle_zerodiv(void);
void kvm_handle_debug(void);
void kvm_handle_nmi(void);
void kvm_handle_breakpoint(void);
void kvm_handle_overflow(void);
void kvm_handle_bound_range_exc(void);
void kvm_handle_bad_opcode(void);
void kvm_handle_device_error(void);
void kvm_handle_double_fault(void);
void kvm_handle_invalid_tss(void);
void kvm_handle_segfault(void);
void kvm_handle_stack_fault(void);
void kvm_handle_gpf(void);
void kvm_handle_page_fault(void);
void kvm_handle_fpu_error(void);
void kvm_handle_alignment_error(void);
void kvm_handle_machine_check(void);
void kvm_handle_simd_error(void);
void kvm_handle_virt_error(void);
void kvm_handle_cpe(void);
void kvm_handle_hv_injection(void);
void kvm_handle_vmm_comm(void);
void kvm_handle_security_error(void);

#endif /* KVM_X86_H_ */
