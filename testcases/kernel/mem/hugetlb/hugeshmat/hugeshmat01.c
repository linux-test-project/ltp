// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Linux Test Project, 2001-2017
 *
 * DESCRIPTION
 *	hugeshmat01 - test that shmat() works correctly
 *
 * ALGORITHM
 *	create a large shared memory resouce with read/write permissions
 *	loop if that option was specified
 *	call shmat() with the TEST() macro using three valid conditions
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		check for the correct conditions after the call
 *		if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 */

#include <limits.h>
#include "hugetlb.h"

#define CASE0 10 /* values to write into the shared */
#define CASE1 20 /* memory location.                */

static size_t shm_size;
static int shm_id_1 = -1;
static void *addr;

static struct tst_option options[] = {
	{"s:", &nr_opt, "-s num   Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

static struct tcase {
	int *shmid;
	void *addr;
	int flags;
} tcases[] = {
	/* a straight forward read/write attach */
	{&shm_id_1, 0, 0},
	/*
	 * an attach using non aligned memory
	 * -1 will be replaced with an unaligned addr
	 */
	{&shm_id_1, (void *)-1, SHM_RND},
	/* a read only attach */
	{&shm_id_1, 0, SHM_RDONLY}
};

static void check_functionality(unsigned int i);

static void verify_hugeshmat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	addr = shmat(*(tc->shmid), tc->addr, tc->flags);
	if (addr == (void *)-1) {
		tst_brk(TFAIL | TERRNO, "shmat");
	} else {
		check_functionality(i);
	}

	/*
	 * addr in tcases[0] will be used to generate an unaligned
	 * address for tcases[1]
	 */
	if (i == 0 && addr != (void *)-1)
		tc[1].addr = (void *)(((unsigned long)addr &
					~(SHMLBA - 1)) + SHMLBA - 1);
	if (shmdt(addr) == -1)
		tst_brk(TBROK | TERRNO, "shmdt");
}

/*
 * check_functionality - check various conditions to make sure they
 *			 are correct.
 */
static void check_functionality(unsigned int i)
{
	void *orig_add;
	int *shared;
	struct shmid_ds buf;

	shared = (int *)addr;

	/* stat the shared memory ID */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1)
		tst_brk(TBROK | TERRNO, "shmctl");

	/* check the number of attaches */
	if (buf.shm_nattch != 1) {
		tst_res(TFAIL, "# of attaches is incorrect");
		return;
	}

	/* check the size of the segment */
	if (buf.shm_segsz != shm_size) {
		tst_res(TFAIL, "segment size is incorrect");
		return;
	}

	/* check for specific conditions depending on the type of attach */
	switch (i) {
	case 0:
		/*
		 * Check the functionality of the first call by simply
		 * "writing" a value to the shared memory space.
		 * If this fails the program will get a SIGSEGV, dump
		 * core and exit.
		 */
		*shared = CASE0;
		break;
	case 1:
		/*
		 * Check the functionality of the second call by writing
		 * a value to the shared memory space and then checking
		 * that the original address given was rounded down as
		 * specified in the man page.
		 */
		*shared = CASE1;
		orig_add = addr + ((unsigned long)tcases[i].addr % SHMLBA);
		if (orig_add != tcases[i].addr) {
			tst_res(TFAIL, "shared memory address is not "
				 "correct");
			return;
		}
		break;
	case 2:
		/*
		 * This time the shared memory is read only.  Read the value
		 * and check that it is equal to the value set in case #2,
		 * because shared memory is persistent.
		 */
		if (*shared != CASE1) {
			tst_res(TFAIL, "shared memory value isn't correct");
			return;
		}
		break;
	}
	tst_res(TPASS, "conditions and functionality are correct");
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
	shm_id_1 = shmget(shmkey++, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget");

}

static void cleanup(void)
{
	rm_shm(shm_id_1);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = options,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_hugeshmat,
	.setup = setup,
	.cleanup = cleanup,
	.request_hugepages = 128,
};
