/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 SUSE LLC <mdoucha@suse.cz>
 *
 * x86-specific KVM helper functions and structures for AMD SVM
 */

#ifndef KVM_X86_SVM_H_
#define KVM_X86_SVM_H_

#include "kvm_x86.h"

/* CPUID_GET_SVM_FEATURES flags returned in EDX */
#define SVM_CPUID_NESTED_PAGING (1 << 0)
#define SVM_CPUID_LBR_VIRT (1 << 1)
#define SVM_CPUID_LOCK (1 << 2)
#define SVM_CPUID_NRIP_SAVE (1 << 3)
#define SVM_CPUID_TSC_RATE_MSR (1 << 4)
#define SVM_CPUID_VMCB_CLEAN (1 << 5)
#define SVM_CPUID_FLUSH_ASID (1 << 6)
#define SVM_CPUID_DECODE_ASSIST (1 << 7)
#define SVM_CPUID_PAUSE_FILTER (1 << 10)
#define SVM_CPUID_PAUSE_THRESHOLD (1 << 12)
#define SVM_CPUID_AVIC (1 << 13)
#define SVM_CPUID_VMSAVE_VIRT (1 << 15)
#define SVM_CPUID_VGIF (1 << 16)
#define SVM_CPUID_GMET (1 << 17)
#define SVM_CPUID_X2AVIC (1 << 18)
#define SVM_CPUID_SSSCHECK (1 << 19)
#define SVM_CPUID_SPEC_CTRL (1 << 20)
#define SVM_CPUID_ROGPT (1 << 21)
#define SVM_CPUID_HOST_MCE_OVERRIDE (1 << 23)
#define SVM_CPUID_TLBI_CTL (1 << 24)
#define SVM_CPUID_NMI_VIRT (1 << 25)
#define SVM_CPUID_IBS_VIRT (1 << 26)

/* SVM event intercept IDs */
#define SVM_INTERCEPT_HLT 0x78
#define SVM_INTERCEPT_VMRUN 0x80
#define SVM_INTERCEPT_VMLOAD 0x82
#define SVM_INTERCEPT_VMSAVE 0x83
#define SVM_INTERCEPT_MAX 0x95

/* SVM vmrun exit codes */
#define SVM_EXIT_HLT 0x78
#define SVM_EXIT_VMRUN 0x80
#define SVM_EXIT_VMLOAD 0x82
#define SVM_EXIT_VMSAVE 0x83
#define SVM_EXIT_AVIC_NOACCEL 0x402
#define SVM_EXIT_INVALID ((uint64_t)-1)

/* SVM VMCB flags */
#define SVM_INTR_AVIC (1 << 7)

struct kvm_vmcb_descriptor {
	uint16_t selector;
	uint16_t attrib;
	uint32_t limit;
	uint64_t base;
};

struct kvm_vmcb {
	/* VMCB control area */
	uint8_t intercepts[20];
	uint8_t reserved1[44];
	uint64_t iopm_base_addr;
	uint64_t msrpm_base_addr;
	uint64_t tsc_offset;
	uint32_t guest_asid;
	uint32_t tlb_control;
	uint8_t virtual_tpr;
	uint8_t virtual_irq;
	unsigned char virt_intr_prio: 4;
	unsigned char virt_ignore_tpr: 4;
	uint8_t virt_intr_ctl;
	uint8_t virt_intr_vector;
	uint8_t reserved2[3];
	uint64_t interrupt_shadow;
	uint64_t exitcode;
	uint64_t exitinfo1;
	uint64_t exitinfo2;
	uint64_t exit_int_info;
	uint64_t enable_nested_paging;
	uint64_t avic_bar;
	uint64_t ghcb_gpa;
	uint64_t event_injection;
	uint64_t nested_cr3;
	uint64_t virt_ext;
	uint32_t vmcb_clean;
	uint8_t reserved3[4];
	uint64_t next_rip;
	uint8_t instr_len;
	uint8_t instr_bytes[15];
	uint64_t avic_backing_page;
	uint8_t reserved4[8];
	uint64_t avic_logical_ptr;
	uint64_t avic_physical_ptr;
	uint8_t reserved5[8];
	uint64_t vmsa_pa;
	uint64_t vmgexit_rax;
	uint8_t vmgexit_cpl;
	uint8_t reserved6[0x2e7];

	/* VMCB state save area */
	struct kvm_vmcb_descriptor es, cs, ss, ds, fs, gs;
	struct kvm_vmcb_descriptor gdtr, ldtr, idtr, tr;
	uint8_t reserved7[43];
	uint8_t cpl;
	uint8_t reserved8[4];
	uint64_t efer;
	uint8_t reserved9[112];
	uint64_t cr4;
	uint64_t cr3;
	uint64_t cr0;
	uint64_t dr7;
	uint64_t dr6;
	uint64_t rflags;
	uint64_t rip;
	uint8_t reserved10[88];
	uint64_t rsp;
	uint64_t s_cet;
	uint64_t ssp;
	uint64_t isst_addr;
	uint64_t rax;
	uint64_t star;
	uint64_t lstar;
	uint64_t cstar;
	uint64_t sfmask;
	uint64_t kernel_gs_base;
	uint64_t sysenter_cs;
	uint64_t sysenter_esp;
	uint64_t sysenter_eip;
	uint64_t cr2;
	uint8_t reserved11[32];
	uint64_t guest_pat;
	uint8_t padding[0x990];
};

struct kvm_svm_vcpu {
	struct kvm_vmcb *vmcb;
	struct kvm_regs64 regs;
};

/* AMD SVM virtualization helper functions */
int kvm_is_svm_supported(void);
int kvm_get_svm_state(void);
void kvm_set_svm_state(int enabled);

void kvm_init_svm(void);	/* Fully initialize host SVM environment */
struct kvm_vmcb *kvm_alloc_vmcb(void);
void kvm_vmcb_copy_gdt_descriptor(struct kvm_vmcb_descriptor *dst,
	unsigned int gdt_id);
void kvm_vmcb_set_intercept(struct kvm_vmcb *vmcb, unsigned int id,
	unsigned int state);
void kvm_init_guest_vmcb(struct kvm_vmcb *vmcb, uint32_t asid, uint16_t ss,
	void *rsp, int (*guest_main)(void));
struct kvm_svm_vcpu *kvm_create_svm_vcpu(int (*guest_main)(void),
	int alloc_stack);

void kvm_svm_vmrun(struct kvm_svm_vcpu *cpu);

#endif /* KVM_X86_SVM_H_ */
