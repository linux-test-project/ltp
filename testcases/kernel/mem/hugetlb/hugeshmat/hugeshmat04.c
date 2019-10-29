/*
 * Copyright (c) Linux Test Project, 2014-2017
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
 */

/*
 * DESCRIPTION
 *	hugeshmat04 - test for hugepage leak inspection.
 *
 *	It is a regression test for shared hugepage leak, when over 1GB
 *	shered memory was alocated in hugepage, the hugepage is not released
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

static long huge_free;
static long huge_free2;
static long hugepages;
static long orig_shmmax, new_shmmax;

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

	shmid = shmget(IPC_PRIVATE, size, SHM_HUGETLB | IPC_CREAT | 0777);
	if (shmid < 0)
		tst_brk(TBROK | TERRNO, "shmget");

	buf = shmat(shmid, (void *)BOUNDARY, SHM_RND | 0777);
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
	long mem_total, hpage_size, orig_hugepages;

	SAFE_FILE_SCANF(PATH_SHMMAX, "%ld", &orig_shmmax);
	orig_hugepages = save_nr_hugepages();
	mem_total = SAFE_READ_MEMINFO("MemTotal:");
	SAFE_FILE_PRINTF(PATH_SHMMAX, "%ld", (long)SIZE);
	SAFE_FILE_SCANF(PATH_SHMMAX, "%ld", &new_shmmax);

	if (mem_total < 2L*1024*1024)
		tst_brk(TCONF,	"Needed > 2GB RAM, have: %ld", mem_total);

	if (new_shmmax < SIZE)
		tst_brk(TCONF,	"shmmax too low, have: %ld", new_shmmax);

	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	hugepages = orig_hugepages + SIZE / hpage_size;
	set_sys_tune("nr_hugepages", hugepages, 1);
}

static void cleanup(void)
{
	restore_nr_hugepages();
	SAFE_FILE_PRINTF(PATH_SHMMAX, "%ld", orig_shmmax);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.tcnt = 3,
	.test = test_hugeshmat,
	.setup = setup,
	.cleanup = cleanup,
};
