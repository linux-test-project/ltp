/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *	semop05.c
 *
 * DESCRIPTION
 *	semop05 - test for EINTR and EIDRM errors
 *
 * ALGORITHM
 *	create a semaphore set with read and alter permissions
 *	loop if that option was specified
 *	set up the s_buf buffer
 *	initialize the primitive semaphores
 *	fork a child process
 *	child calls semop() and sleeps
 *	parent either removes the semaphore set or sends a signal to the child
 *	parent then exits
 *	child gets a return from the semop() call
 *	check the errno value
 *	  issue a PASS message if we get EINTR or EIDRM
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  semop05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *      14/03/2008 Matthieu Fertré (Matthieu.Fertre@irisa.fr)
 *      - Fix concurrency issue. Due to the use of usleep function to
 *        synchronize processes, synchronization issues can occur on a loaded
 *        system. Fix this by using pipes to synchronize processes.
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcsem.h"

#include <sys/types.h>
#include <sys/wait.h>
#include "safe_macros.h"

char *TCID = "semop05";
int TST_TOTAL = 4;

int sem_id_1 = -1;

struct sembuf s_buf;

struct test_case_t {
	union semun semunptr;
	short op;
	short flg;
	short num;
	int error;
} TC[] = {
	/* EIRDM sem_op = 0 */
	{ {
	1}, 0, 0, 2, EIDRM},
	    /* EIRDM sem_op = -1 */
	{ {
	0}, -1, 0, 3, EIDRM},
	    /* EINTR sem_op = 0 */
	{ {
	1}, 0, 0, 4, EINTR},
	    /* EINTR sem_op = -1 */
	{ {
	0}, -1, 0, 5, EINTR}
};

#ifdef UCLINUX
#define PIPE_NAME	"semop05"
void do_child_uclinux();
static int i_uclinux;
#endif

int main(int ac, char **av)
{
	int lc;
	int i;
	pid_t pid;
	void do_child();

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "dd", &i_uclinux, &sem_id_1);
#endif

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* initialize the s_buf buffer */
			s_buf.sem_op = TC[i].op;
			s_buf.sem_flg = TC[i].flg;
			s_buf.sem_num = TC[i].num;

			/* initialize all of the primitive semaphores */
			if (semctl(sem_id_1, TC[i].num, SETVAL, TC[i].semunptr)
			    == -1) {
				tst_brkm(TBROK, cleanup, "semctl() failed");
			}

			if ((pid = FORK_OR_VFORK()) == -1) {
				tst_brkm(TBROK, cleanup, "could not fork");
			}

			if (pid == 0) {	/* child */

#ifdef UCLINUX
				if (self_exec(av[0], "dd", i, sem_id_1) < 0) {
					tst_brkm(TBROK, cleanup,
						 "could not self_exec");
				}
#else
				do_child(i);
#endif
			} else {
				TST_PROCESS_STATE_WAIT(cleanup, pid, 'S');

				/*
				 * If we are testing for EIDRM then remove
				 * the semaphore, else send a signal that
				 * must be caught as we are testing for
				 * EINTR.
				 */
				if (TC[i].error == EIDRM) {
					/* remove the semaphore resource */
					rm_sema(sem_id_1);
				} else {
					SAFE_KILL(cleanup, pid, SIGHUP);
				}

				/* let the child carry on */
				waitpid(pid, NULL, 0);
			}

			/*
			 * recreate the semaphore resource if needed
			 */
			if (TC[i].error == EINTR) {
				continue;
			}

			if ((sem_id_1 = semget(semkey, PSEMS, IPC_CREAT |
					       IPC_EXCL | SEM_RA)) == -1) {
				tst_brkm(TBROK, cleanup, "couldn't recreate "
					 "semaphore");
			}
		}
	}

	cleanup();

	tst_exit();
}

/*
 * do_child()
 */
void do_child(int i)
{
	/*
	 * make the call with the TEST macro
	 */

	TEST(semop(sem_id_1, &s_buf, 1));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "call succeeded when error expected");
		exit(-1);
	}

	if (TEST_ERRNO == TC[i].error) {
		tst_resm(TPASS, "expected failure - errno = %d"
			 " : %s", TEST_ERRNO, strerror(TEST_ERRNO));
	} else {
		tst_resm(TFAIL, "unexpected error - "
			 "%d : %s", TEST_ERRNO, strerror(TEST_ERRNO));
	}

	exit(0);
}

void sighandler(int sig)
{
	if (sig == SIGHUP)
		return;
	else
		tst_brkm(TBROK, NULL, "unexpected signal %d received", sig);
}

#ifdef UCLINUX
/*
 * do_child_uclinux() - capture signals, re-initialize s_buf then call do_child
 *                      with the appropriate argument
 */
void do_child_uclinux(void)
{
	int i = i_uclinux;

	tst_sig(FORK, sighandler, cleanup);

	/* initialize the s_buf buffer */
	s_buf.sem_op = TC[i].op;
	s_buf.sem_flg = TC[i].flg;
	s_buf.sem_num = TC[i].num;

	do_child(i);
}
#endif

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	semkey = getipckey();

	/* create a semaphore set with read and alter permissions */
	/* and PSEMS "primitive" semaphores                       */
	if ((sem_id_1 =
	     semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create semaphore in setup");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the semaphore resource */
	rm_sema(sem_id_1);

	tst_rmdir();
}
