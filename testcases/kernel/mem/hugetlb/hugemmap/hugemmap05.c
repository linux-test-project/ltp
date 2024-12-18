// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
 *
 * hugetlbfs allows to overcommit hugepages and there are tunables in
 * sysfs and procfs. The test here want to ensure it is possible to
 * overcommit by either mmap or shared memory. Also ensure those
 * reservation can be read/write, and several statistics work correctly.
 *
 * First, it resets nr_hugepages and nr_overcommit_hugepages. Then, set
 * both to a specify value - N, and allocate N + %50 x N hugepages.
 * Finally, it reads and writes every page. There are command options to
 * choose either to manage hugepages from sysfs or procfs, and reserve
 * them by mmap or shmget.
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "hugetlb.h"
#include "tst_safe_sysv_ipc.h"
#include "tst_test.h"

#define PROTECTION		(PROT_READ | PROT_WRITE)
#define PATH_MEMINFO		"/proc/meminfo"

static char path_sys_sz[BUFSIZ];
static char path_sys_sz_over[BUFSIZ];
static char path_sys_sz_free[BUFSIZ];
static char path_sys_sz_resv[BUFSIZ];
static char path_sys_sz_surp[BUFSIZ];
static char path_sys_sz_huge[BUFSIZ];

#define PATH_PROC_VM		"/proc/sys/vm/"
#define PATH_PROC_OVER		PATH_PROC_VM "nr_overcommit_hugepages"
#define PATH_PROC_HUGE		PATH_PROC_VM "nr_hugepages"

/* Only ia64 requires this */
#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define FLAGS (MAP_SHARED | MAP_FIXED)
#define SHMAT_FLAGS (SHM_RND)
#else
#define ADDR (void *)(0x0UL)
#define FLAGS (MAP_SHARED)
#define SHMAT_FLAGS (0)
#endif

#ifndef SHM_HUGETLB
#define SHM_HUGETLB 04000
#endif

#define NR_HPAGES 2
#define MOUNT_DIR "hugemmap05"
#define TEST_FILE MOUNT_DIR "/file"

static unsigned long long shmmax;
static char *path, *pathover;
static int key = -1, shmid = -1, fd = -1;
static int mounted, restore_shmmax, restore_overcomm_hgpgs;
static long hugepagesize, nr_overcommit_hugepages;
static long size = NR_HPAGES, length = (NR_HPAGES + NR_HPAGES/2) * 2;

char *opt_sysfs;
char *opt_alloc;
char *opt_shmid;

static void check_wr_bytes(void *addr);
static int checkproc(long act_val, char *string, long exp_val);
static int checksys(char *path, char *pattern, long exp_val);
static void init_sys_sz_paths(void);

static void test_overcommit(void)
{
	void *addr = NULL, *shmaddr = NULL;

	if (opt_shmid) {
		shmid = SAFE_SHMGET(key, (length / 2 * hugepagesize),
				 SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W);
	} else {
		fd = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR, 0755);
		addr = SAFE_MMAP(ADDR, (length / 2 * hugepagesize),
				 PROTECTION, FLAGS, fd, 0);
	}

	if (opt_sysfs) {
		tst_res(TINFO, "check sysfs before allocation.");
		if (checksys(path_sys_sz_huge, "HugePages_Total", length / 2))
			return;
		if (checksys(path_sys_sz_free, "HugePages_Free", length / 2))
			return;
		if (checksys(path_sys_sz_surp, "HugePages_Surp",
			     length / 2 - size))
			return;
		if (checksys(path_sys_sz_resv, "HugePages_Rsvd", length / 2))
			return;
	} else {
		tst_res(TINFO, "check /proc/meminfo before allocation.");
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Total:"),
			      "HugePages_Total", length / 2))
			return;
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Free:"),
			      "HugePages_Free", length / 2))
			return;
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Surp:"),
			      "HugePages_Surp", length / 2 - size))
			return;
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Rsvd:"),
			      "HugePages_Rsvd", length / 2))
			return;
	}

	if (opt_shmid) {
		tst_res(TINFO, "shmid: 0x%x", shmid);
		shmaddr = SAFE_SHMAT(shmid, ADDR, SHMAT_FLAGS);
		check_wr_bytes(shmaddr);
	} else {
		check_wr_bytes(addr);
	}

	if (opt_sysfs) {
		tst_res(TINFO, "check sysfs.");
		if (checksys(path_sys_sz_huge, "HugePages_Total", length / 2))
			return;
		if (checksys(path_sys_sz_free, "HugePages_Free", 0))
			return;
		if (checksys(path_sys_sz_surp, "HugePages_Surp",
			     length / 2 - size))
			return;
		if (checksys(path_sys_sz_resv, "HugePages_Rsvd", 0))
			return;
	} else {
		tst_res(TINFO, "check /proc/meminfo.");
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Total:"),
			      "HugePages_Total", length / 2))
			return;
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Free:"),
			      "HugePages_Free", 0))
			return;
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Surp:"),
			      "HugePages_Surp", length / 2 - size))
			return;
		if (checkproc(SAFE_READ_MEMINFO("HugePages_Rsvd:"),
			      "HugePages_Rsvd", 0))
			return;
	}

	if (opt_shmid) {
		SAFE_SHMDT(shmaddr);
		SAFE_SHMCTL(shmid, IPC_RMID, NULL);
	} else {
		SAFE_MUNMAP(addr, (length / 2 * hugepagesize));
		SAFE_CLOSE(fd);
		SAFE_UNLINK(TEST_FILE);
	}

	tst_res(TPASS, "hugepages overcommit test pass");
}

static void cleanup(void)
{
	if (opt_shmid && shmid != -1)
		SAFE_SHMCTL(shmid, IPC_RMID, NULL);

	if (!opt_shmid && fd != -1) {
		SAFE_CLOSE(fd);
		SAFE_UNLINK(TEST_FILE);
	}

	if (mounted)
		tst_umount(MOUNT_DIR);

	if (restore_shmmax)
		SAFE_FILE_PRINTF(PATH_SHMMAX, "%llu", shmmax);

	if (restore_overcomm_hgpgs) {
		tst_res(TINFO, "restore nr_overcommit_hugepages to %ld.",
			nr_overcommit_hugepages);
		SAFE_FILE_PRINTF(pathover, "%ld", nr_overcommit_hugepages);
	}
}

static void setup(void)
{
	unsigned long hpages;

	hugepagesize = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
	init_sys_sz_paths();

	if (opt_sysfs) {
		path = path_sys_sz_huge;
		pathover = path_sys_sz_over;
	} else {
		path = PATH_PROC_HUGE;
		pathover = PATH_PROC_OVER;
	}

	if (opt_alloc) {
		size = atoi(opt_alloc);
		length = (size + size * 0.5) * 2;
	}

	if (opt_shmid) {
		SAFE_FILE_SCANF(PATH_SHMMAX, "%llu", &shmmax);
		if (shmmax < (unsigned long long)(length / 2 * hugepagesize)) {
			restore_shmmax = 1;
			SAFE_FILE_PRINTF(PATH_SHMMAX, "%ld",
					(length / 2 * hugepagesize));
		}
	}

	/* Reset. */
	SAFE_FILE_PRINTF(path, "%ld", size);
	SAFE_FILE_SCANF(path, "%lu", &hpages);
	if (hpages != size)
		tst_brk(TCONF, "Not enough hugepages for testing!");

	if (access(pathover, F_OK)) {
		tst_brk(TCONF, "file %s does not exist in the system",
			pathover);
	}

	SAFE_FILE_SCANF(pathover, "%ld", &nr_overcommit_hugepages);
	tst_res(TINFO, "original nr_overcommit_hugepages is %ld",
		nr_overcommit_hugepages);

	/* Reset. */
	SAFE_FILE_PRINTF(pathover, "%ld", size);
	restore_overcomm_hgpgs = 1;

	SAFE_MKDIR(MOUNT_DIR, 0700);
	SAFE_MOUNT(NULL, MOUNT_DIR, "hugetlbfs", 0, NULL);
	mounted = 1;

	if (opt_shmid) {
		/* Use /proc/meminfo to generate an IPC key. */
		key = ftok(PATH_MEMINFO, strlen(PATH_MEMINFO));
		if (key == -1)
			tst_brk(TBROK | TERRNO, "ftok");
	}
}

static void check_wr_bytes(void *addr)
{
	long i;

	memset((char *)addr, '\a', (length / 2 * hugepagesize));

	tst_res(TINFO, "First hex is %x", *((unsigned int *)addr));
	for (i = 0; i < (length / 2 * hugepagesize); i++) {
		if (((char *)addr)[i] != '\a') {
			tst_res(TFAIL, "mismatch at %ld", i);
			break;
		}
	}
}

static int checksys(char *path, char *string, long exp_val)
{
	long act_val;

	SAFE_FILE_SCANF(path, "%ld", &act_val);
	tst_res(TINFO, "%s is %ld.", string, act_val);
	if (act_val != exp_val) {
		tst_res(TFAIL, "%s is not %ld but %ld.", string, exp_val,
			act_val);
		return 1;
	}
	return 0;
}

static int checkproc(long act_val, char *pattern, long exp_val)
{
	tst_res(TINFO, "%s is %ld.", pattern, act_val);
	if (act_val != exp_val) {
		tst_res(TFAIL, "%s is not %ld but %ld.",
			pattern, exp_val, act_val);
		return 1;
	}
	return 0;
}

static void init_sys_sz_paths(void)
{
	sprintf(path_sys_sz, "/sys/kernel/mm/hugepages/hugepages-%ldkB",
		hugepagesize / 1024);
	sprintf(path_sys_sz_over, "%s/nr_overcommit_hugepages", path_sys_sz);
	sprintf(path_sys_sz_free, "%s/free_hugepages", path_sys_sz);
	sprintf(path_sys_sz_resv, "%s/resv_hugepages", path_sys_sz);
	sprintf(path_sys_sz_surp, "%s/surplus_hugepages", path_sys_sz);
	sprintf(path_sys_sz_huge, "%s/nr_hugepages", path_sys_sz);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = (struct tst_option[]) {
		{"s",  &opt_sysfs, "Setup hugepages from sysfs"},
		{"m",  &opt_shmid, "Reserve hugepages by shmget"},
		{"a:", &opt_alloc, "Number of overcommint hugepages"},
		{}
},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_overcommit,
	.hugepages = {NR_HPAGES, TST_NEEDS},
};
