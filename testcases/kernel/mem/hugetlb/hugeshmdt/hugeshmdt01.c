/*
 *
 *   Copyright (c) International Business Machines  Corp., 2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	hugeshmdt01.c
 *
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
 * USAGE:  <for command-line>
 *  hugeshmdt01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 *
 * RESTRICTIONS
 *	none
 */

#include <setjmp.h>
#include "hugetlb.h"
#include "safe_macros.h"
#include "mem.h"

char *TCID = "hugeshmdt01";
int TST_TOTAL = 1;

static size_t shm_size;
static int shm_id_1 = -1;
struct shmid_ds buf;
static int *shared;
static int pass;
static sigjmp_buf env;

static long hugepages = 128;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

static void check_functionality(void);
static void sighandler(int sig);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, options, NULL);

	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if (shmdt(shared) == -1) {
			tst_resm(TFAIL | TERRNO, "shmdt");
		} else {
			check_functionality();
		}

		/* reattach the shared memory segment in case we are looping */
		shared = shmat(shm_id_1, 0, 0);
		if (shared == (void *)-1)
			tst_brkm(TBROK | TERRNO, cleanup, "shmat #2: reattach");

		/* also reset pass */
		pass = 0;
	}
	cleanup();
	tst_exit();
}

static void check_functionality(void)
{
	/* stat the shared memory segment */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmctl");

	if (buf.shm_nattch != 0) {
		tst_resm(TFAIL, "# of attaches is incorrect");
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
		tst_resm(TPASS, "huge shared memory detached correctly");
	else
		tst_resm(TFAIL, "huge shared memory was not detached "
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
		tst_brkm(TBROK, cleanup, "unexpected signal received: %d", sig);
	}
}

void setup(void)
{
	long hpage_size;

	tst_require_root();
	check_hugepage();
	tst_sig(NOFORK, sighandler, cleanup);
	tst_tmpdir();

	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = read_meminfo("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey(cleanup);

	/* create a shared memory resource with read and write permissions */
	shm_id_1 = shmget(shmkey, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget");

	/* attach the shared memory segment */
	shared = shmat(shm_id_1, 0, 0);
	if (shared == (void *)-1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmat #1");

	/* give a value to the shared memory integer */
	*shared = 4;

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
