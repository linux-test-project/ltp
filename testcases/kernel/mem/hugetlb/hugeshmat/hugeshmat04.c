// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2014-2017
 *
 * DESCRIPTION
 *	hugeshmat04 - test for hugepage leak inspection.
 *
 *	It is a regression test for shared hugepage leak, when over 1GB
 *	shared memory was alocated in hugepage, the hugepage is not released
 *	though process finished.
 *
 *	You need more than 2GB memory in test job
 *
 * HISTORY
 * 	05/2014 - Written by Fujistu Corp.
 *	12/2014 - Port to LTP by Li Wang.
 *
 * RESTRICTIONS
 * 	test must be run at root
 */

#include "hugetlb.h"

#define SIZE	(1024 * 1024 * 1024)
#define BOUNDARY (1024 * 1024 * 1024)
#define BOUNDARY_MAX (3U * 1024 * 1024 * 1024)

static long huge_free;
static long huge_free2;

static void shared_hugepage(void);

static void test_hugeshmat(unsigned int i LTP_ATTRIBUTE_UNUSED)
{
	huge_free = SAFE_READ_MEMINFO("HugePages_Free:");
	shared_hugepage();
	huge_free2 = SAFE_READ_MEMINFO("HugePages_Free:");

	if (huge_free2 != huge_free)
		tst_brk(TFAIL, "Test failed. Hugepage leak inspection.");
	else
		tst_res(TPASS, "No regression found.");
}

static void shared_hugepage(void)
{
	pid_t pid;
	int status, shmid;
	size_t size = (size_t)SIZE;
	void *buf;
	unsigned long boundary = BOUNDARY;

	shmid = shmget(IPC_PRIVATE, size, SHM_HUGETLB | IPC_CREAT | 0777);
	if (shmid < 0)
		tst_brk(TBROK | TERRNO, "shmget");

	while (boundary <= BOUNDARY_MAX
		&& range_is_mapped(boundary, boundary+SIZE))
		boundary += 128*1024*1024;
	if (boundary > BOUNDARY_MAX)
		tst_brk(TCONF, "failed to find free unmapped range");

	tst_res(TINFO, "attaching at 0x%lx", boundary);
	buf = shmat(shmid, (void *)boundary, SHM_RND | 0777);
	if (buf == (void *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		tst_brk(TBROK | TERRNO, "shmat");
	}

	memset(buf, 2, size);
	pid = SAFE_FORK();
	if (pid == 0)
		exit(1);

	wait(&status);
	shmdt(buf);
	shmctl(shmid, IPC_RMID, NULL);
}

static void setup(void)
{
	long hpage_size, orig_hugepages;
	unsigned long new_shmmax;

	orig_hugepages = get_sys_tune("nr_hugepages");
	SAFE_FILE_SCANF(PATH_SHMMAX, "%lu", &new_shmmax);

	if (new_shmmax < SIZE)
		tst_brk(TCONF,	"shmmax too low, have: %lu", new_shmmax);

	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	struct tst_hugepage hp = { orig_hugepages + SIZE / hpage_size, TST_NEEDS };
	tst_reserve_hugepages(&hp);
}

static struct tst_test test = {
	.tags = (const struct tst_tag[]) {
		{"linux-git", "c5c99429fa57"},
		{}
	},
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.tcnt = 3,
	.test = test_hugeshmat,
	.min_mem_avail = 2048,
	.setup = setup,
	.hugepages = {1, TST_NEEDS},
	.save_restore = (const struct tst_path_val[]) {
		{PATH_SHMMAX, "1073741824", TST_SR_TCONF_MISSING | TST_SR_TBROK_RO},
		{}
	},
};
