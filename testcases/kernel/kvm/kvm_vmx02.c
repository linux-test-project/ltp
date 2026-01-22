// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Verify that reads and writes to virtualized APIC in nested VM get
 * redirected to the memory page selected by parent VM.
 */

#include "kvm_test.h"

#ifdef COMPILE_PAYLOAD
#if defined(__i386__) || defined(__x86_64__)

#include "kvm_x86_vmx.h"

#define TPR_OFFSET 0x80
#define TPR_VALUE 0x7
#define TPR_OLD_VALUE 0x9
#define MSR_TPR 0x808

static void *apic_base;

int memapic_guest_main(void)
{
	int ret;
	uint32_t *ptr;

	ptr = apic_base + TPR_OFFSET;
	ret = *ptr;
	*ptr = TPR_VALUE;
	return ret;
}

int msrapic_guest_main(void)
{
	int ret;

	ret = kvm_rdmsr(MSR_TPR);
	kvm_wrmsr(MSR_TPR, TPR_VALUE);
	return ret;
}

int setup_vmcs(void *apic_ptr, void *msr_mask, uint64_t ec2_flags)
{
	uint32_t *tpr;
	uint64_t val, mask, flags;

	/* Check secondary VMCS execctl support */
	mask = kvm_vmx_read_vmctl_mask(VMX_CTLMASK_EXECCTL);

	if (!((mask >> 32) & VMX_EXECCTL_ENABLE_CTL2))
		return 0;

	/* Create and configure guest VMCS */
	tpr = apic_ptr + TPR_OFFSET;
	memset(apic_ptr, 0xa9, PAGESIZE);
	memset(apic_base, 0xab, PAGESIZE);
	*tpr = TPR_OLD_VALUE;
	val = kvm_vmx_vmread(VMX_VMCS_VMEXEC_CTL);
	flags = VMX_EXECCTL_TPR_SHADOW | VMX_EXECCTL_ENABLE_CTL2;
	flags |= VMX_EXECCTL_MSR_BITMAP;
	flags &= mask >> 32;
	kvm_vmx_vmwrite(VMX_VMCS_VMEXEC_CTL, val | flags);

	if (flags & VMX_EXECCTL_MSR_BITMAP) {
		kvm_vmx_vmwrite(VMX_VMCS_MSR_BITMAP_POINTER,
			(uintptr_t)msr_mask);
	}

	mask = kvm_vmx_read_vmctl_mask(VMX_CTLMASK_EXECCTL2);
	ec2_flags &= mask >> 32;

	if (!ec2_flags)
		return 0;

	val = ec2_flags | (uint32_t)mask;
	kvm_vmx_vmwrite(VMX_VMCS_VMEXEC_CTL2, val);
	kvm_vmx_vmwrite(VMX_VMCS_VIRT_APIC_POINTER, (uintptr_t)apic_ptr);
	kvm_vmx_vmwrite(VMX_VMCS_VIRT_APIC_BASE, (uintptr_t)apic_base);
	return 1;
}

void check_result(struct kvm_vmx_vcpu *vcpu, unsigned int tpr,
	unsigned int tpr_b)
{
	/* Cast RAX value to int. The upper 32 bits may contain garbage. */
	int tpr_orig = vcpu->regs.rax;

	if (tpr_orig == TPR_OLD_VALUE)
		tst_res(TPASS, "vTPR has correct value");
	else
		tst_res(TFAIL, "Unexpected vTPR value: %x", tpr_orig);

	if (tpr == TPR_VALUE)
		tst_res(TPASS, "vAPIC write was handled correctly");
	else if (tpr_b == TPR_VALUE)
		tst_res(TFAIL, "vAPIC write was stored to literal address");
	else
		tst_res(TFAIL, "vAPIC write may have overwritten host memory!");
}

void main(void)
{
	void *apic_ptr, *msr_mask;
	struct kvm_vmx_vcpu *vcpu;
	uintptr_t rsp;
	uint32_t *tpr, *tpr_b;
	uint64_t reason;
	uint16_t ss;

	kvm_set_vmx_state(1);

	apic_ptr = tst_heap_alloc_aligned(2 * PAGESIZE, PAGESIZE);
	apic_base = apic_ptr + PAGESIZE;
	tpr = apic_ptr + TPR_OFFSET;
	tpr_b = apic_base + TPR_OFFSET;
	msr_mask = tst_heap_alloc_aligned(PAGESIZE, PAGESIZE);
	memset(msr_mask, 0, PAGESIZE);
	vcpu = kvm_create_vmx_vcpu(memapic_guest_main, 1);
	kvm_vmx_vmptrld(vcpu->vmcs);
	ss = kvm_vmx_vmread(VMX_VMCS_GUEST_SS) >> 3;
	rsp = kvm_vmx_vmread(VMX_VMCS_GUEST_RSP);
	tst_res(TINFO, "Testing memory-mapped APIC");

	if (setup_vmcs(apic_ptr, msr_mask, VMX_EXECCTL2_VIRT_APIC)) {
		kvm_vmx_vmrun(vcpu);
		reason = kvm_vmx_vmread(VMX_VMCS_EXIT_REASON);

		if (reason != VMX_EXIT_HLT) {
			tst_res(TFAIL, "Unexpected guest exit reason %llx",
				reason);
		} else {
			check_result(vcpu, *tpr, *tpr_b);
		}
	} else {
		tst_res(TCONF, "CPU does not support memory mapped vAPIC");
	}

	tst_res(TINFO, "Testing MSR-based APIC");
	kvm_init_vmx_vcpu(vcpu, ss, (void *)rsp, msrapic_guest_main);

	if (setup_vmcs(apic_ptr, msr_mask, VMX_EXECCTL2_VIRT_X2APIC)) {
		kvm_vmx_vmrun(vcpu);
		reason = kvm_vmx_vmread(VMX_VMCS_EXIT_REASON);

		if (reason == VMX_EXIT_RDMSR) {
			tst_res(TCONF, "CPU does not support MSR bitmaps");
		} else if (reason != VMX_EXIT_HLT) {
			tst_res(TFAIL, "Unexpected guest exit reason %llx",
				reason);
		} else {
			check_result(vcpu, *tpr, *tpr_b);
		}
	} else {
		tst_res(TCONF, "CPU does not support MSR-based vAPIC");
	}
}

#else /* defined(__i386__) || defined(__x86_64__) */
TST_TEST_TCONF("Test supported only on x86");
#endif /* defined(__i386__) || defined(__x86_64__) */

#else /* COMPILE_PAYLOAD */

#include "tst_module.h"

#define NESTED_INTEL_SYSFILE "/sys/module/kvm_intel/parameters/nested"

static void setup(void)
{
	if (!tst_read_bool_sys_param(NESTED_INTEL_SYSFILE)) {
		tst_module_reload("kvm_intel",
			(char *const[]){"nested=1", NULL});
	}

	tst_kvm_setup();
}

static struct tst_test test = {
	.test_all = tst_kvm_run,
	.setup = setup,
	.cleanup = tst_kvm_cleanup,
	.needs_root = 1,
	.needs_drivers = (const char *const []) {
		"kvm",
		NULL
	},
	.supported_archs = (const char *const []) {
		"x86_64",
		"x86",
		NULL
	},
};

#endif /* COMPILE_PAYLOAD */
