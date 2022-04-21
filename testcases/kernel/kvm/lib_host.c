// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * KVM host library for setting up and running virtual machine tests.
 */

#include <stdlib.h>
#include <errno.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "kvm_host.h"

static struct tst_kvm_instance test_vm = { .vm_fd = -1 };

const unsigned char tst_kvm_reset_code[VM_RESET_CODE_SIZE] = {
	0xea, 0x00, 0x10, 0x00, 0x00	/* JMP 0x1000 */
};

void tst_kvm_validate_result(int value)
{
	int ttype, valid_result[] = {TPASS, TFAIL, TBROK, TWARN, TINFO, TCONF};
	size_t i;

	if (value == KVM_TNONE)
		tst_brk(TBROK, "KVM test did not return any result");

	ttype = TTYPE_RESULT(value);

	for (i = 0; i < ARRAY_SIZE(valid_result); i++) {
		if (ttype == valid_result[i])
			return;
	}

	tst_brk(TBROK, "KVM test returned invalid result value %d", value);
}

void tst_kvm_print_result(const struct tst_kvm_instance *inst)
{
	int ttype;
	const struct tst_kvm_result *result = inst->result;
	const char *file = inst->ram;

	tst_kvm_validate_result(result->result);
	ttype = TTYPE_RESULT(result->result);
	file += result->file_addr;

	if (ttype == TBROK)
		tst_brk_(file, result->lineno, ttype, "%s", result->message);
	else
		tst_res_(file, result->lineno, ttype, "%s", result->message);
}

void *tst_kvm_alloc_memory(int vm, unsigned int slot, uint64_t baseaddr,
	size_t size, unsigned int flags)
{
	size_t pagesize;
	void *ret;
	struct kvm_userspace_memory_region memslot = {
		.slot = slot,
		.flags = flags
	};

	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);
	size += (baseaddr % pagesize) + pagesize - 1;
	baseaddr -= baseaddr % pagesize;
	size -= size % pagesize;
	ret = tst_alloc(size);

	memslot.guest_phys_addr = baseaddr;
	memslot.memory_size = size;
	memslot.userspace_addr = (uintptr_t)ret;
	SAFE_IOCTL(vm, KVM_SET_USER_MEMORY_REGION, &memslot);
	return ret;
}

struct kvm_cpuid2 *tst_kvm_get_cpuid(int sysfd)
{
	unsigned int count;
	int result;
	struct kvm_cpuid2 *ret;

	if (!SAFE_IOCTL(sysfd, KVM_CHECK_EXTENSION, KVM_CAP_EXT_CPUID))
		return NULL;

	for (count = 8; count < 1 << 30; count *= 2) {
		ret = SAFE_MALLOC(sizeof(struct kvm_cpuid2) +
			count * sizeof(struct kvm_cpuid_entry2));
		ret->nent = count;
		errno = 0;
		result = ioctl(sysfd, KVM_GET_SUPPORTED_CPUID, ret);

		if (!result)
			return ret;

		free(ret);

		if (errno != E2BIG)
			break;
	}

	tst_brk(TBROK | TERRNO, "ioctl(KVM_GET_SUPPORTED_CPUID) failed");
	return NULL;
}

void tst_kvm_create_instance(struct tst_kvm_instance *inst, size_t ram_size)
{
	int sys_fd;
	size_t pagesize, result_pageaddr = KVM_RESULT_BASEADDR;
	char *vm_result, *reset_ptr;
	struct kvm_cpuid2 *cpuid_data;
	const size_t payload_size = kvm_payload_end - kvm_payload_start;

	memset(inst, 0, sizeof(struct tst_kvm_instance));
	inst->vm_fd = -1;
	inst->vcpu_fd = -1;
	inst->vcpu_info = MAP_FAILED;

	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);
	result_pageaddr -= result_pageaddr % pagesize;

	if (payload_size + MIN_FREE_RAM > ram_size - VM_KERNEL_BASEADDR) {
		ram_size = payload_size + MIN_FREE_RAM + VM_KERNEL_BASEADDR;
		ram_size += 1024 * 1024 - 1;
		ram_size -= ram_size % (1024 * 1024);
		tst_res(TWARN, "RAM size increased to %zu bytes", ram_size);
	}

	if (ram_size > result_pageaddr) {
		ram_size = result_pageaddr;
		tst_res(TWARN, "RAM size truncated to %zu bytes", ram_size);
	}

	sys_fd = SAFE_OPEN("/dev/kvm", O_RDWR);
	inst->vcpu_info_size = SAFE_IOCTL(sys_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
	inst->vm_fd = SAFE_IOCTL(sys_fd, KVM_CREATE_VM, 0);
	cpuid_data = tst_kvm_get_cpuid(sys_fd);
	SAFE_CLOSE(sys_fd);

	inst->vcpu_fd = SAFE_IOCTL(inst->vm_fd, KVM_CREATE_VCPU, 0);

	if (cpuid_data) {
		SAFE_IOCTL(inst->vcpu_fd, KVM_SET_CPUID2, cpuid_data);
		free(cpuid_data);
	}

	inst->vcpu_info = SAFE_MMAP(NULL, inst->vcpu_info_size,
		PROT_READ | PROT_WRITE, MAP_SHARED, inst->vcpu_fd, 0);

	inst->ram = tst_kvm_alloc_memory(inst->vm_fd, 0, 0, ram_size, 0);
	vm_result = tst_kvm_alloc_memory(inst->vm_fd, 1, KVM_RESULT_BASEADDR,
		KVM_RESULT_SIZE, 0);
	memset(vm_result, 0, KVM_RESULT_SIZE);
	memcpy(inst->ram + VM_KERNEL_BASEADDR, kvm_payload_start, payload_size);

	reset_ptr = vm_result + (VM_RESET_BASEADDR % pagesize);
	memcpy(reset_ptr, tst_kvm_reset_code, sizeof(tst_kvm_reset_code));
	inst->result = (struct tst_kvm_result *)(vm_result +
		(KVM_RESULT_BASEADDR % pagesize));
	inst->result->result = KVM_TNONE;
	inst->result->message[0] = '\0';
}

void tst_kvm_run_instance(struct tst_kvm_instance *inst)
{
	struct kvm_regs regs;

	while (1) {
		inst->result->result = KVM_TNONE;
		inst->result->message[0] = '\0';
		SAFE_IOCTL(inst->vcpu_fd, KVM_RUN, 0);

		if (inst->vcpu_info->exit_reason != KVM_EXIT_HLT) {
			SAFE_IOCTL(inst->vcpu_fd, KVM_GET_REGS, &regs);
			tst_brk(TBROK,
				"Unexpected VM exit, RIP=0x%llx, reason=%u",
				regs.rip, inst->vcpu_info->exit_reason);
		}

		if (inst->result->result == KVM_TEXIT)
			break;

		tst_kvm_print_result(inst);
	}
}

void tst_kvm_destroy_instance(struct tst_kvm_instance *inst)
{
	if (inst->vm_fd < 0)
		return;

	if (inst->vcpu_info != MAP_FAILED)
		SAFE_MUNMAP(inst->vcpu_info, inst->vcpu_info_size);

	if (inst->vcpu_fd >= 0)
		SAFE_CLOSE(inst->vcpu_fd);

	SAFE_CLOSE(inst->vm_fd);
}

void tst_kvm_setup(void)
{
	tst_kvm_create_instance(&test_vm, DEFAULT_RAM_SIZE);
}

void tst_kvm_run(void)
{
	tst_kvm_run_instance(&test_vm);
}

void tst_kvm_cleanup(void)
{
	tst_kvm_destroy_instance(&test_vm);
}
