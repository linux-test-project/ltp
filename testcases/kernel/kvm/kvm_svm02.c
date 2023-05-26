// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * CVE 2021-3656
 *
 * Check that KVM correctly intercepts VMSAVE and VMLOAD instructions
 * in a nested virtual machine even when the parent guest disables
 * intercepting either instruction. If KVM does not override the disabled
 * intercepts, it'll give the nested VM read/write access to a few bytes
 * of an arbitrary physical memory page. Unauthorized memory access fixed in:
 *
 *  commit c7dfa4009965a9b2d7b329ee970eb8da0d32f0bc
 *  Author: Maxim Levitsky <mlevitsk@redhat.com>
 *  Date:   Mon Jul 19 16:05:00 2021 +0300
 *
 *  KVM: nSVM: always intercept VMLOAD/VMSAVE when nested (CVE-2021-3656)
 */

#include "kvm_test.h"

#ifdef COMPILE_PAYLOAD
#if defined(__i386__) || defined(__x86_64__)

#include "kvm_x86_svm.h"

static void *vmsave_buf;

/* Load FS, GS, TR and LDTR state from vmsave_buf */
static int guest_vmload(void)
{
	asm (
		"vmload %0\n"
		:
		: "a" (vmsave_buf)
	);
	return 0;
}

/* Save current FS, GS, TR and LDTR state to vmsave_buf */
static int guest_vmsave(void)
{
	asm (
		"vmsave %0\n"
		:
		: "a" (vmsave_buf)
	);
	return 0;
}

static int cmp_descriptor(const struct kvm_vmcb_descriptor *a,
	const struct kvm_vmcb_descriptor *b)
{
	int ret;

	ret = a->selector != b->selector;
	ret = ret || a->attrib != b->attrib;
	ret = ret || a->limit != b->limit;
	ret = ret || a->base != b->base;
	return ret;
}

/* Return non-zero if the VMCB fields touched by vmsave/vmload differ */
static int cmp_vmcb(const struct kvm_vmcb *a, const struct kvm_vmcb *b)
{
	int ret;

	ret = cmp_descriptor(&a->fs, &b->fs);
	ret = ret || cmp_descriptor(&a->gs, &b->gs);
	ret = ret || cmp_descriptor(&a->tr, &b->tr);
	ret = ret || cmp_descriptor(&a->ldtr, &b->ldtr);
	ret = ret || a->kernel_gs_base != b->kernel_gs_base;
	ret = ret || a->star != b->star;
	ret = ret || a->lstar != b->lstar;
	ret = ret || a->cstar != b->cstar;
	ret = ret || a->sfmask != b->sfmask;
	ret = ret || a->sysenter_cs != b->sysenter_cs;
	ret = ret || a->sysenter_esp != b->sysenter_esp;
	ret = ret || a->sysenter_eip != b->sysenter_eip;
	return ret;
}

void main(void)
{
	uint16_t ss;
	uint64_t rsp;
	struct kvm_svm_vcpu *vcpu;

	kvm_init_svm();
	vcpu = kvm_create_svm_vcpu(guest_vmload, 1);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_VMLOAD, 0);
	vmsave_buf = kvm_alloc_vmcb();

	/* Save allocated stack for later VM reinit */
	ss = vcpu->vmcb->ss.selector;
	rsp = vcpu->vmcb->rsp;

	/* Load partial state from vmsave_buf and save it to vcpu->vmcb */
	kvm_svm_vmrun(vcpu);

	if (vcpu->vmcb->exitcode != SVM_EXIT_HLT)
		tst_brk(TBROK, "Nested VM exited unexpectedly");

	if (cmp_vmcb(vcpu->vmcb, vmsave_buf)) {
		tst_res(TFAIL, "Nested VM can read host memory");
		return;
	}

	/* Load state from vcpu->vmcb and save it to vmsave_buf */
	memset(vmsave_buf, 0xaa, sizeof(struct kvm_vmcb));
	kvm_init_guest_vmcb(vcpu->vmcb, 1, ss, (void *)rsp, guest_vmsave);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_VMSAVE, 0);
	kvm_svm_vmrun(vcpu);

	if (vcpu->vmcb->exitcode != SVM_EXIT_HLT)
		tst_brk(TBROK, "Nested VM exited unexpectedly");

	if (cmp_vmcb(vcpu->vmcb, vmsave_buf)) {
		tst_res(TFAIL, "Nested VM can overwrite host memory");
		return;
	}

	tst_res(TPASS, "VMLOAD and VMSAVE were intercepted by kernel");
}

#else /* defined(__i386__) || defined(__x86_64__) */
TST_TEST_TCONF("Test supported only on x86");
#endif /* defined(__i386__) || defined(__x86_64__) */

#else /* COMPILE_PAYLOAD */

static struct tst_test test = {
	.test_all = tst_kvm_run,
	.setup = tst_kvm_setup,
	.cleanup = tst_kvm_cleanup,
	.supported_archs = (const char *const []) {
		"x86_64",
		"x86",
		NULL
	},
	.tags = (struct tst_tag[]){
		{"linux-git", "c7dfa4009965"},
		{"CVE", "2021-3656"},
		{}
	}
};

#endif /* COMPILE_PAYLOAD */
