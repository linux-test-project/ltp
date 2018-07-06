/*
 * Copyright (C) 2011-2017  Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * KSM - NULL pointer dereference in ksm_do_scan() (CVE-2011-2183)
 *
 * This is a testcase from upstream commit:
 * 2b472611a32a72f4a118c069c2d62a1a3f087afd.
 *
 * an exiting task can race against ksmd::scan_get_next_rmap_item
 * (http://lkml.org/lkml/2011/6/1/742) easily triggering a NULL pointer
 * dereference in ksmd.
 * ksm_scan.mm_slot == &ksm_mm_head with only one registered mm
 *
 * CPU 1 (__ksm_exit)          CPU 2 (scan_get_next_rmap_item)
 *                             list_empty() is false
 * lock                        slot == &ksm_mm_head
 * list_del(slot->mm_list)
 * (list now empty)
 * unlock
 *                              lock
 *                              slot = list_entry(slot->mm_list.next)
 *                              (list is empty, so slot is still ksm_mm_head)
 *                              unlock
 *                              slot->mm == NULL ... Oops
 *
 * Close this race by revalidating that the new slot is not simply the list
 * head again.
 *
 * Test Prerequisites:
 *
 * *) ksm and ksmtuned daemons need to be disabled. Otherwise, it could
 *    distrub the testing as they also change some ksm tunables depends
 *    on current workloads.
 */

#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include "tst_test.h"
#include "mem.h"

#ifdef HAVE_MADV_MERGEABLE

static int ksm_run_orig = -1;
static void sighandler(int sig);

static void test_ksm(void)
{
	int status;
	long ps;
	pid_t pid;
	void *ptr;
	struct sigaction sa;

	memset (&sa, '\0', sizeof(sa));
	sa.sa_handler = sighandler;
	sa.sa_flags = 0;
	TEST(sigaction(SIGSEGV, &sa, NULL));
	if (TST_RET == -1)
		tst_brk(TBROK | TRERRNO,
				"SIGSEGV signal setup failed");

	ps = sysconf(_SC_PAGESIZE);

	pid = SAFE_FORK();
	if (pid == 0) {
		ptr = SAFE_MEMALIGN(ps, ps);
		if (madvise(ptr, ps, MADV_MERGEABLE) < 0)
			tst_brk(TBROK | TERRNO, "madvise");
		*(volatile char *)NULL = 0; /* SIGSEGV occurs as expected. */
	}
	SAFE_WAITPID(pid, &status, WUNTRACED | WCONTINUED);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_brk(TBROK, "invalid signal received: %d", status);

	tst_res(TPASS, "still alive.");
}

static void sighandler(int sig)
{
	_exit((sig == SIGSEGV) ? 0 : sig);
}

static void setup(void)
{
	if (access(PATH_KSM, F_OK) == -1)
		tst_brk(TCONF, "KSM configuration is not enabled");

	/* save original /sys/kernel/mm/ksm/run value */
	SAFE_FILE_SCANF(PATH_KSM "run", "%d", &ksm_run_orig);

	/* echo 1 > /sys/kernel/mm/ksm/run */
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
}

static void cleanup(void)
{
	/* restore /sys/kernel/mm/ksm/run value */
	if (ksm_run_orig > 0)
		FILE_PRINTF(PATH_KSM "run", "%d", ksm_run_orig);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_ksm,
	.min_kver = "2.6.32",
};

#else
	TST_TEST_TCONF("no MADV_MERGEABLE found.");
#endif
