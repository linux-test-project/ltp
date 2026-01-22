// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * CVE 2021-3653
 *
 * Check that KVM either blocks enabling virtual interrupt controller (AVIC)
 * in nested VMs or correctly sets up the required memory address translation.
 * If AVIC is enabled without address translation in the host kernel,
 * the nested VM will be able to read and write an arbitraty physical memory
 * page specified by the parent VM. Unauthorized memory access fixed in:
 *
 *  commit 0f923e07124df069ba68d8bb12324398f4b6b709
 *  Author: Maxim Levitsky <mlevitsk@redhat.com>
 *  Date:   Thu Jul 15 01:56:24 2021 +0300
 *
 *  KVM: nSVM: avoid picking up unsupported bits from L2 in int_ctl (CVE-2021-3653)
 */

#include "kvm_test.h"

#ifdef COMPILE_PAYLOAD
#if defined(__i386__) || defined(__x86_64__)

#include "kvm_x86_svm.h"

#define AVIC_REG_ADDR 0x280
#define AVIC_TEST_VAL 0xec
#define AVIC_READ_FAIL 0x12ead

#define AVIC_INFO_MASK ((1ULL << 32) | 0xff0)
#define AVIC_INFO_EXP ((1ULL << 32) | AVIC_REG_ADDR)

static uint32_t * const avic_ptr = (uint32_t *)AVIC_REG_ADDR;

static int guest_main(void)
{
	if (*avic_ptr != 0xaaaaaaaa)
		return AVIC_READ_FAIL;

	*avic_ptr = AVIC_TEST_VAL;
	return 0;
}

void main(void)
{
	struct kvm_svm_vcpu *vcpu;

	kvm_init_svm();
	vcpu = kvm_create_svm_vcpu(guest_main, 1);

	/*
	 * Enable AVIC and set both the AVIC base address (where the nested VM
	 * will write) and backing page address (where the parent VM expects
	 * to see the changes) to 0
	 */
	vcpu->vmcb->virt_intr_ctl |= SVM_INTR_AVIC;
	vcpu->vmcb->avic_backing_page = 0;
	vcpu->vmcb->avic_bar = 0;
	memset((void *)8, 0xaa, PAGESIZE - 8);

	/* Write into AVIC backing page in the nested VM */
	kvm_svm_vmrun(vcpu);

	switch (vcpu->vmcb->exitcode) {
	case SVM_EXIT_HLT:
		if (vcpu->vmcb->rax == AVIC_READ_FAIL) {
			tst_res(TFAIL, "Nested VM can read host memory");
			return;
		}

		if (vcpu->vmcb->rax)
			tst_brk(TBROK, "Unexpected guest_main() return value");

		break;

	case SVM_EXIT_AVIC_NOACCEL:
		if ((vcpu->vmcb->exitinfo1 & AVIC_INFO_MASK) == AVIC_INFO_EXP) {
			tst_res(TPASS, "AVIC register write caused VMEXIT");
			break;
		}

		/* unexpected exit, fall through */

	default:
		tst_brk(TBROK, "Nested VM exited unexpectedly");
	}

	if (*avic_ptr != AVIC_TEST_VAL) {
		tst_res(TFAIL, "Write into AVIC ESR redirected to host memory");
		return;
	}

	tst_res(TPASS, "Writes into AVIC backing page were not redirected");
}

#else /* defined(__i386__) || defined(__x86_64__) */
TST_TEST_TCONF("Test supported only on x86");
#endif /* defined(__i386__) || defined(__x86_64__) */

#else /* COMPILE_PAYLOAD */

static struct tst_test test = {
	.test_all = tst_kvm_run,
	.setup = tst_kvm_setup,
	.cleanup = tst_kvm_cleanup,
	.needs_drivers = (const char *const []) {
		"kvm",
		NULL
	},
	.supported_archs = (const char *const []) {
		"x86_64",
		"x86",
		NULL
	},
	.tags = (struct tst_tag[]){
		{"linux-git", "0f923e07124d"},
		{"CVE", "2021-3653"},
		{}
	}
};

#endif /* COMPILE_PAYLOAD */
