/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
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
 *	hugeshmget05 - test for EACCES error
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include "hugetlb.h"
#include "mem.h"
#include "hugetlb.h"

static size_t shm_size;
static int shm_id_1 = -1;
static uid_t ltp_uid;
static char *ltp_user = "nobody";

static long hugepages = 128;
static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

static void do_child(void);

static void test_hugeshmget(void)
{
	pid_t pid;
	int status;

	switch (pid = fork()) {
	case -1:
		tst_brk(TBROK | TERRNO, "fork");
	case 0:
		/* set the user ID of the child to the non root user */
		if (setuid(ltp_uid) == -1)
			tst_brk(TBROK | TERRNO, "setuid");
		do_child();
		exit(0);
	default:
		/* wait for the child to return */
		if (waitpid(pid, &status, 0) == -1)
			tst_brk(TBROK | TERRNO, "waitpid");
	}
}

static void do_child(void)
{
	TEST(shmget(shmkey, shm_size, SHM_HUGETLB | SHM_RW));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "shmget succeeded unexpectedly");
		return;
	}
	if (TEST_ERRNO == EACCES)
		tst_res(TPASS | TTERRNO, "shmget failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "shmget failed unexpectedly "
				"- expect errno=EACCES, got");
}

void setup(void)
{
	long hpage_size;

	check_hugepage();
	if (nr_opt)
		hugepages = SAFE_STRTOL(nr_opt, 0, LONG_MAX);

	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey();
	shm_id_1 = shmget(shmkey, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget #setup");

	/* get the userid for a non-root user */
	ltp_uid = getuserid(ltp_user);
}

void cleanup(void)
{
	rm_shm(shm_id_1);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmget,
};
