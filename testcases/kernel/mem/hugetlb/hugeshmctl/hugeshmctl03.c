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
 *	hugeshmctl03 - check for EACCES, and EPERM errors
 *
 * ALGORITHM
 *	create a large shared memory segment with root only read & write
 *	permissions fork a child process
 *	if child
 *	  set the ID of the child process to that of "ltpuser1"
 *	  call do_child()
 *	  loop if that option was specified
 *	    call shmctl() using three different invalid cases
 *	    check the errno value
 *	      issue a PASS message if we get EACCES or EPERM
 *	    otherwise, the tests fails
 *	      issue a FAIL message
 *	  call cleanup
 *	if parent
 *	  wait for child to exit
 *	  remove the large shared memory segment
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 *
 * RESTRICTIONS
 *	test must be run as root
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include "mem.h"
#include "hugetlb.h"

static size_t shm_size;
static int shm_id_1 = -1;
static struct shmid_ds buf;
static uid_t ltp_uid;
static char *ltp_user = "nobody";

static long hugepages = 128;

static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

struct tcase {
	int *shmid;
	int cmd;
	struct shmid_ds *sbuf;
	int error;
} tcases[] = {
	/* EACCES - child has no read permission for segment */
	{&shm_id_1, IPC_STAT, &buf, EACCES},
	/* EPERM - IPC_SET - child doesn't have permission to change segment */
	{&shm_id_1, IPC_SET, &buf, EPERM},
	/* EPERM - IPC_RMID - child can not remove the segment */
	{&shm_id_1, IPC_RMID, &buf, EPERM},
};

static void do_child(void);

static void test_hugeshmctl(void)
{
	pid_t pid;
	int status;

	switch (pid = SAFE_FORK()) {
	case 0:
		/* set the user ID of the child to the non root user */
		if (setuid(ltp_uid) == -1)
			tst_brk(TBROK | TERRNO, "setuid");
		do_child();
		exit(0);
	default:
		if (waitpid(pid, &status, 0) == -1)
			tst_brk(TBROK | TERRNO, "waitpid");
	}
}

static void do_child(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		TEST(shmctl(*(tcases[i].shmid), tcases[i].cmd, tcases[i].sbuf));
		if (TEST_RETURN != -1) {
			tst_res(TFAIL, "shmctl succeeded "
					"unexpectedly");
			continue;
		}
		if (TEST_ERRNO == tcases[i].error)
			tst_res(TPASS | TTERRNO, "shmctl failed "
					"as expected");
		else
			tst_res(TFAIL | TTERRNO, "shmctl failed "
					"unexpectedly - expect errno = "
					"%d, got", tcases[i].error);
	}
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
			  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget");

	/* get the userid for a non root user */
	ltp_uid = getuserid(ltp_user);
}

void cleanup(void)
{
	rm_shm(shm_id_1);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmctl,
};
