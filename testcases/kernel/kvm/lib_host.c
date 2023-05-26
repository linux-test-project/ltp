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
#include "tst_clocks.h"
#include "tst_timer.h"
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

uint64_t tst_kvm_get_phys_address(const struct tst_kvm_instance *inst,
	uint64_t addr)
{
	struct kvm_translation trans = { .linear_address = addr };

	TEST(ioctl(inst->vcpu_fd, KVM_TRANSLATE, &trans));

	/* ioctl(KVM_TRANSLATE) is not implemented for this arch */
	if (TST_RET == -1 && TST_ERR == EINVAL)
		return addr;

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "ioctl(KVM_TRANSLATE) failed");

	if (TST_RET) {
		tst_brk(TBROK | TTERRNO,
			"Invalid ioctl(KVM_TRANSLATE) return value");
	}

	return trans.valid ? trans.physical_address : 0;
}

int tst_kvm_find_phys_memslot(const struct tst_kvm_instance *inst,
	uint64_t paddr)
{
	int i;
	uint64_t base;

	for (i = 0; i < MAX_KVM_MEMSLOTS; i++) {
		if (!inst->ram[i].userspace_addr)
			continue;

		base = inst->ram[i].guest_phys_addr;

		if (paddr >= base && paddr - base < inst->ram[i].memory_size)
			return i;
	}

	return -1;
}

int tst_kvm_find_memslot(const struct tst_kvm_instance *inst, uint64_t addr)
{
	addr = tst_kvm_get_phys_address(inst, addr);

	if (!addr)
		return -1;

	return tst_kvm_find_phys_memslot(inst, addr);
}

void *tst_kvm_get_memptr(const struct tst_kvm_instance *inst, uint64_t addr)
{
	int slot;
	char *ret;

	addr = tst_kvm_get_phys_address(inst, addr);

	if (!addr)
		return NULL;

	slot = tst_kvm_find_phys_memslot(inst, addr);

	if (slot < 0)
		return NULL;

	ret = (char *)(uintptr_t)inst->ram[slot].userspace_addr;
	return ret + (addr - inst->ram[slot].guest_phys_addr);
}

void tst_kvm_print_result(const struct tst_kvm_instance *inst)
{
	int ttype;
	const struct tst_kvm_result *result = inst->result;
	const char *file;

	tst_kvm_validate_result(result->result);
	ttype = TTYPE_RESULT(result->result);
	file = tst_kvm_get_memptr(inst, result->file_addr);

	if (ttype == TBROK)
		tst_brk_(file, result->lineno, ttype, "%s", result->message);
	else
		tst_res_(file, result->lineno, ttype, "%s", result->message);
}

void *tst_kvm_alloc_memory(struct tst_kvm_instance *inst, unsigned int slot,
	uint64_t baseaddr, size_t size, unsigned int flags)
{
	size_t pagesize, offset;
	char *ret;
	struct kvm_userspace_memory_region memslot = {
		.slot = slot,
		.flags = flags
	};

	if (slot >= MAX_KVM_MEMSLOTS)
		tst_brk(TBROK, "Invalid KVM memory slot %u", slot);

	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);
	offset = baseaddr % pagesize;
	size = LTP_ALIGN(size + offset, pagesize);
	ret = tst_alloc(size);

	memslot.guest_phys_addr = baseaddr - offset;
	memslot.memory_size = size;
	memslot.userspace_addr = (uintptr_t)ret;
	SAFE_IOCTL(inst->vm_fd, KVM_SET_USER_MEMORY_REGION, &memslot);
	inst->ram[slot] = memslot;
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
	char *buf, *reset_ptr;
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
		ram_size = LTP_ALIGN(ram_size, 1024 * 1024);
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

	buf = tst_kvm_alloc_memory(inst, 0, 0, ram_size, 0);
	memcpy(buf + VM_KERNEL_BASEADDR, kvm_payload_start, payload_size);
	buf = tst_kvm_alloc_memory(inst, 1, KVM_RESULT_BASEADDR,
		KVM_RESULT_SIZE, 0);
	memset(buf, 0, KVM_RESULT_SIZE);

	reset_ptr = buf + (VM_RESET_BASEADDR % pagesize);
	memcpy(reset_ptr, tst_kvm_reset_code, sizeof(tst_kvm_reset_code));
	inst->result = (struct tst_kvm_result *)(buf +
		(KVM_RESULT_BASEADDR % pagesize));
	inst->result->result = KVM_TNONE;
	inst->result->message[0] = '\0';
}

int tst_kvm_run_instance(struct tst_kvm_instance *inst, int exp_errno)
{
	struct kvm_regs regs;
	int ret;

	while (1) {
		inst->result->result = KVM_TNONE;
		inst->result->message[0] = '\0';
		errno = 0;
		ret = ioctl(inst->vcpu_fd, KVM_RUN, 0);

		if (ret == -1) {
			if (errno == exp_errno)
				return ret;

			tst_brk(TBROK | TERRNO, "ioctl(KVM_RUN) failed");
		}

		if (ret < 0) {
			tst_brk(TBROK | TERRNO,
				"Invalid ioctl(KVM_RUN) return value %d", ret);
		}

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

	return ret;
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
	memset(inst->ram, 0, sizeof(inst->ram));
}

int tst_kvm_wait_guest(struct tst_kvm_instance *inst, int timeout_ms)
{
	volatile struct tst_kvm_result *result = inst->result;
	int32_t res;
	struct timespec start, now;

	if (timeout_ms >= 0)
		tst_clock_gettime(CLOCK_MONOTONIC, &start);

	while ((res = result->result) != KVM_TSYNC) {
		if (res == KVM_TEXIT)
			return res;

		if (timeout_ms >= 0) {
			tst_clock_gettime(CLOCK_MONOTONIC, &now);

			if (tst_timespec_diff_ms(now, start) >= timeout_ms)
				return -1;
		}

		usleep(1000);
	}

	return 0;
}

void tst_kvm_clear_guest_signal(struct tst_kvm_instance *inst)
{
	inst->result->result = KVM_TNONE;
}

void tst_kvm_setup(void)
{

}

void tst_kvm_run(void)
{
	tst_kvm_create_instance(&test_vm, DEFAULT_RAM_SIZE);
	tst_kvm_run_instance(&test_vm, 0);
	tst_kvm_destroy_instance(&test_vm);
	tst_free_all();
}

void tst_kvm_cleanup(void)
{
	tst_kvm_destroy_instance(&test_vm);
}
