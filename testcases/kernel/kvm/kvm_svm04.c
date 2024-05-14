// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Functional test for VMSAVE/VMLOAD instructions in KVM environment. Verify
 * that both instructions save/load the CPU state according to CPU
 * documentation.
 */

#include "kvm_test.h"

#ifdef COMPILE_PAYLOAD
#if defined(__i386__) || defined(__x86_64__)

#include "kvm_x86_svm.h"

static struct kvm_vmcb *src_vmcb, *dest_vmcb, *msr_vmcb;
static struct kvm_sregs sregs_buf;

static int check_descriptor(const char *name,
	const struct kvm_vmcb_descriptor *data,
	const struct kvm_vmcb_descriptor *exp)
{
	int ret = 0;

	if (data->selector != exp->selector) {
		tst_res(TFAIL, "%s.selector = %hx (expected %hx)",
			name, data->selector, exp->selector);
		ret = 1;
	}

	if (data->attrib != exp->attrib) {
		tst_res(TFAIL, "%s.attrib = 0x%hx (expected 0x%hx)",
			name, data->attrib, exp->attrib);
		ret = 1;
	}

	if (data->limit != exp->limit) {
		tst_res(TFAIL, "%s.limit = 0x%x (expected 0x%x)",
			name, data->limit, exp->limit);
		ret = 1;
	}

	if (data->base != exp->base) {
		tst_res(TFAIL, "%s.base = 0x%llx (expected 0x%llx)",
			name, data->base, exp->base);
		ret = 1;
	}

	return ret;
}

static int check_value(const char *name, uint64_t val, uint64_t exp,
	uint64_t backup, uint64_t reg, uint64_t nested_val)
{
	int ret = 0;

	if (exp != backup) {
		tst_res(TFAIL, "%s source was modified (0x%llx != 0x%llx)",
			name, exp, backup);
		ret = 1;
	}

	if (reg != exp) {
		tst_res(TFAIL, "%s was not loaded (0x%llx != 0x%llx)",
			name, reg, exp);
		ret = 1;
	}

	if (val != exp) {
		tst_res(TFAIL, "%s was not saved (0x%llx != 0x%llx)",
			name, val, exp);
		ret = 1;
	}

	if (val != nested_val) {
		tst_res(TFAIL, "Inconsistent %s on VM exit (0x%llx != 0x%llx)",
			name, val, nested_val);
		ret = 1;
	}

	if (!ret)
		tst_res(TPASS, "%s has correct value 0x%llx", name, val);

	return ret;
}

static int vmsave_copy(void)
{
	kvm_svm_vmload(src_vmcb);
	kvm_read_sregs(&sregs_buf);
	msr_vmcb->star = kvm_rdmsr(MSR_STAR);
	msr_vmcb->lstar = kvm_rdmsr(MSR_LSTAR);
	msr_vmcb->cstar = kvm_rdmsr(MSR_CSTAR);
	msr_vmcb->sfmask = kvm_rdmsr(MSR_SFMASK);
	msr_vmcb->fs.base = kvm_rdmsr(MSR_FS_BASE);
	msr_vmcb->gs.base = kvm_rdmsr(MSR_GS_BASE);
	msr_vmcb->kernel_gs_base = kvm_rdmsr(MSR_KERNEL_GS_BASE);
	msr_vmcb->sysenter_cs = kvm_rdmsr(MSR_SYSENTER_CS);
	msr_vmcb->sysenter_esp = kvm_rdmsr(MSR_SYSENTER_ESP);
	msr_vmcb->sysenter_eip = kvm_rdmsr(MSR_SYSENTER_EIP);
	kvm_svm_vmsave(dest_vmcb);
	return 0;
}

static int check_vmsave_result(struct kvm_vmcb *copy_vmcb,
	struct kvm_vmcb *nested_vmcb)
{
	int ret = 0;

	/* Nested VMCB is only compared to dest VMCB, bypass the check */
	if (!nested_vmcb)
		nested_vmcb = dest_vmcb;

	ret = check_descriptor("FS", &dest_vmcb->fs, &src_vmcb->fs);
	ret = check_value("FS.selector", dest_vmcb->fs.selector,
		src_vmcb->fs.selector, copy_vmcb->fs.selector,
		sregs_buf.fs, nested_vmcb->fs.selector) || ret;
	ret = check_descriptor("GS", &dest_vmcb->gs, &src_vmcb->gs) || ret;
	ret = check_value("GS.selector", dest_vmcb->gs.selector,
		src_vmcb->gs.selector, copy_vmcb->gs.selector,
		sregs_buf.gs, nested_vmcb->gs.selector) || ret;
	ret = check_descriptor("LDTR", &dest_vmcb->ldtr, &src_vmcb->ldtr) ||
		ret;
	ret = check_descriptor("TR", &dest_vmcb->tr, &src_vmcb->tr) || ret;
	ret = check_value("STAR", dest_vmcb->star, src_vmcb->star,
		copy_vmcb->star, msr_vmcb->star, nested_vmcb->star) || ret;
	ret = check_value("LSTAR", dest_vmcb->lstar, src_vmcb->lstar,
		copy_vmcb->lstar, msr_vmcb->lstar, nested_vmcb->lstar) || ret;
	ret = check_value("CSTAR", dest_vmcb->cstar, src_vmcb->cstar,
		copy_vmcb->cstar, msr_vmcb->cstar, nested_vmcb->cstar) || ret;
	ret = check_value("SFMASK", dest_vmcb->sfmask, src_vmcb->sfmask,
		copy_vmcb->sfmask, msr_vmcb->sfmask, nested_vmcb->sfmask) ||
		ret;
	ret = check_value("FS.base", dest_vmcb->fs.base, src_vmcb->fs.base,
		copy_vmcb->fs.base, msr_vmcb->fs.base, nested_vmcb->fs.base) ||
		ret;
	ret = check_value("GS.base", dest_vmcb->gs.base, src_vmcb->gs.base,
		copy_vmcb->gs.base, msr_vmcb->gs.base, nested_vmcb->gs.base) ||
		ret;
	ret = check_value("KernelGSBase", dest_vmcb->kernel_gs_base,
		src_vmcb->kernel_gs_base, copy_vmcb->kernel_gs_base,
		msr_vmcb->kernel_gs_base, nested_vmcb->kernel_gs_base) || ret;
	ret = check_value("Sysenter_CS", dest_vmcb->sysenter_cs,
		src_vmcb->sysenter_cs, copy_vmcb->sysenter_cs,
		msr_vmcb->sysenter_cs, nested_vmcb->sysenter_cs) || ret;
	ret = check_value("Sysenter_ESP", dest_vmcb->sysenter_esp,
		src_vmcb->sysenter_esp, copy_vmcb->sysenter_esp,
		msr_vmcb->sysenter_esp, nested_vmcb->sysenter_esp) || ret;
	ret = check_value("Sysenter_EIP", dest_vmcb->sysenter_eip,
		src_vmcb->sysenter_eip, copy_vmcb->sysenter_eip,
		msr_vmcb->sysenter_eip, nested_vmcb->sysenter_eip) || ret;

	return ret;
}

static int create_segment_descriptor(uint64_t baseaddr, uint32_t limit,
	unsigned int flags)
{
	int ret = kvm_find_free_descriptor(kvm_gdt, KVM_GDT_SIZE);

	if (ret < 0)
		tst_brk(TBROK, "Descriptor table is full");

	kvm_set_segment_descriptor(kvm_gdt + ret, baseaddr, limit, flags);
	return ret;
}

static void dirty_vmcb(struct kvm_vmcb *buf)
{
	buf->fs.selector = 0x60;
	buf->fs.attrib = SEGTYPE_RWDATA | SEGFLAG_PRESENT;
	buf->fs.limit = 0xffff;
	buf->fs.base = 0xfff000;
	buf->gs.selector = 0x68;
	buf->gs.attrib = SEGTYPE_RWDATA | SEGFLAG_PRESENT;
	buf->gs.limit = 0xffff;
	buf->gs.base = 0xfff000;
	buf->ldtr.selector = 0x70;
	buf->ldtr.attrib = SEGTYPE_LDT | SEGFLAG_PRESENT;
	buf->ldtr.limit = 0xffff;
	buf->ldtr.base = 0xfff000;
	buf->tr.selector = 0x78;
	buf->tr.attrib = SEGTYPE_TSS | SEGFLAG_PRESENT;
	buf->tr.limit = 0xffff;
	buf->tr.base = 0xfff000;
	buf->star = 0xffff;
	buf->lstar = 0xffff;
	buf->cstar = 0xffff;
	buf->sfmask = 0xffff;
	buf->fs.base = 0xffff;
	buf->gs.base = 0xffff;
	buf->kernel_gs_base = 0xffff;
	buf->sysenter_cs = 0xffff;
	buf->sysenter_esp = 0xffff;
	buf->sysenter_eip = 0xffff;
}

void main(void)
{
	uint16_t ss;
	uint64_t rsp;
	struct kvm_svm_vcpu *vcpu;
	int data_seg1, data_seg2, ldt_seg, task_seg;
	struct segment_descriptor *ldt;
	struct kvm_vmcb *backup_vmcb, *zero_vmcb;
	unsigned int ldt_size = KVM_GDT_SIZE*sizeof(struct segment_descriptor);

	kvm_init_svm();

	src_vmcb = kvm_alloc_vmcb();
	dest_vmcb = kvm_alloc_vmcb();
	msr_vmcb = kvm_alloc_vmcb();
	backup_vmcb = kvm_alloc_vmcb();
	zero_vmcb = kvm_alloc_vmcb();

	vcpu = kvm_create_svm_vcpu(vmsave_copy, 1);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_VMLOAD, 0);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_VMSAVE, 0);
	/* Save allocated stack for later VM reinit */
	ss = vcpu->vmcb->ss.selector >> 3;
	rsp = vcpu->vmcb->rsp;

	ldt = tst_heap_alloc_aligned(ldt_size, 8);
	memset(ldt, 0, ldt_size);
	data_seg1 = create_segment_descriptor(0xda7a1000, 0x1000,
		SEGTYPE_RODATA | SEGFLAG_PRESENT);
	data_seg2 = create_segment_descriptor(0xda7a2000, 2,
		SEGTYPE_RWDATA | SEGFLAG_PRESENT | SEGFLAG_PAGE_LIMIT);
	ldt_seg = create_segment_descriptor((uintptr_t)ldt, ldt_size,
		SEGTYPE_LDT | SEGFLAG_PRESENT);
	task_seg = create_segment_descriptor(0x7a53000, 0x1000,
		SEGTYPE_TSS | SEGFLAG_PRESENT);
	kvm_vmcb_copy_gdt_descriptor(&src_vmcb->fs, data_seg1);
	kvm_vmcb_copy_gdt_descriptor(&src_vmcb->gs, data_seg2);
	kvm_vmcb_copy_gdt_descriptor(&src_vmcb->ldtr, ldt_seg);
	kvm_vmcb_copy_gdt_descriptor(&src_vmcb->tr, task_seg);

	src_vmcb->star = 0x5742;
	src_vmcb->lstar = 0x15742;
	src_vmcb->cstar = 0xc5742;
	src_vmcb->sfmask = 0xf731;
	src_vmcb->fs.base = 0xf000;
	src_vmcb->gs.base = 0x10000;
	src_vmcb->kernel_gs_base = 0x20000;
	src_vmcb->sysenter_cs = 0x595c5;
	src_vmcb->sysenter_esp = 0x595e50;
	src_vmcb->sysenter_eip = 0x595e10;

	memcpy(backup_vmcb, src_vmcb, sizeof(struct kvm_vmcb));
	tst_res(TINFO, "VMLOAD/VMSAVE non-zero values");
	vmsave_copy();
	check_vmsave_result(backup_vmcb, NULL);

	memset(src_vmcb, 0, sizeof(struct kvm_vmcb));
	tst_res(TINFO, "VMLOAD/VMSAVE zero values");
	dirty_vmcb(dest_vmcb);
	vmsave_copy();
	check_vmsave_result(zero_vmcb, NULL);

	memcpy(src_vmcb, backup_vmcb, sizeof(struct kvm_vmcb));
	tst_res(TINFO, "Nested VMLOAD/VMSAVE non-zero values");
	dirty_vmcb(vcpu->vmcb);
	memset(dest_vmcb, 0, sizeof(struct kvm_vmcb));
	kvm_svm_vmrun(vcpu);

	if (vcpu->vmcb->exitcode != SVM_EXIT_HLT)
		tst_brk(TBROK, "Nested VM exited unexpectedly");

	check_vmsave_result(backup_vmcb, vcpu->vmcb);

	memset(src_vmcb, 0, sizeof(struct kvm_vmcb));
	tst_res(TINFO, "Nested VMLOAD/VMSAVE zero values");
	kvm_init_guest_vmcb(vcpu->vmcb, 1, ss, (void *)rsp, vmsave_copy);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_VMLOAD, 0);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_VMSAVE, 0);
	dirty_vmcb(vcpu->vmcb);
	kvm_svm_vmrun(vcpu);

	if (vcpu->vmcb->exitcode != SVM_EXIT_HLT)
		tst_brk(TBROK, "Nested VM exited unexpectedly");

	check_vmsave_result(zero_vmcb, vcpu->vmcb);
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
};

#endif /* COMPILE_PAYLOAD */
