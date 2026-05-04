// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation
 * Author: David Gibson & Adam Litke
 */

/*
 * This test is designed to detect a kernel allocation race introduced
 * with hugepage demand-faulting.  The problem is that no lock is held
 * between allocating a hugepage and instantiating it in the
 * pagetables or page cache index.  In between the two, the (huge)
 * page is cleared, so there's substantial time.  Thus two processes
 * can race instantiating the (same) last available hugepage - one
 * will fail on the allocation, and thus cause an OOM fault even
 * though the page it actually wants is being instantiated by the
 * other racing process.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include "tst_safe_pthread.h"
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"

static char *str_op;
static int child1, child2, race_type, fd_sync;

struct racer_info {
	void *p;
	int cpu;
	int status;
};

static int one_racer(void *p, int cpu)
{
	volatile int *pi = p;
	cpu_set_t *cpuset;
	size_t mask_size;
	int err;

	cpuset = CPU_ALLOC(cpu + 1);
	if (!cpuset)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC() failed");

	mask_size = CPU_ALLOC_SIZE(cpu + 1);

	/* Split onto different CPUs to encourage the race */
	CPU_ZERO_S(mask_size, cpuset);
	CPU_SET_S(cpu, mask_size, cpuset);

	err = sched_setaffinity(getpid(), mask_size, cpuset);
	if (err == -1)
		tst_brk(TBROK | TERRNO, "sched_setaffinity() failed");

	/* Wait for parent to signal both racers to start */
	TST_CHECKPOINT_WAIT(0);

	/* Set the shared value */
	*pi = 1;

	CPU_FREE(cpuset);
	return 0;
}

static void proc_racer(void *p, int cpu)
{
	exit(one_racer(p, cpu));
}

static void *thread_racer(void *info)
{
	struct racer_info *ri = info;

	ri->status = one_racer(ri->p, ri->cpu);
	return ri;
}

static void check_online_cpus(int online_cpus[], int nr_cpus_needed)
{
	cpu_set_t cpuset;
	int total_cpus, cpu_idx;

	CPU_ZERO(&cpuset);

	for (int i = 0; i < CPU_SETSIZE; i++)
		CPU_SET(i, &cpuset);

	if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1)
		tst_brk(TBROK | TERRNO, "sched_setaffinity() reset failed");

	total_cpus = get_nprocs_conf();

	if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset) == -1)
		tst_brk(TBROK | TERRNO, "sched_getaffinity() failed");

	tst_res(TINFO, "Online CPUs needed: %d, available: %d",
		nr_cpus_needed, CPU_COUNT(&cpuset));

	if (CPU_COUNT(&cpuset) < nr_cpus_needed)
		tst_brk(TCONF, "At least %d online CPUs are required", nr_cpus_needed);

	cpu_idx = 0;
	for (int i = 0; i < total_cpus && cpu_idx < nr_cpus_needed; i++) {
		if (CPU_ISSET(i, &cpuset))
			online_cpus[cpu_idx++] = i;
	}

	if (cpu_idx < nr_cpus_needed)
		tst_brk(TBROK, "Unable to find enough online CPUs");
}

static void run_race(int race_type)
{
	int fd = -1;
	void *p = MAP_FAILED;
	void *tret1, *tret2;
	int status1 = 0, status2 = 0;
	int online_cpus[2];
	long hpage_size;
	pthread_t thread1, thread2;

	check_online_cpus(online_cpus, 2);

	hpage_size = tst_get_hugepage_size();

	/* Get a new file for the final page */
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	tst_res(TINFO, "Mapping final page..");

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, race_type, fd, 0);

	if (race_type == MAP_SHARED) {
		child1 = SAFE_FORK();
		if (child1 == 0)
			proc_racer(p, online_cpus[0]);

		child2 = SAFE_FORK();
		if (child2 == 0)
			proc_racer(p, online_cpus[1]);

		/* Wake both children to start the race simultaneously */
		TST_CHECKPOINT_WAKE2(0, 2);

		SAFE_WAITPID(child1, &status1, 0);
		tst_res(TINFO, "Child 1 status: %x", status1);

		SAFE_WAITPID(child2, &status2, 0);
		tst_res(TINFO, "Child 2 status: %x", status2);

		if (WIFSIGNALED(status1))
			tst_res(TFAIL, "Child 1 killed by signal %s",
				strsignal(WTERMSIG(status1)));
		if (WIFSIGNALED(status2))
			tst_res(TFAIL, "Child 2 killed by signal %s",
				strsignal(WTERMSIG(status2)));
	} else {
		struct racer_info ri1 = {
			.p = p,
			.cpu = online_cpus[0],
			.status = -1,
		};
		struct racer_info ri2 = {
			.p = p,
			.cpu = online_cpus[1],
			.status = -1,
		};

		SAFE_PTHREAD_CREATE(&thread1, NULL, thread_racer, &ri1);
		SAFE_PTHREAD_CREATE(&thread2, NULL, thread_racer, &ri2);

		/* Wake both threads to start the race simultaneously */
		TST_CHECKPOINT_WAKE2(0, 2);

		SAFE_PTHREAD_JOIN(thread1, &tret1);
		if (tret1 != &ri1)
			tst_res(TFAIL, "Thread 1 returned %p not %p, killed?",
				tret1, &ri1);

		SAFE_PTHREAD_JOIN(thread2, &tret2);
		if (tret2 != &ri2)
			tst_res(TFAIL, "Thread 2 returned %p not %p, killed?",
				tret2, &ri2);

		status1 = ri1.status;
		status2 = ri2.status;
	}

	if (status1 != 0)
		tst_res(TFAIL, "Racer 1 terminated with code %d", status1);

	if (status2 != 0)
		tst_res(TFAIL, "Racer 2 terminated with code %d", status2);

	if (status1 == 0 && status2 == 0)
		tst_res(TPASS, "Test completed successfully");

	if (fd >= 0)
		SAFE_CLOSE(fd);

	if (p != MAP_FAILED)
		SAFE_MUNMAP(p, hpage_size);
}

static void run_test(void)
{
	unsigned long totpages;
	long hpage_size;
	void *p_sync = MAP_FAILED;

	totpages = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	hpage_size = tst_get_hugepage_size();

	tst_res(TINFO, "Instantiating..");

	fd_sync = tst_creat_unlinked(MNTPOINT, 0, 0600);

	tst_res(TINFO, "Mapping %ld/%ld pages..", totpages - 1, totpages);
	p_sync = SAFE_MMAP(NULL, (totpages - 1) * hpage_size, PROT_READ|PROT_WRITE,
			   MAP_SHARED, fd_sync, 0);

	run_race(race_type);

	if (fd_sync >= 0)
		SAFE_CLOSE(fd_sync);

	if (p_sync != MAP_FAILED)
		SAFE_MUNMAP(p_sync, (totpages - 1) * hpage_size);
}

static void setup(void)
{
	if (str_op) {
		if (strcmp(str_op, "shared") == 0)
			race_type = MAP_SHARED;
		else if (strcmp(str_op, "private") == 0)
			race_type = MAP_PRIVATE;
		else
			tst_brk(TBROK, "Invalid parameter: use -m <private|shared>");
	} else {
		/* Default to shared if no option is passed */
		race_type = MAP_SHARED;
	}
}

static void cleanup(void)
{
	if (fd_sync >= 0)
		SAFE_CLOSE(fd_sync);

	if (child1 > 0) {
		if (kill(child1, 0) == 0)
			SAFE_KILL(child1, SIGKILL);
	}

	if (child2 > 0) {
		if (kill(child2, 0) == 0)
			SAFE_KILL(child2, SIGKILL);
	}
}

static struct tst_test test = {
	.options = (struct tst_option[]) {
		{"m:", &str_op, "Type of mmap() mapping <private|shared>"},
		{NULL, NULL, NULL}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
	.forks_child = 1,
	.needs_checkpoints = 1,
	.min_cpus = 2
};
