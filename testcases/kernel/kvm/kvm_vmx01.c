// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Basic functional test for VMREAD/VMWRITE instructions in KVM environment.
 * Verify that VMWRITE instruction changes the contents of current VMCS and
 * the values written into shadow VMCS can be read in both parent and nested
 * VM.
 */

#include "kvm_test.h"

#ifdef COMPILE_PAYLOAD
#if defined(__i386__) || defined(__x86_64__)

#include "kvm_x86_vmx.h"

#define GUEST_READ_ERROR 1
#define GUEST_WRITE_ERROR 2
#define SHADOW_DATA_LENGTH 37
#define VMCS_FIELD(x) x, #x

struct vmcs_field_table {
	unsigned long field_id;
	const char *name;
	unsigned long value;
};

/* Data written into shadow VMCS by the parent VM and read by the nested VM */
static struct vmcs_field_table host_data[SHADOW_DATA_LENGTH] = {
	{VMCS_FIELD(VMX_VMCS_GUEST_ES), 0xe5},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS), 0xc5},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS), 0x55},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS), 0xd5},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS), 0xf5},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS), 0x65},
	{VMCS_FIELD(VMX_VMCS_GUEST_LDTR), 0x1d72},
	{VMCS_FIELD(VMX_VMCS_GUEST_TR), 0x72},
	{VMCS_FIELD(VMX_VMCS_HOST_ES), 0x5e},
	{VMCS_FIELD(VMX_VMCS_HOST_CS), 0x5c},
	{VMCS_FIELD(VMX_VMCS_HOST_SS), 0x55},
	{VMCS_FIELD(VMX_VMCS_HOST_DS), 0x5d},
	{VMCS_FIELD(VMX_VMCS_HOST_FS), 0x5f},
	{VMCS_FIELD(VMX_VMCS_HOST_GS), 0x56},
	{VMCS_FIELD(VMX_VMCS_HOST_TR), 0x27},
	{VMCS_FIELD(VMX_VMCS_GUEST_ES_LIMIT), 0xe51},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS_LIMIT), 0xc51},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS_LIMIT), 0x551},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS_LIMIT), 0xd51},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS_LIMIT), 0xf51},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS_LIMIT), 0x651},
	{VMCS_FIELD(VMX_VMCS_GUEST_LDTR_LIMIT), 0x1d721},
	{VMCS_FIELD(VMX_VMCS_GUEST_ES_ACCESS), 0xa0e5},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS_ACCESS), 0xa0c5},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS_ACCESS), 0xa055},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS_ACCESS), 0xa0d5},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS_ACCESS), 0xa0f5},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS_ACCESS), 0xa065},
	{VMCS_FIELD(VMX_VMCS_GUEST_SYSENTER_CS), 0x65c},
	{VMCS_FIELD(VMX_VMCS_HOST_SYSENTER_CS), 0x45c},
	{VMCS_FIELD(VMX_VMCS_GUEST_ES_BASE), 0xe5b},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS_BASE), 0xc5b},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS_BASE), 0x55b},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS_BASE), 0xd5b},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS_BASE), 0xf5b},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS_BASE), 0x65b},
	{VMCS_FIELD(VMX_VMCS_GUEST_LDTR_BASE), 0x1d72b}
};

/* Data written into shadow VMCS by the nested VM and read by the parent VM */
static struct vmcs_field_table guest_data[SHADOW_DATA_LENGTH] = {
	{VMCS_FIELD(VMX_VMCS_GUEST_ES), 0x5e},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS), 0x5c},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS), 0x55},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS), 0x5d},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS), 0x5f},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS), 0x56},
	{VMCS_FIELD(VMX_VMCS_GUEST_LDTR), 0x721d},
	{VMCS_FIELD(VMX_VMCS_GUEST_TR), 0x27},
	{VMCS_FIELD(VMX_VMCS_HOST_ES), 0xe5},
	{VMCS_FIELD(VMX_VMCS_HOST_CS), 0xc5},
	{VMCS_FIELD(VMX_VMCS_HOST_SS), 0x55},
	{VMCS_FIELD(VMX_VMCS_HOST_DS), 0xd5},
	{VMCS_FIELD(VMX_VMCS_HOST_FS), 0xf5},
	{VMCS_FIELD(VMX_VMCS_HOST_GS), 0x65},
	{VMCS_FIELD(VMX_VMCS_HOST_TR), 0x72},
	{VMCS_FIELD(VMX_VMCS_GUEST_ES_LIMIT), 0x1e5},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS_LIMIT), 0x1c5},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS_LIMIT), 0x155},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS_LIMIT), 0x1d5},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS_LIMIT), 0x1f5},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS_LIMIT), 0x165},
	{VMCS_FIELD(VMX_VMCS_GUEST_LDTR_LIMIT), 0x11d72},
	{VMCS_FIELD(VMX_VMCS_GUEST_ES_ACCESS), 0xa05e},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS_ACCESS), 0xa05c},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS_ACCESS), 0xa055},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS_ACCESS), 0xa05d},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS_ACCESS), 0xa05f},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS_ACCESS), 0xa056},
	{VMCS_FIELD(VMX_VMCS_GUEST_SYSENTER_CS), 0x5c6},
	{VMCS_FIELD(VMX_VMCS_HOST_SYSENTER_CS), 0x5c4},
	{VMCS_FIELD(VMX_VMCS_GUEST_ES_BASE), 0xbe5},
	{VMCS_FIELD(VMX_VMCS_GUEST_CS_BASE), 0xbc5},
	{VMCS_FIELD(VMX_VMCS_GUEST_SS_BASE), 0xb55},
	{VMCS_FIELD(VMX_VMCS_GUEST_DS_BASE), 0xbd5},
	{VMCS_FIELD(VMX_VMCS_GUEST_FS_BASE), 0xbf5},
	{VMCS_FIELD(VMX_VMCS_GUEST_GS_BASE), 0xb65},
	{VMCS_FIELD(VMX_VMCS_GUEST_LDTR_BASE), 0xb1d72}
};

static unsigned long vmread_buffer[SHADOW_DATA_LENGTH];

int guest_main(void)
{
	int i;

	/* kvm_vmx_vmread() calls tst_brk(), don't use it in nested VM */
	for (i = 0; i < SHADOW_DATA_LENGTH; i++) {
		asm goto(
			"vmread %1, (%0)\n"
			"jna %l[read_error]\n"
			"vmwrite %2, %3\n"
			"jna %l[write_error]\n"
			:
			: "r" (&vmread_buffer[i]), "r" (host_data[i].field_id),
				"r" (guest_data[i].value),
				"r" (guest_data[i].field_id)
			: "cc", "memory"
			: read_error, write_error
		);
	}

	return 0;

read_error:
	return GUEST_READ_ERROR;

write_error:
	return GUEST_WRITE_ERROR;
}

void main(void)
{
	struct kvm_vmx_vcpu *vcpu;
	struct kvm_vmcs *shadow_vmcs;
	char *vmcs_backup;
	int i, errors;
	uint64_t val;

	kvm_set_vmx_state(1);

	/* Check secondary VMCS execctl support */
	val = kvm_vmx_read_vmctl_mask(VMX_CTLMASK_EXECCTL);

	if (!((val >> 32) & VMX_EXECCTL_ENABLE_CTL2))
		tst_brk(TCONF, "CPU does not support shadow VMCS");

	/* Create and configure guest VMCS */
	shadow_vmcs = kvm_alloc_vmcs();
	kvm_vmx_vmclear(shadow_vmcs);
	shadow_vmcs->version |= VMX_SHADOW_VMCS;
	vcpu = kvm_create_vmx_vcpu(guest_main, 1);
	kvm_vmx_vmptrld(vcpu->vmcs);
	val = kvm_vmx_vmread(VMX_VMCS_VMEXEC_CTL);
	val |= VMX_EXECCTL_ENABLE_CTL2;
	kvm_vmx_vmwrite(VMX_VMCS_VMEXEC_CTL, val);
	val = kvm_vmx_read_vmctl_mask(VMX_CTLMASK_EXECCTL2);

	if (!((val >> 32) & VMX_EXECCTL2_SHADOW_VMCS))
		tst_brk(TCONF, "CPU does not support shadow VMCS");

	val = VMX_EXECCTL2_SHADOW_VMCS | (uint32_t)val;
	kvm_vmx_vmwrite(VMX_VMCS_VMEXEC_CTL2, val);
	kvm_vmx_vmwrite(VMX_VMCS_LINK_POINTER, (uintptr_t)shadow_vmcs);

	/* Configure shadow VMCS */
	vmcs_backup = tst_heap_alloc(sizeof(struct kvm_vmcs));
	memcpy(vmcs_backup, shadow_vmcs, sizeof(struct kvm_vmcs));
	kvm_vmx_vmptrld(shadow_vmcs);

	for (i = 0; i < SHADOW_DATA_LENGTH; i++)
		kvm_vmx_vmwrite(host_data[i].field_id, host_data[i].value);

	/* Flush shadow VMCS just in case */
	kvm_vmx_vmptrld(vcpu->vmcs);

	if (!memcmp(vmcs_backup, shadow_vmcs, sizeof(struct kvm_vmcs)))
		tst_res(TFAIL, "VMWRITE did not modify raw VMCS data");

	/* Run nested VM */
	memcpy(vmcs_backup, shadow_vmcs, sizeof(struct kvm_vmcs));
	kvm_vmx_vmrun(vcpu);
	val = kvm_vmx_vmread(VMX_VMCS_EXIT_REASON);

	if (val != VMX_EXIT_HLT) {
		tst_res(TFAIL, "Unexpected guest exit reason %llx", val);
		return;
	}

	if (vcpu->regs.rax == GUEST_READ_ERROR) {
		tst_res(TFAIL, "Guest failed to read shadow VMCS");
		return;
	}

	if (vcpu->regs.rax == GUEST_WRITE_ERROR) {
		tst_res(TFAIL, "Guest failed to write shadow VMCS");
		return;
	}

	if (!memcmp(vmcs_backup, shadow_vmcs, sizeof(struct kvm_vmcs)))
		tst_res(TFAIL, "Nested VMWRITE did not modify raw VMCS data");

	/* Check values read by the nested VM from shadow VMCS */
	for (i = 0, errors = 0; i < SHADOW_DATA_LENGTH; i++) {
		if (vmread_buffer[i] == host_data[i].value)
			continue;

		errors++;
		tst_res(TFAIL, "Shadow %s guest mismatch: %lx != %lx",
			host_data[i].name, vmread_buffer[i],
			host_data[i].value);
	}

	if (!errors)
		tst_res(TPASS, "Guest read correct values from shadow VMCS");

	/* Check values written by the nested VM to shadow VMCS */
	kvm_vmx_vmptrld(shadow_vmcs);

	for (i = 0, errors = 0; i < SHADOW_DATA_LENGTH; i++) {
		val = kvm_vmx_vmread(guest_data[i].field_id);

		if (val == guest_data[i].value)
			continue;

		errors++;
		tst_res(TFAIL, "Shadow %s parent mismatch: %llx != %lx",
			guest_data[i].name, val, guest_data[i].value);
	}

	if (!errors)
		tst_res(TPASS, "Parent read correct values from shadow VMCS");
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
