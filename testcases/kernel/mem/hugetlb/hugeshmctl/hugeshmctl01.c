// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2020
 */

/*
 * DESCRIPTION
 *	hugeshmctl01 - test the IPC_STAT, IPC_SET and IPC_RMID commands as
 *		   they are used with shmctl()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	create a large shared memory segment with read and write permission
 *	set up any test case specific conditions
 *	call shmctl() using the TEST macro
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		call the correct test function
 *		if the conditions are correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	  otherwise
 *	    issue a PASS message
 *	call cleanup
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 */

#include <limits.h>
#include "hugetlb.h"

#define N_ATTACH	4U
#define NEWMODE		0066

static size_t shm_size;
static int shm_id_1 = -1;
static struct shmid_ds buf;
static time_t save_time;
static void *attach_to_parent;

static void stat_setup_1(void);
static void stat_cleanup(void);
static void stat_setup_2(void);
static void set_setup(void);
static void func_stat(void);
static void func_set(void);
static void func_rmid(void);
static void *set_shmat(void);

struct tcase {
	int cmd;
	void (*func_test) (void);
	void (*func_setup) (void);
} tcases[] = {
	{IPC_STAT, func_stat, stat_setup_1},
	{IPC_STAT, func_stat, stat_setup_2},
	{IPC_SET,  func_set,  set_setup},
	{IPC_RMID, func_rmid, NULL}
};

static void test_hugeshmctl(unsigned int i)
{
	/*
	 * Create a shared memory segment with read and write
	 * permissions.  Do this here instead of in setup()
	 * so that looping (-i) will work correctly.
	 */
	if (i == 0)
		shm_id_1 = shmget(shmkey, shm_size,
			SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget #main");

	if (tcases[i].func_setup != NULL)
		(*tcases[i].func_setup) ();

	if (shmctl(shm_id_1, tcases[i].cmd, &buf) == -1) {
		tst_res(TFAIL | TERRNO, "shmctl #main");
		return;
	}
	(*tcases[i].func_test)();
}

/*
 * set_shmat() - Attach the shared memory and return the pointer.
 */
void *set_shmat(void)
{
	void *rval;

	rval = shmat(shm_id_1, 0, 0);
	if (rval == (void *)-1)
		tst_brk(TBROK | TERRNO, "set shmat");

	return rval;
}

/*
 * stat_setup_2() - Set up for the IPC_STAT command with shmctl().
 * 		  Attach the shared memory to parent process and
 * 		  some children will inherit the shared memory.
 */
static void stat_setup_2(void)
{
	if (!attach_to_parent)
		attach_to_parent = set_shmat();
	stat_setup_1();
}

/*
 * stat_setup_1() - Set up for the IPC_STAT command with shmctl().
 *                some children will inherit or attatch the shared memory.
 *                It deponds on whther we attach the shared memory
 *                to parent process.
 */
static void stat_setup_1(void)
{
	unsigned int i;
	void *test;
	pid_t pid;

	for (i = 0; i < N_ATTACH; i++) {
		switch (pid = SAFE_FORK()) {
		case 0:
			test = (attach_to_parent == NULL) ? set_shmat() : attach_to_parent;
			/* do an assignement for fun */
			*(int *)test = i;

			TST_CHECKPOINT_WAKE(0);

			TST_CHECKPOINT_WAIT(1);

			/* now we're back - detach the memory and exit */
			if (shmdt(test) == -1)
				tst_brk(TBROK | TERRNO,
					 "shmdt in stat_setup()");

			exit(0);
		default:
			TST_CHECKPOINT_WAIT(0);
		}
	}
}


/*
 * func_stat() - check the functionality of the IPC_STAT command with shmctl()
 *		 by looking at the pid of the creator, the segement size,
 *		 the number of attaches and the mode.
 */
static void func_stat(void)
{
	pid_t pid;
	unsigned int num;

	/* check perm, pid, nattach and size */
	pid = getpid();

	if (buf.shm_cpid != pid) {
		tst_res(TFAIL, "creator pid is incorrect");
		goto fail;
	}

	if (buf.shm_segsz != shm_size) {
		tst_res(TFAIL, "segment size is incorrect");
		goto fail;
	}

	/*
	 * The first case, only the children attach the memory, so
	 * the attaches equal N_ATTACH. The second case, the parent
	 * attaches the memory and the children inherit that memory
	 * so the attaches equal N_ATTACH + 1.
	 */
	num = (attach_to_parent == NULL) ? 0 : 1;
	if (buf.shm_nattch != N_ATTACH + num) {
		tst_res(TFAIL, "# of attaches is incorrect - %lu",
			 (unsigned long)buf.shm_nattch);
		goto fail;
	}

	/* use MODE_MASK to make sure we are comparing the last 9 bits */
	if ((buf.shm_perm.mode & MODE_MASK) != ((SHM_RW) & MODE_MASK)) {
		tst_res(TFAIL, "segment mode is incorrect");
		goto fail;
	}

	tst_res(TPASS, "pid, size, # of attaches and mode are correct "
		 "- pass #%d", num);

fail:
	stat_cleanup();

	/* save the change time for use in the next test */
	save_time = buf.shm_ctime;
}

/*
 * stat_cleanup() - signal the children to clean up after themselves and
 *		    have the parent make dessert, er, um, make that remove
 *		    the shared memory that is no longer needed.
 */
static void stat_cleanup(void)
{
	unsigned int i;
	int status;

	/* wake up the childern so they can detach the memory and exit */
	TST_CHECKPOINT_WAKE2(1, N_ATTACH);

	for (i = 0; i < N_ATTACH; i++)
		SAFE_WAIT(&status);

	/* remove the parent's shared memory if we set*/
	if (attach_to_parent) {
		if (shmdt(attach_to_parent) == -1)
			tst_res(TFAIL | TERRNO, "shmdt in stat_cleanup()");
		attach_to_parent = NULL;
	}
}

/*
 * set_setup() - set up for the IPC_SET command with shmctl()
 */
static void set_setup(void)
{
	/* set up a new mode for the shared memory segment */
	buf.shm_perm.mode = SHM_RW | NEWMODE;

	/* sleep for one second to get a different shm_ctime value */
	sleep(1);
}

/*
 * func_set() - check the functionality of the IPC_SET command with shmctl()
 */
static void func_set(void)
{
	/* first stat the shared memory to get the new data */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
		tst_res(TFAIL | TERRNO, "shmctl in func_set()");
		return;
	}

	if ((buf.shm_perm.mode & MODE_MASK) != ((SHM_RW | NEWMODE) & MODE_MASK)) {
		tst_res(TFAIL, "new mode is incorrect");
		return;
	}

	if (save_time >= buf.shm_ctime) {
		tst_res(TFAIL, "change time is incorrect");
		return;
	}

	tst_res(TPASS, "new mode and change time are correct");
}

/*
 * func_rmid() - check the functionality of the IPC_RMID command with shmctl()
 */
static void func_rmid(void)
{
	/* Do another shmctl() - we should get EINVAL */
	if (shmctl(shm_id_1, IPC_STAT, &buf) != -1)
		tst_brk(TBROK, "shmctl in func_rmid() "
			 "succeeded unexpectedly");
	if (errno != EINVAL)
		tst_res(TFAIL | TERRNO, "shmctl in func_rmid() failed "
			 "unexpectedly - expect errno=EINVAL, got");
	else
		tst_res(TPASS, "shmctl in func_rmid() failed as expected, "
			 "shared memory appears to be removed");
	shm_id_1 = -1;
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
}

void cleanup(void)
{
	rm_shm(shm_id_1);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"s:", &nr_opt, "Set the number of the been allocated hugepages"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test = test_hugeshmctl,
	.needs_checkpoints = 1,
	.request_hugepages = 128,
};
