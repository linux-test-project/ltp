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
 *	hugeshmdt01 - check that largr shared memory is detached correctly
 *
 * ALGORITHM
 *	create a large shared memory resource
 *	attach it to the current process and give it a value
 *	call shmdt() using the TEST macro
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		attempt to write a value to the large shared memory address
 *		this should generate a SIGSEGV which will be caught in
 *		    the signal handler
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

#include <setjmp.h>
#include <limits.h>
#include "hugetlb.h"
#include "mem.h"
#include "hugetlb.h"

static size_t shm_size;
static int shm_id_1 = -1;
struct shmid_ds buf;
static int *shared;
static int pass;
static sigjmp_buf env;

static long hugepages = 128;
static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

static void check_functionality(void);
static void sighandler(int sig);

static void hugeshmdt_test(void)
{
	struct sigaction sa;

	sa.sa_handler = sighandler;
	sigaction(SIGSEGV, &sa, NULL);

	if (shmdt(shared) == -1)
		tst_res(TFAIL | TERRNO, "shmdt");
	else
		check_functionality();

	/* reattach the shared memory segment in case we are looping */
	shared = shmat(shm_id_1, 0, 0);
	if (shared == (void *)-1)
		tst_brk(TBROK | TERRNO, "shmat #2: reattach");

	/* also reset pass */
	pass = 0;
}

static void check_functionality(void)
{
	/* stat the shared memory segment */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1)
		tst_brk(TBROK | TERRNO, "shmctl");

	if (buf.shm_nattch != 0) {
		tst_res(TFAIL, "# of attaches is incorrect");
		return;
	}

	/*
	 * Try writing to the shared memory.  This should generate a
	 * SIGSEGV which will be caught below.
	 *
	 * This is wrapped by the sigsetjmp() call that will take care of
	 * restoring the program's context in an elegant way in conjunction
	 * with the call to siglongjmp() in the signal handler.
	 *
	 * An attempt to do the assignment without using the sigsetjmp()
	 * and siglongjmp() calls will result in an infinite loop.  Program
	 * control is returned to the assignment statement after the execution
	 * of the signal handler and another SIGSEGV will be generated.
	 */

	if (sigsetjmp(env, 1) == 0)
		*shared = 2;

	if (pass)
		tst_res(TPASS, "huge shared memory detached correctly");
	else
		tst_res(TFAIL, "huge shared memory was not detached "
			 "correctly");
}

static void sighandler(int sig)
{
	/* if we have received a SIGSEGV, we are almost done */
	if (sig == SIGSEGV) {
		/* set the global variable and jump back */
		pass = 1;
		siglongjmp(env, 1);
	} else {
		tst_brk(TBROK, "unexpected signal received: %d", sig);
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

	/* create a shared memory resource with read and write permissions */
	shm_id_1 = shmget(shmkey, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget");

	/* attach the shared memory segment */
	shared = shmat(shm_id_1, 0, 0);
	if (shared == (void *)-1)
		tst_brk(TBROK | TERRNO, "shmat #1");

	/* give a value to the shared memory integer */
	*shared = 4;
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
	.test_all = hugeshmdt_test,
};
