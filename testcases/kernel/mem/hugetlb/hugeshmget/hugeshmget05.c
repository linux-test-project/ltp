// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
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

static size_t shm_size;
static int shm_id_1 = -1;
static uid_t ltp_uid;
static char *ltp_user = "nobody";

static void do_child(void);

static void test_hugeshmget(void)
{
	pid_t pid;
	int status;

	switch (pid = fork()) {
	case -1:
		tst_brk(TBROK | TERRNO, "fork");
		break;
	case 0:
		/* set the user ID of the child to the non root user */
		SAFE_SETUID(ltp_uid);
		do_child();
		exit(0);
	default:
		/* wait for the child to return */
		SAFE_WAITPID(pid, &status, 0);
	}
}

static void do_child(void)
{
	TEST(shmget(shmkey, shm_size, SHM_HUGETLB | SHM_RW));
	if (TST_RET != -1) {
		tst_res(TFAIL, "shmget succeeded unexpectedly");
		return;
	}
	if (TST_ERR == EACCES)
		tst_res(TPASS | TTERRNO, "shmget failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "shmget failed unexpectedly "
				"- expect errno=EACCES, got");
}

void setup(void)
{
	long hpage_size;

	if (tst_hugepages == 0)
		tst_brk(TCONF, "No enough hugepages for testing.");

	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	shm_size = hpage_size * tst_hugepages / 2;
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
}

static struct tst_test test = {
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"s:", &nr_opt, "Set the number of the been allocated hugepages"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmget,
	.hugepages = {128, TST_REQUEST},
};
