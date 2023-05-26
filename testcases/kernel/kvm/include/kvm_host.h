/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * KVM host library for setting up and running virtual machine tests. Tests
 * can either use the default setup/run/host functions or use the advanced
 * API to create customized VMs.
 */

/*
 * Most basic usage:
 *
 * #include "kvm_test.h"
 *
 * #ifdef COMPILE_PAYLOAD
 *
 * void main(void)
 * {
 *	[VM guest code goes here]
 * }
 *
 * #else
 *
 * [optional VM host setup/run/cleanup code goes here]
 *
 * static struct tst_test test = {
 *	.test_all = tst_kvm_run,
 *	.setup = tst_kvm_setup,
 *	.cleanup = tst_kvm_cleanup,
 * };
 *
 * #endif
 */

#ifndef KVM_HOST_H_
#define KVM_HOST_H_

#include <inttypes.h>
#include <linux/kvm.h>
#include "kvm_common.h"

#define VM_KERNEL_BASEADDR 0x1000
#define VM_RESET_BASEADDR 0xfffffff0
#define VM_RESET_CODE_SIZE 8

#define MIN_FREE_RAM (10 * 1024 * 1024)
#define DEFAULT_RAM_SIZE (16 * 1024 * 1024)
#define MAX_KVM_MEMSLOTS 8

struct tst_kvm_instance {
	int vm_fd, vcpu_fd;
	struct kvm_run *vcpu_info;
	size_t vcpu_info_size;
	struct kvm_userspace_memory_region ram[MAX_KVM_MEMSLOTS];
	struct tst_kvm_result *result;
};

/* Test binary to be installed into the VM at VM_KERNEL_BASEADDR */
extern const char kvm_payload_start[], kvm_payload_end[];

/* CPU reset code to be installed into the VM at VM_RESET_BASEADDR */
extern const unsigned char tst_kvm_reset_code[VM_RESET_CODE_SIZE];

/* Default KVM test functions. */
void tst_kvm_setup(void);
void tst_kvm_run(void);
void tst_kvm_cleanup(void);

/*
 * Validate KVM guest test result (usually passed via result->result) and
 * fail with TBROK if the value cannot be safely passed to tst_res() or
 * tst_brk().
 */
void tst_kvm_validate_result(int value);

/*
 * Allocate memory slot for the VM. The returned pointer is page-aligned
 * so the actual requested base address is at ret[baseaddr % pagesize].
 *
 * The first argument is a VM file descriptor created by ioctl(KVM_CREATE_VM)
 *
 * The return value points to a guarded buffer and the user should not attempt
 * to free() it. Any extra space added at the beginning or end for page
 * alignment will be writable.
 */
void *tst_kvm_alloc_memory(struct tst_kvm_instance *inst, unsigned int slot,
	uint64_t baseaddr, size_t size, unsigned int flags);

/*
 * Translate VM virtual memory address to the corresponding physical address.
 * Returns 0 if the virtual address is unmapped or otherwise invalid.
 */
uint64_t tst_kvm_get_phys_address(const struct tst_kvm_instance *inst,
	uint64_t addr);

/*
 * Find the struct tst_kvm_instance memory slot ID for the give virtual
 * or physical VM memory address. Returns -1 if the address is not backed
 * by any memory buffer.
 */
int tst_kvm_find_phys_memslot(const struct tst_kvm_instance *inst,
	uint64_t paddr);
int tst_kvm_find_memslot(const struct tst_kvm_instance *inst, uint64_t addr);

/*
 * Convert VM virtual memory address to a directly usable pointer.
 */
void *tst_kvm_get_memptr(const struct tst_kvm_instance *inst, uint64_t addr);

/*
 * Find CPUIDs supported by KVM. x86_64 tests must set non-default CPUID,
 * otherwise bootstrap will fail to initialize 64bit mode.
 * Returns NULL if ioctl(KVM_GET_SUPPORTED_CPUID) is not supported.
 *
 * The argument is a file descriptor created by open("/dev/kvm")
 */
struct kvm_cpuid2 *tst_kvm_get_cpuid(int sysfd);

/*
 * Initialize the given KVM instance structure. Creates new KVM virtual machine
 * with 1 virtual CPU, allocates VM RAM (max. 4GB minus one page) and
 * shared result structure. KVM memory slots 0 and 1 will be set by this
 * function.
 */
void tst_kvm_create_instance(struct tst_kvm_instance *inst, size_t ram_size);

/*
 * Execute the given KVM instance and print results. If ioctl(KVM_RUN) is
 * expected to fail, pass the expected error code in exp_errno, otherwise
 * set it to zero. Returns last value returned by ioctl(KVM_RUN).
 */
int tst_kvm_run_instance(struct tst_kvm_instance *inst, int exp_errno);

/*
 * Close the given KVM instance.
 */
void tst_kvm_destroy_instance(struct tst_kvm_instance *inst);

/*
 * Wait for given VM to call tst_signal_host() or tst_wait_host(). Timeout
 * value is in milliseconds. Zero means no wait, negative value means wait
 * forever. Returns 0 if signal was received, KVM_TEXIT if the VM exited
 * without sending a signal, or -1 if timeout was reached.
 */
int tst_kvm_wait_guest(struct tst_kvm_instance *inst, int timeout_ms);

/*
 * Clear VM signal sent by tst_signal_host(). If the VM is waiting
 * in tst_wait_host(), this function will signal the VM to resume execution.
 */
void tst_kvm_clear_guest_signal(struct tst_kvm_instance *inst);

#endif /* KVM_HOST_H_ */
