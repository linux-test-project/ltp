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

#define FIRST		0
#define SECOND		1
#define N_ATTACH	4U
#define NEWMODE		0066

static size_t shm_size;
static int shm_id_1 = -1;
static struct shmid_ds buf;
static time_t save_time;
static int stat_time;
static void *set_shared;

static void stat_setup(void);
static void stat_cleanup(void);
static void set_setup(void);
static void func_stat(void);
static void func_set(void);
static void func_rmid(void);
static void *set_shmat(void);

static long hugepages = 128;

static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

struct tcase {
	int cmd;
	void (*func_test) (void);
	void (*func_setup) (void);
} tcases[] = {
	{IPC_STAT, func_stat, stat_setup},
	{IPC_STAT, func_stat, stat_setup},
	{IPC_SET,  func_set,  set_setup},
	{IPC_RMID, func_rmid, NULL}
};

static void test_hugeshmctl(void)
{
	unsigned int i;

	/* initialize stat_time */
	stat_time = FIRST;

	/*
	 * Create a shared memory segment with read and write
	 * permissions.  Do this here instead of in setup()
	 * so that looping (-i) will work correctly.
	 */
	shm_id_1 = shmget(shmkey, shm_size,
			SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget #main");

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		/*
		 * if needed, set up any required conditions by
		 * calling the appropriate setup function
		 */
		if (tcases[i].func_setup != NULL)
			(*tcases[i].func_setup) ();

		if (shmctl(shm_id_1, tcases[i].cmd, &buf) == -1) {
			tst_res(TFAIL | TERRNO, "shmctl #main");
			continue;
		}
		(*tcases[i].func_test) ();
	}
}

/*
 * set_shmat() - Attach the shared memory and return the pointer.  Use
 *		 this seperate routine to avoid code duplication in
 *		 stat_setup() below.
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
 * stat_setup() - Set up for the IPC_STAT command with shmctl().
 *		  Make things interesting by forking some children
 *		  that will either attach or inherit the shared memory.
 */
static void stat_setup(void)
{
	unsigned int i;
	void *test;
	pid_t pid;

	/*
	 * The first time through, let the children attach the memory.
	 * The second time through, attach the memory first and let
	 * the children inherit the memory.
	 */

	if (stat_time == SECOND) {
		/*
		 * use the global "set_shared" variable here so that
		 * it can be removed in the stat_func() routine.
		 */
		set_shared = set_shmat();
	}

	for (i = 0; i < N_ATTACH; i++) {
		switch (pid = SAFE_FORK()) {
		case 0:
			test = (stat_time == FIRST) ? set_shmat() : set_shared;

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
	 * The first time through, only the children attach the memory, so
	 * the attaches equal N_ATTACH + stat_time (0).  The second time
	 * through, the parent attaches the memory and the children inherit
	 * that memory so the attaches equal N_ATTACH + stat_time (1).
	 */
	if (buf.shm_nattch != N_ATTACH + stat_time) {
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
		 "- pass #%d", stat_time);

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

	/* remove the parent's shared memory the second time through */
	if (stat_time == SECOND)
		if (shmdt(set_shared) == -1)
			tst_res(TBROK | TERRNO, "shmdt in stat_cleanup()");
	stat_time++;
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
		tst_res(TBROK | TERRNO, "shmctl in func_set()");
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

	save_nr_hugepages();
	if (nr_opt)
		hugepages = SAFE_STRTOL(nr_opt, 0, LONG_MAX);

	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey();
}

void cleanup(void)
{
	rm_shm(shm_id_1);
	restore_nr_hugepages();
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmctl,
	.needs_checkpoints = 1,
};
