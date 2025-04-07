/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
 *
 * x86-specific KVM helper functions and structures for Intel VMX
 */

#ifndef KVM_X86_VMX_H_
#define KVM_X86_VMX_H_

#include "kvm_x86.h"

/* CPUID_GET_MODEL_INFO flags returned in ECX */
#define CPUID_MODEL_VMX (1 << 5)
#define CPUID_MODEL_SMX (1 << 6)

#define MSR_IA32_VMX_BASIC 0x480
#define MSR_IA32_VMX_PINX_MASK 0x481
#define MSR_IA32_VMX_EXECCTL_MASK 0x482
#define MSR_IA32_VMX_EXITCTL_MASK 0x483
#define MSR_IA32_VMX_ENTRYCTL_MASK 0x484
#define MSR_IA32_VMX_CR0_FIXED0 0x486
#define MSR_IA32_VMX_CR0_FIXED1 0x487
#define MSR_IA32_VMX_CR4_FIXED0 0x488
#define MSR_IA32_VMX_CR4_FIXED1 0x489
#define MSR_IA32_VMX_EXECCTL2_MASK 0x48b
#define MSR_IA32_VMX_PINX_MASK2 0x48d
#define MSR_IA32_VMX_EXECCTL_MASK2 0x48e
#define MSR_IA32_VMX_EXITCTL_MASK2 0x48f
#define MSR_IA32_VMX_ENTRYCTL_MASK2 0x490
#define MSR_IA32_VMX_EXECCTL3_MASK 0x492

#define VMX_CTLMASK_PINX 0
#define VMX_CTLMASK_EXECCTL 1
#define VMX_CTLMASK_EXECCTL2 2
#define VMX_CTLMASK_EXECCTL3 3
#define VMX_CTLMASK_EXITCTL 4
#define VMX_CTLMASK_ENTRYCTL 5
#define VMX_CTLMASK_MAX 6

#define IA32FC_LOCK (1 << 0)
#define IA32FC_VMXON_SMX (1 << 1)
#define IA32FC_VMXON_NORMAL (1 << 2)

#define IA32_VMXBASIC_USELESS_CTL_MASKS (1ULL << 55)

#define VMX_VMCS_GUEST_ES	0x800
#define VMX_VMCS_GUEST_CS	0x802
#define VMX_VMCS_GUEST_SS	0x804
#define VMX_VMCS_GUEST_DS	0x806
#define VMX_VMCS_GUEST_FS	0x808
#define VMX_VMCS_GUEST_GS	0x80a
#define VMX_VMCS_GUEST_LDTR	0x80c
#define VMX_VMCS_GUEST_TR	0x80e
#define VMX_VMCS_GUEST_INTR	0x810
#define VMX_VMCS_HOST_ES	0xc00
#define VMX_VMCS_HOST_CS	0xc02
#define VMX_VMCS_HOST_SS	0xc04
#define VMX_VMCS_HOST_DS	0xc06
#define VMX_VMCS_HOST_FS	0xc08
#define VMX_VMCS_HOST_GS	0xc0a
#define VMX_VMCS_HOST_TR	0xc0c

#define VMX_VMCS_MSR_BITMAP_POINTER	0x2004
#define VMX_VMCS_VIRT_APIC_POINTER	0x2012
#define VMX_VMCS_VIRT_APIC_BASE		0x2014
#define VMX_VMCS_LINK_POINTER		0x2800

#define VMX_VMCS_GUEST_ES_LIMIT		0x4800
#define VMX_VMCS_GUEST_CS_LIMIT		0x4802
#define VMX_VMCS_GUEST_SS_LIMIT		0x4804
#define VMX_VMCS_GUEST_DS_LIMIT		0x4806
#define VMX_VMCS_GUEST_FS_LIMIT		0x4808
#define VMX_VMCS_GUEST_GS_LIMIT		0x480a
#define VMX_VMCS_GUEST_LDTR_LIMIT	0x480c
#define VMX_VMCS_GUEST_TR_LIMIT		0x480e
#define VMX_VMCS_GUEST_GDTR_LIMIT	0x4810
#define VMX_VMCS_GUEST_IDTR_LIMIT	0x4812
#define VMX_VMCS_GUEST_ES_ACCESS	0x4814
#define VMX_VMCS_GUEST_CS_ACCESS	0x4816
#define VMX_VMCS_GUEST_SS_ACCESS	0x4818
#define VMX_VMCS_GUEST_DS_ACCESS	0x481a
#define VMX_VMCS_GUEST_FS_ACCESS	0x481c
#define VMX_VMCS_GUEST_GS_ACCESS	0x481e
#define VMX_VMCS_GUEST_LDTR_ACCESS	0x4820
#define VMX_VMCS_GUEST_TR_ACCESS	0x4822
#define VMX_VMCS_GUEST_INTR_STATE	0x4824
#define VMX_VMCS_GUEST_ACT_STATE	0x4826
#define VMX_VMCS_GUEST_SMBASE		0x4828
#define VMX_VMCS_GUEST_SYSENTER_CS	0x482a
#define VMX_VMCS_HOST_SYSENTER_CS	0x4c00

#define VMX_VMCS_GUEST_CR0		0x6800
#define VMX_VMCS_GUEST_CR3		0x6802
#define VMX_VMCS_GUEST_CR4		0x6804
#define VMX_VMCS_GUEST_ES_BASE		0x6806
#define VMX_VMCS_GUEST_CS_BASE		0x6808
#define VMX_VMCS_GUEST_SS_BASE		0x680a
#define VMX_VMCS_GUEST_DS_BASE		0x680c
#define VMX_VMCS_GUEST_FS_BASE		0x680e
#define VMX_VMCS_GUEST_GS_BASE		0x6810
#define VMX_VMCS_GUEST_LDTR_BASE	0x6812
#define VMX_VMCS_GUEST_TR_BASE		0x6814
#define VMX_VMCS_GUEST_GDTR_BASE	0x6816
#define VMX_VMCS_GUEST_IDTR_BASE	0x6818
#define VMX_VMCS_GUEST_DR7		0x681a
#define VMX_VMCS_GUEST_RSP		0x681c
#define VMX_VMCS_GUEST_RIP		0x681e
#define VMX_VMCS_GUEST_RFLAGS		0x6820
#define VMX_VMCS_GUEST_DEBUG_EXC	0x6822
#define VMX_VMCS_GUEST_SYSENTER_ESP	0x6824
#define VMX_VMCS_GUEST_SYSENTER_EIP	0x6826
#define VMX_VMCS_HOST_CR0		0x6c00
#define VMX_VMCS_HOST_CR3		0x6c02
#define VMX_VMCS_HOST_CR4		0x6c04
#define VMX_VMCS_HOST_FS_BASE		0x6c06
#define VMX_VMCS_HOST_GS_BASE		0x6c08
#define VMX_VMCS_HOST_TR_BASE		0x6c0a
#define VMX_VMCS_HOST_GDTR_BASE		0x6c0c
#define VMX_VMCS_HOST_IDTR_BASE		0x6c0e
#define VMX_VMCS_HOST_SYSENTER_ESP	0x6c10
#define VMX_VMCS_HOST_SYSENTER_EIP	0x6c12
#define VMX_VMCS_HOST_RSP		0x6c14
#define VMX_VMCS_HOST_RIP		0x6c16

#define VMX_VMCS_VMPINX_CTL		0x4000
#define VMX_VMCS_VMEXEC_CTL		0x4002
#define VMX_VMCS_VMEXIT_CTL		0x400c
#define VMX_VMCS_VMEXIT_MSR_STORE	0x400e
#define VMX_VMCS_VMEXIT_MSR_LOAD	0x4010
#define VMX_VMCS_VMENTRY_CTL		0x4012
#define VMX_VMCS_VMENTRY_MSR_LOAD	0x4014
#define VMX_VMCS_VMENTRY_INTR		0x4016
#define VMX_VMCS_VMENTRY_EXC		0x4018
#define VMX_VMCS_VMENTRY_INST_LEN	0x401a
#define VMX_VMCS_VMEXEC_CTL2		0x401e

#define VMX_VMCS_VMINST_ERROR		0x4400
#define VMX_VMCS_EXIT_REASON		0x4402
#define VMX_VMCS_VMEXIT_INTR_INFO	0x4404
#define VMX_VMCS_VMEXIT_INTR_ERRNO	0x4406
#define VMX_VMCS_IDTVEC_INFO		0x4408
#define VMX_VMCS_IDTVEC_ERRNO		0x440a
#define VMX_VMCS_VMEXIT_INST_LEN	0x440c
#define VMX_VMCS_VMEXIT_INST_INFO	0x440e
#define VMX_VMCS_EXIT_QUALIFICATION	0x6400

#define VMX_INTERCEPT_HLT (1 << 7)
#define VMX_EXECCTL_TPR_SHADOW (1 << 21)
#define VMX_EXECCTL_MSR_BITMAP (1 << 28)
#define VMX_EXECCTL_ENABLE_CTL2 (1 << 31)

#define VMX_EXECCTL2_VIRT_APIC (1 << 0)
#define VMX_EXECCTL2_VIRT_X2APIC (1 << 4)
#define VMX_EXECCTL2_VIRT_APIC_REG (1 << 8)
#define VMX_EXECCTL2_VIRT_INTR (1 << 9)
#define VMX_EXECCTL2_SHADOW_VMCS (1 << 14)

#define VMX_EXITCTL_SAVE_DR (1 << 2)
#define VMX_EXITCTL_X64 (1 << 9)

#define VMX_ENTRYCTL_LOAD_DR (1 << 2)
#define VMX_ENTRYCTL_X64 (1 << 9)

#define VMX_SHADOW_VMCS 0x80000000
#define VMX_VMCSFIELD_64BIT 0x2000
#define VMX_VMCSFIELD_SIZE_MASK 0x6000

#define VMX_INVALID_VMCS 0xffffffffffffffffULL

#define VMX_EXIT_HLT 12
#define VMX_EXIT_RDMSR 31
#define VMX_EXIT_FAILED_ENTRY 0x80000000

struct kvm_vmcs {
	uint32_t version;
	uint32_t abort;
	uint8_t data[4088];
};

struct kvm_vmx_vcpu {
	struct kvm_vmcs *vmcs;
	struct kvm_regs64 regs;
	int launched;
};

/* Intel VMX virtualization helper functions */
int kvm_is_vmx_supported(void);
void kvm_set_vmx_state(int enabled);
struct kvm_vmcs *kvm_alloc_vmcs(void);

/* Copy GDT entry to given fields of the current VMCS */
void kvm_vmcs_copy_gdt_descriptor(unsigned int gdt_id,
	unsigned long vmcs_selector, unsigned long vmcs_flags,
	unsigned long vmcs_limit, unsigned long vmcs_baseaddr);
uint64_t kvm_vmx_read_vmctl_mask(unsigned int ctl_id);
void kvm_init_vmx_vcpu(struct kvm_vmx_vcpu *cpu, uint16_t ss, void *rsp,
	int (*guest_main)(void));
struct kvm_vmx_vcpu *kvm_create_vmx_vcpu(int (*guest_main)(void),
	int alloc_stack);

/* Set the VMCS as current and update the host state fields */
void kvm_vmx_activate_vcpu(struct kvm_vmx_vcpu *cpu);
void kvm_vmx_vmrun(struct kvm_vmx_vcpu *cpu);

void kvm_vmx_vmclear(struct kvm_vmcs *buf);
void kvm_vmx_vmptrld(struct kvm_vmcs *buf);
uint64_t kvm_vmx_vmptrst(void);
uint64_t kvm_vmx_vmread(unsigned long var_id);
void kvm_vmx_vmwrite(unsigned long var_id, uint64_t value);
int kvm_vmx_vmlaunch(struct kvm_vmx_vcpu *buf);
int kvm_vmx_vmresume(struct kvm_vmx_vcpu *buf);

/* Read last VMX instruction error from current VMCS */
int kvm_vmx_inst_errno(void);
/* Get VMX instruction error description */
const char *kvm_vmx_inst_strerr(int vmx_errno);
/* Get description of last VMX instruction error in current VMCS */
const char *kvm_vmx_inst_err(void);

#endif /* KVM_X86_VMX_H_ */
