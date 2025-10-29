// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Red Hat Inc. All Rights Reserved.
 * Author: Chunfu Wen <chwen@redhat.com>
 */

/*\
 * Test that in a thread started by clone() that runs in the same address
 * space (CLONE_VM) but with a different TLS (CLONE_SETTLS) writtes to a
 * thread local variables are not propagated back from the cloned thread.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "clone_platform.h"
#include "lapi/syscalls.h"
#include "lapi/tls.h"

#define TLS_EXP 100

#ifndef ARCH_SET_FS
#define ARCH_SET_FS 0x1002
#endif

void *tls_ptr;
struct user_desc *tls_desc;

static __thread int tls_var;

static char *child_stack;
static volatile int child_done;

static int flags = CLONE_THREAD |  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SETTLS;

static int touch_tls_in_child(void *arg LTP_ATTRIBUTE_UNUSED)
{
#if defined(__x86_64__)
	if (syscall(SYS_arch_prctl, ARCH_SET_FS, tls_ptr) == -1)
		exit(EXIT_FAILURE);
#endif
	tls_var = TLS_EXP + 1;
	tst_res(TINFO, "Child (PID: %d, TID: %d): TLS value set to: %d", getpid(), (pid_t)syscall(SYS_gettid), tls_var);

	TST_CHECKPOINT_WAKE(0);
	free_tls();
	tst_syscall(__NR_exit, 0);
	return 0;
}

static void verify_tls(void)
{
	tls_var = TLS_EXP;

	TEST(ltp_clone7(flags, touch_tls_in_child, NULL, CHILD_STACK_SIZE, child_stack, NULL, tls_ptr, NULL));

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "clone() failed");

	TST_CHECKPOINT_WAIT(0);

	if (tls_var == TLS_EXP) {
		tst_res(TPASS,
			"Parent (PID: %d, TID: %d): TLS value correct: %d",
			getpid(), (pid_t)syscall(SYS_gettid), tls_var);
	} else {
		tst_res(TFAIL,
			"Parent (PID: %d, TID: %d): TLS value mismatch: got %d, expected %d",
			getpid(), (pid_t)syscall(SYS_gettid), tls_var, TLS_EXP);
	}
}

static void setup(void)
{
	child_stack = SAFE_MALLOC(CHILD_STACK_SIZE);
	init_tls();
}

static void cleanup(void)
{
	free(child_stack);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_checkpoints = 1,
	.test_all = verify_tls,
};
