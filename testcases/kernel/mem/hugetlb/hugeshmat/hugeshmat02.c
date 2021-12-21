// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * DESCRIPTION
 *	hugeshmat02 - check for EINVAL and EACCES errors with hugetlb
 *
 * ALGORITHM
 *	loop if that option was specified
 *	  call shmat() using three invalid test cases
 *	  check the errno value
 *	    issue a PASS message if we get EINVAL or EACCES
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated By Robbie Williamson
 *
 * RESTRICTIONS
 *	Must be ran as root
 */

#include <pwd.h>
#include <limits.h>
#include "lapi/abisize.h"
#include "hugetlb.h"

#ifdef TST_ABI64
#define NADDR	0x10000000eef	/* a 64bit non alligned address value */
#else
#define NADDR	0x60000eef	/* a non alligned address value */
#endif

static size_t shm_size;
static int shm_id_1 = -1;
static int shm_id_2 = -1;
static void *addr;

struct tcase {
	int *shmid;
	void *addr;
	int error;
} tcases[] = {
	/* EINVAL - the shared memory ID is not valid */
	{&shm_id_1, NULL, EINVAL},
	/* EINVAL - the address is not page aligned and SHM_RND is not given */
	{&shm_id_2, (void *)NADDR, EINVAL}
};

static void verify_hugeshmat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	addr = shmat(*(tc->shmid), tc->addr, 0);
	if (addr != (void *)-1) {
		tst_res(TFAIL, "shmat suceeded unexpectedly");
		return;
	}

	if (errno == tc->error) {
		tst_res(TPASS | TERRNO, "shmat failed as "
				"expected");
	} else {
		tst_res(TFAIL | TERRNO, "shmat failed "
				"unexpectedly - expect errno=%d, "
				"got", tc->error);
	}
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

	/* create a shared memory resource with read and write permissions */
	/* also post increment the shmkey for the next shmget call */
	shm_id_2 = shmget(shmkey++, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_2 == -1)
		tst_brk(TBROK | TERRNO, "shmget");
}

void cleanup(void)
{
	rm_shm(shm_id_2);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = (struct tst_option[]) {
		{"s:", &nr_opt, "Set the number of the been allocated hugepages"},
		{}
	},
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_hugeshmat,
	.setup = setup,
	.cleanup = cleanup,
	.request_hugepages = 128,
};
