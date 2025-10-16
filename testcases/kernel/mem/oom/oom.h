// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2011-2021
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2024
 */
#ifndef OOM_H_
#define OOM_H_

#include <pthread.h>
#include "config.h"
#include "numa_helper.h"

#define PATH_KSM        "/sys/kernel/mm/ksm/"

#define LENGTH			(3UL<<30)
#define NORMAL			1
#define MLOCK			2
#define KSM			3

#ifdef HAVE_NUMA_V2
static inline void set_global_mempolicy(int mempolicy)
{
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	int num_nodes, *nodes;
	int ret;

	if (mempolicy) {
		ret = get_allowed_nodes_arr(NH_MEMS|NH_CPUS, &num_nodes, &nodes);
		if (ret != 0)
			tst_brk(TBROK|TERRNO, "get_allowed_nodes_arr");
		if (num_nodes < 2) {
			tst_res(TINFO, "mempolicy need NUMA system support");
			free(nodes);
			return;
		}
		switch(mempolicy) {
		case MPOL_BIND:
			/* bind the second node */
			set_node(nmask, nodes[1]);
			break;
		case MPOL_INTERLEAVE:
		case MPOL_PREFERRED:
			if (num_nodes == 2) {
				tst_res(TINFO, "The mempolicy need "
					 "more than 2 numa nodes");
				free(nodes);
				return;
			} else {
				/* Using the 2nd,3rd node */
				set_node(nmask, nodes[1]);
				set_node(nmask, nodes[2]);
			}
			break;
		default:
			tst_brk(TBROK|TERRNO, "Bad mempolicy mode");
		}
		if (set_mempolicy(mempolicy, nmask, MAXNODES) == -1)
			tst_brk(TBROK|TERRNO, "set_mempolicy");
	}
}
#else
static void set_global_mempolicy(int mempolicy LTP_ATTRIBUTE_UNUSED) { }
#endif

static int alloc_mem(size_t length, int testcase)
{
	char *s;
	size_t i;
	long pagesz = getpagesize();
	int loop = 10;

	tst_res(TINFO, "thread (%lx), allocating %zu bytes.",
		(unsigned long) pthread_self(), length);

	s = mmap(NULL, length, PROT_READ | PROT_WRITE,
		 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (s == MAP_FAILED)
		return errno;

	if (testcase == MLOCK) {
		while (mlock(s, length) == -1 && loop > 0) {
			if (EAGAIN != errno)
				return errno;
			usleep(300000);
			loop--;
		}
	}

#ifdef HAVE_DECL_MADV_MERGEABLE
	if (testcase == KSM && madvise(s, length, MADV_MERGEABLE) == -1)
		return errno;
#endif
	for (i = 0; i < length; i += pagesz)
		s[i] = '\a';

	return 0;
}

static void *child_alloc_thread(void *args)
{
	int ret = 0;

	/* keep allocating until there's an error */
	while (!ret)
		ret = alloc_mem(LENGTH, (long)args);
	exit(ret);
}

static void child_alloc(int testcase, int lite, int threads)
{
	int i;
	pthread_t *th;

	if (lite) {
		int ret = alloc_mem((size_t)TESTMEM * 2 + TST_MB, testcase);
		exit(ret);
	}

	th = malloc(sizeof(pthread_t) * threads);
	if (!th) {
		tst_res(TINFO | TERRNO, "malloc");
		goto out;
	}

	for (i = 0; i < threads; i++) {
		TEST(pthread_create(&th[i], NULL, child_alloc_thread,
			(void *)((long)testcase)));
		if (TST_RET) {
			tst_res(TINFO | TRERRNO, "pthread_create");
			/*
			 * Keep going if thread other than first fails to
			 * spawn due to lack of resources.
			 */
			if (i == 0 || TST_RET != EAGAIN)
				goto out;
		}
	}

	/* wait for one of threads to exit whole process */
	while (1)
		sleep(1);
out:
	exit(1);
}

/*
 * oom - allocates memory according to specified testcase and checks
 *       desired outcome (e.g. child killed, operation failed with ENOMEM)
 * @testcase: selects how child allocates memory
 *            valid choices are: NORMAL, MLOCK and KSM
 * @lite: if non-zero, child makes only single TESTMEM+TST_MB allocation
 *        if zero, child keeps allocating memory until it gets killed
 *        or some operation fails
 * @retcode: expected return code of child process
 *           if matches child ret code, this function reports PASS,
 *           otherwise it reports FAIL
 * @allow_sigkill: if zero and child is killed, this function reports FAIL
 *                 if non-zero, then if child is killed by SIGKILL
 *                 it is considered as PASS
 */
static inline void oom(int testcase, int lite, int retcode, int allow_sigkill)
{
	pid_t pid;
	int status, threads;

	tst_enable_oom_protection(0);

	switch (pid = SAFE_FORK()) {
	case 0:
		tst_disable_oom_protection(0);
		threads = MAX(1, tst_ncpus() - 1);
		child_alloc(testcase, lite, threads);
	default:
		break;
	}

	tst_res(TINFO, "expected victim is %d.", pid);
	SAFE_WAITPID(-1, &status, 0);

	if (WIFSIGNALED(status)) {
		if (allow_sigkill && WTERMSIG(status) == SIGKILL) {
			tst_res(TPASS, "victim signalled: (%d) %s",
				SIGKILL,
				tst_strsig(SIGKILL));
		} else {
			tst_res(TFAIL, "victim signalled: (%d) %s",
				WTERMSIG(status),
				tst_strsig(WTERMSIG(status)));
		}
	} else if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == retcode) {
			tst_res(TPASS, "victim retcode: (%d) %s",
				retcode, strerror(retcode));
		} else {
			tst_res(TFAIL, "victim unexpectedly ended with "
				"retcode: %d, expected: %d",
				WEXITSTATUS(status), retcode);
		}
	} else {
		tst_res(TFAIL, "victim unexpectedly ended");
	}
}

static inline void testoom(int mempolicy, int lite, int retcode, int allow_sigkill)
{
	int ksm_run_orig;

	set_global_mempolicy(mempolicy);

	tst_res(TINFO, "start normal OOM testing.");
	oom(NORMAL, lite, retcode, allow_sigkill);

	tst_res(TINFO, "start OOM testing for mlocked pages.");
	oom(MLOCK, lite, retcode, allow_sigkill);

	/*
	 * Skip oom(KSM) if lite == 1, since limit_in_bytes may vary from
	 * run to run, which isn't reliable for oom03 cgroup test.
	 */
	if (access(PATH_KSM, F_OK) == -1 || lite == 1) {
		tst_res(TINFO, "KSM is not configed or lite == 1, "
			 "skip OOM test for KSM pags");
	} else {
		tst_res(TINFO, "start OOM testing for KSM pages.");
		SAFE_FILE_SCANF(PATH_KSM "run", "%d", &ksm_run_orig);
		SAFE_FILE_PRINTF(PATH_KSM "run", "1");
		oom(KSM, lite, retcode, allow_sigkill);
		SAFE_FILE_PRINTF(PATH_KSM "run", "%d", ksm_run_orig);
	}
}

#endif /* OOM_H_ */
