// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * Check that KVM correctly intercepts the CLGI instruction in a nested
 * virtual machine even when the parent guest disables intercept.
 * If KVM does not override the disabled intercept, it'll allow the nested VM
 * to hold the physical CPU indefinitely and potentially perform a denial
 * of service attack against the host kernel. CPU lockup fixed in:
 *
 *  commit 91b7130cb6606d8c6b3b77e54426b3f3a83f48b1
 *  Author: Paolo Bonzini <pbonzini@redhat.com>
 *  Date:   Fri May 22 12:28:52 2020 -0400
 *
 *  KVM: SVM: preserve VGIF across VMCB switch
 */

#include "kvm_test.h"

#ifdef COMPILE_PAYLOAD
#if defined(__i386__) || defined(__x86_64__)

#include "kvm_x86_svm.h"

/* Disable global interrupts */
static int guest_clgi(void)
{
	int ret, *result = (int *)KVM_RESULT_BASEADDR;

	/*
	 * Make sure that result page is present in memory. CLGI may disable
	 * page fault handling on the current CPU. The actual value
	 * at that address is irrelevant.
	 */
	ret = *result;

	/* Disable global interrupts */
	asm ("clgi");

	/* Signal host to kill the VM and wait */
	tst_wait_host(NULL);
	return ret;
}

void main(void)
{
	struct kvm_svm_vcpu *vcpu;

	kvm_init_svm();
	vcpu = kvm_create_svm_vcpu(guest_clgi, 1);
	kvm_vmcb_set_intercept(vcpu->vmcb, SVM_INTERCEPT_CLGI, 0);
	kvm_svm_vmrun(vcpu);

	if (vcpu->vmcb->exitcode != SVM_EXIT_HLT)
		tst_brk(TBROK, "Nested VM exited unexpectedly");
}

#else /* defined(__i386__) || defined(__x86_64__) */
TST_TEST_TCONF("Test supported only on x86");
#endif /* defined(__i386__) || defined(__x86_64__) */

#else /* COMPILE_PAYLOAD */

#include <pthread.h>
#include "tst_safe_pthread.h"
#include "tst_safe_clocks.h"

static struct tst_kvm_instance test_vm = { .vm_fd = -1 };
static pthread_mutex_t mutex;
static int mutex_init;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{

}

static void *vm_thread(void *arg)
{
	SAFE_PTHREAD_MUTEX_LOCK(&mutex);
	tst_kvm_run_instance(&test_vm, EINTR);
	SAFE_PTHREAD_MUTEX_UNLOCK(&mutex);
	return arg;
}

static void setup(void)
{
	struct sigaction sa = { .sa_handler = sighandler };
	pthread_mutexattr_t attr;

	SAFE_PTHREAD_MUTEXATTR_INIT(&attr);
	SAFE_PTHREAD_MUTEXATTR_SETTYPE(&attr, PTHREAD_MUTEX_NORMAL);
	SAFE_PTHREAD_MUTEX_INIT(&mutex, &attr);
	mutex_init = 1;
	SAFE_PTHREAD_MUTEXATTR_DESTROY(&attr);
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);
}

static void run(void)
{
	struct timespec timeout;
	pthread_t tid;
	int ret;

	tst_kvm_create_instance(&test_vm, DEFAULT_RAM_SIZE);

	SAFE_PTHREAD_CREATE(&tid, NULL, vm_thread, NULL);
	ret = tst_kvm_wait_guest(&test_vm, 2000);

	if (ret == KVM_TEXIT) {
		SAFE_PTHREAD_JOIN(tid, NULL);
		tst_brk(TCONF, "Guest exited early");
	}

	if (ret)
		tst_brk(TBROK, "Wait for guest initialization timed out");

	SAFE_PTHREAD_KILL(tid, SIGUSR1);
	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 2;

	if (SAFE_PTHREAD_MUTEX_TIMEDLOCK(&mutex, &timeout)) {
		tst_kvm_clear_guest_signal(&test_vm);
		tst_res(TFAIL, "VM thread does not respond to signals");
	} else {
		SAFE_PTHREAD_MUTEX_UNLOCK(&mutex);
		tst_res(TPASS, "VM thread was interrupted by signal");
	}

	SAFE_PTHREAD_JOIN(tid, NULL);
	tst_kvm_destroy_instance(&test_vm);
	tst_free_all();
}

static void cleanup(void)
{
	/*
	 * If the mutex is locked, the VM is likely still running, cannot
	 * clean up anything
	 */
	if (!mutex_init || SAFE_PTHREAD_MUTEX_TRYLOCK(&mutex))
		return;

	if (!SAFE_PTHREAD_MUTEX_UNLOCK(&mutex))
		SAFE_PTHREAD_MUTEX_DESTROY(&mutex);

	tst_kvm_destroy_instance(&test_vm);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_drivers = (const char *const []) {
		"kvm",
		NULL
	},
	.min_cpus = 2,
	.supported_archs = (const char *const []) {
		"x86_64",
		"x86",
		NULL
	},
	.tags = (struct tst_tag[]){
		{"linux-git", "91b7130cb660"},
		{}
	}
};

#endif /* COMPILE_PAYLOAD */
