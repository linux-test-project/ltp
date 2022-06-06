// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * DESCRIPTION
 *	hugeshmat03 - test for EACCES error
 *
 * ALGORITHM
 *	create a shared memory segment with root only read & write permissions
 *	fork a child process
 *	if child
 *	  set the ID of the child process to that of "nobody"
 *	  loop if that option was specified
 *	    call shmat() using the TEST() macro
 *	    check the errno value
 *	      issue a PASS message if we get EACCES
 *	    otherwise, the tests fails
 *	      issue a FAIL message
 *	  call cleanup
 *	if parent
 *	  wait for child to exit
 *	  remove the shared memory segment
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 *
 * RESTRICTIONS
 *	test must be run at root
 */

#include <limits.h>
#include "hugetlb.h"

static size_t shm_size;
static int shm_id_1 = -1;
static void *addr;
static uid_t ltp_uid;
static char *ltp_user = "nobody";

static void verify_hugeshmat(void)
{
	int status;
	pid_t pid;

	switch (pid = SAFE_FORK()) {
	case 0:
		SAFE_SETUID(ltp_uid);

		addr = shmat(shm_id_1, NULL, 0);
		if (addr != (void *)-1) {
			tst_res(TFAIL, "shmat succeeded unexpectedly");
			return;
		}
		if (errno == EACCES) {
			tst_res(TPASS | TERRNO, "shmat failed as expected");
		} else {
			tst_res(TFAIL | TERRNO, "shmat failed unexpectedly "
					"- expect errno=EACCES, got");
		}
		break;
	default:
		SAFE_WAITPID(pid, &status, 0);
	}
}

static void setup(void)
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
		tst_brk(TBROK | TERRNO, "shmget");

	ltp_uid = getuserid(ltp_user);
}

static void cleanup(void)
{
	rm_shm(shm_id_1);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.options = (struct tst_option[]) {
		{"s:", &nr_opt, "Set the number of the been allocated hugepages"},
		{}
	},
	.test_all = verify_hugeshmat,
	.setup = setup,
	.cleanup = cleanup,
	.hugepages = {128, TST_REQUEST},
};
