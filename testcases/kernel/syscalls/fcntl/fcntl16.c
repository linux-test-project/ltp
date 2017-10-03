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
 *	fcntl16.c
 *
 * DESCRIPTION
 *	Additional file locking test cases for checking proper notifictaion
 *	of processes on lock change
 *
 * ALGORITHM
 *	Various test cases are used to lock a file opened without mandatory
 *	locking, with madatory locking and mandatory locking with NOBLOCK.
 *	Checking that processes waiting on lock boundaries are notified
 *	properly when boundaries change
 *
 * USAGE
 *	fcntl16
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	04/2002 wjhuie sigset cleanups
 *
 * RESTRICTIONS
 *	None
 */

#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


#define SKIPVAL 0x0f00
//#define       SKIP    SKIPVAL, 0, 0L, 0L, IGNORED
#define SKIP 0,0,0L,0L,0
#if (SKIPVAL == F_RDLCK) || (SKIPVAL == F_WRLCK)
#error invalid SKIP, must not be F_RDLCK or F_WRLCK
#endif

#define	IGNORED		0
#define	NOBLOCK		2	/* immediate success */
#define	WILLBLOCK	3	/* blocks, succeeds, parent unlocks records */
#define	TIME_OUT	10
int NO_NFS = 1;			/* Test on NFS or not */

typedef struct {
	struct flock parent_a;
	struct flock parent_b;
	struct flock child_a;
	struct flock child_b;
	struct flock parent_c;
	struct flock parent_d;
} testcase;

static testcase testcases[] = {
	/* #1 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 1 to byte 5 */
	 {F_RDLCK, 0, 0L, 5L, NOBLOCK},
	 /* Child_b read lock on byte 6 to byte 10 */
	 {F_RDLCK, 0, 6L, 5L, NOBLOCK},
	 /*
	  * Parent_c read lock on entire file
	  */
	 {F_RDLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #2 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 1 to byte 5 */
	 {F_RDLCK, 0, 0L, 5L, WILLBLOCK},
	 /* Child_b read lock on byte 6 to byte 10 */
	 {F_RDLCK, 0, 6L, 5L, WILLBLOCK},
	 /*
	  * Parent_c write lock on entire
	  * file
	  */
	 {F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #3 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 2 to byte 4 */
	 {F_RDLCK, 0, 2L, 3L, WILLBLOCK},
	 /* Child_b read lock on byte 6 to byte 8 */
	 {F_RDLCK, 0, 6L, 3L, WILLBLOCK},
	 /*
	  * Parent_c read lock on byte 3 to
	  * byte 7
	  */
	 {F_RDLCK, 0, 3L, 5L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #4 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 2 to byte 4 */
	 {F_RDLCK, 0, 2L, 3L, WILLBLOCK},
	 /* Child_b read lock on byte 6 to byte 8 */
	 {F_RDLCK, 0, 6L, 3L, NOBLOCK},
	 /*
	  * Parent_c read lock on byte 5 to
	  * byte 9
	  */
	 {F_RDLCK, 0, 5L, 5L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #5 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 3 to byte 7 */
	 {F_RDLCK, 0, 3L, 5L, NOBLOCK},
	 /* Child_b read lock on byte 5 to byte 10 */
	 {F_RDLCK, 0, 5L, 6L, WILLBLOCK},
	 /*
	  * Parent_c read lock on byte 2 to
	  * byte 8
	  */
	 {F_RDLCK, 0, 2L, 7L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #6 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 2 to byte 4 */
	 {F_RDLCK, 0, 2L, 3L, WILLBLOCK},
	 /* Child_b write lock on byte 6 to byte 8 */
	 {F_RDLCK, 0, 6L, 3L, NOBLOCK},
	 /* Parent_c no lock on byte 3 to 9 */
	 {F_UNLCK, 0, 3L, 7L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #7 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b read lock on byte 3 to byte 7 */
	 {F_RDLCK, 0, 3L, 5L, IGNORED},
	 /* Child_a read lock on byte 2 to byte 4 */
	 {F_RDLCK, 0, 2L, 3L, NOBLOCK},
	 /* Child_b read lock on byte 6 to byte 8 */
	 {F_RDLCK, 0, 6L, 3L, NOBLOCK},
	 /*
	  * Parent_c read lock on byte 1 to
	  * byte 9
	  */
	 {F_RDLCK, 0, 1L, 9L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},

	/* #8 Parent_a making a write lock on byte 2 to byte 4 */
	{{F_WRLCK, 0, 2L, 3L, IGNORED},
	 /* Parent_b write lock on byte 6 to byte 8 */
	 {F_WRLCK, 0, 6L, 3L, IGNORED},
	 /* Child_a read lock on byte 3 to byte 7 */
	 {F_RDLCK, 0, 3L, 5L, NOBLOCK},
	 /* Child_b skipped */
	 {SKIP},
	 /*
	  * Parent_c read lock on byte 1 to
	  * byte 5
	  */
	 {F_RDLCK, 0, 1L, 5L, IGNORED},
	 /*
	  * Parent_d read lock on
	  * byte 5 to byte 9
	  */
	 {F_RDLCK, 0, 5L, 5L,
	  IGNORED},},

	/* #9 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b read lock on byte 3 to byte 7 */
	 {F_RDLCK, 0, 3L, 5L, IGNORED},
	 /* Child_a read lock on byte 2 to byte 4 */
	 {F_RDLCK, 0, 2L, 3L, NOBLOCK},
	 /* Child_b read lock on byte 6 to byte 8 */
	 {F_RDLCK, 0, 6L, 3L, NOBLOCK},
	 /*
	  * Parent_c read lock on byte 1 to
	  * byte 3
	  */
	 {F_RDLCK, 0, 1L, 3L, IGNORED},
	 /*
	  * Parent_d read lock on
	  * byte 7 to byte 9
	  */
	 {F_RDLCK, 0, 7L, 3L,
	  IGNORED},},

	/* #10 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 2 to byte 4 */
	 {F_RDLCK, 0, 2L, 3L, NOBLOCK},
	 /* Child_b read lock on byte 6 to byte 8 */
	 {F_RDLCK, 0, 6L, 3L, NOBLOCK},
	 /*
	  * Parent_c read lock on byte 1 to
	  * byte 7
	  */
	 {F_RDLCK, 0, 1L, 7L, IGNORED},
	 /*
	  * Parent_d read lock on
	  * byte 3 to byte 9
	  */
	 {F_RDLCK, 0, 3L, 7L,
	  IGNORED},},

	/* #11 Parent_a making a write lock on entire file */
	{{F_WRLCK, 0, 0L, 0L, IGNORED},
	 /* Parent_b skipped */
	 {SKIP},
	 /* Child_a read lock on byte 3 to byte 7 */
	 {F_RDLCK, 0, 3L, 5L, NOBLOCK},
	 /* Child_b read lock on byte 3 to byte 7 */
	 {F_RDLCK, 0, 3L, 5L, NOBLOCK},
	 /*
	  * Parent_c read lock on byte 3 to
	  * byte 7
	  */
	 {F_RDLCK, 0, 3L, 5L, IGNORED},
	 /* Parent_d skipped */
	 {SKIP},},
};

static testcase *thiscase;
static struct flock *thislock;
static int parent;
static int child_flag1 = 0;
static int child_flag2 = 0;
static int parent_flag = 0;
static int alarm_flag = 0;
static int child_pid[2], flag[2];
static int fd;
static int test;
static char tmpname[40];

#define	FILEDATA	"tenbytes!"

extern void catch_int(int sig);	/* signal catching subroutine */

char *TCID = "fcntl16";
int TST_TOTAL = 1;

#ifdef UCLINUX
static char *argv0;
#endif

/*
 * cleanup - performs all the ONE TIME cleanup for this test at completion or
 *	premature exit
 */
void cleanup(void)
{
	tst_rmdir();

}

void dochild(int kid)
{
	/* child process */
	struct sigaction sact;
	sact.sa_flags = 0;
	sact.sa_handler = catch_int;
	sigemptyset(&sact.sa_mask);
	(void)sigaction(SIGUSR1, &sact, NULL);

	/* Lock should succeed after blocking and parent releases lock */
	if (kid) {
		if ((kill(parent, SIGUSR2)) < 0) {
			tst_resm(TFAIL, "Attempt to send signal to parent "
				 "failed");
			tst_resm(TFAIL, "Test case %d, child %d, errno = %d",
				 test + 1, kid, errno);
			exit(1);
		}
	} else {
		if ((kill(parent, SIGUSR1)) < 0) {
			tst_resm(TFAIL, "Attempt to send signal to parent "
				 "failed");
			tst_resm(TFAIL, "Test case %d, child %d, errno = %d",
				 test + 1, kid, errno);
			exit(1);
		}
	}

	if ((fcntl(fd, F_SETLKW, thislock)) < 0) {
		if (errno == EINTR && parent_flag) {
			/*
			 * signal received is waiting for lock to clear,
			 * this is expected if flag = WILLBLOCK
			 */
			exit(1);
		} else {
			tst_resm(TFAIL, "Attempt to set child BLOCKING lock "
				 "failed");
			tst_resm(TFAIL, "Test case %d, errno = %d", test + 1,
				 errno);
			exit(2);
		}
	}
	exit(0);
}				/* end of child process */

#ifdef UCLINUX
static int kid_uc;

void dochild_uc(void)
{
	dochild(kid_uc);
}
#endif

void catch_alarm(int sig)
{
	alarm_flag = 1;
}

void catch_usr1(int sig)
{				/* invoked on catching SIGUSR1 */
	/*
	 * Set flag to let parent know that child #1 is ready to have the
	 * lock removed
	 */
	child_flag1 = 1;
}

void catch_usr2(int sig)
{				/* invoked on catching SIGUSR2 */
	/*
	 * Set flag to let parent know that child #2 is ready to have the
	 * lock removed
	 */
	child_flag2 = 1;
}

void catch_int(int sig)
{				/* invoked on child catching SIGUSR1 */
	/*
	 * Set flag to interrupt fcntl call in child and force a controlled
	 * exit
	 */
	parent_flag = 1;
}

void child_sig(int sig, int nkids)
{
	int i;

	for (i = 0; i < nkids; i++) {
		if (kill(child_pid[i], 0) == 0) {
			if ((kill(child_pid[i], sig)) < 0) {
				tst_resm(TFAIL, "Attempt to signal child %d, "
					 "failed", i + 1);
			}
		}
	}
}

/*
 * setup - performs all ONE TIME steup for this test
 */
void setup(void)
{
	struct sigaction sact;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if option was specified */
	TEST_PAUSE;

	parent = getpid();

	tst_tmpdir();

	/* On NFS or not */
	if (tst_fs_type(cleanup, ".") == TST_NFS_MAGIC)
		NO_NFS = 0;

	/* set up temp filename */
	sprintf(tmpname, "fcntl4.%d", parent);

	/*
	 * Set up signal handling functions
	 */
	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = catch_usr1;
	sigemptyset(&sact.sa_mask);
	sigaddset(&sact.sa_mask, SIGUSR1);
	sigaction(SIGUSR1, &sact, NULL);

	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = catch_usr2;
	sigemptyset(&sact.sa_mask);
	sigaddset(&sact.sa_mask, SIGUSR2);
	sigaction(SIGUSR2, &sact, NULL);

	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = catch_alarm;
	sigemptyset(&sact.sa_mask);
	sigaddset(&sact.sa_mask, SIGALRM);
	sigaction(SIGALRM, &sact, NULL);
}

int run_test(int file_flag, int file_mode, int start, int end)
{
	int child_count;
	int child;
	int nexited;
	int status, expect_stat;
	int i, fail = 0;

	/* loop through all test cases */
	for (test = start; test < end; test++) {
		/* open a temp file to lock */
		fd = SAFE_OPEN(cleanup, tmpname, file_flag, file_mode);

		/* write some dummy data to the file */
		(void)write(fd, FILEDATA, 10);

		/* Initialize first parent lock structure */
		thiscase = &testcases[test];
		thislock = &thiscase->parent_a;

		/* set the initial parent lock on the file */
		if ((fcntl(fd, F_SETLK, thislock)) < 0) {
			tst_resm(TFAIL, "First parent lock failed");
			tst_resm(TFAIL, "Test case %d, errno = %d", test + 1,
				 errno);
			close(fd);
			unlink(tmpname);
			return 1;
		}

		/* Initialize second parent lock structure */
		thislock = &thiscase->parent_b;

		if ((thislock->l_type) != IGNORED) {	/*SKIPVAL */
			/* set the second parent lock */
			if ((fcntl(fd, F_SETLK, thislock)) < 0) {
				tst_resm(TFAIL, "Second parent lock failed");
				tst_resm(TFAIL, "Test case %d, errno = %d",
					 test + 1, errno);
				close(fd);
				unlink(tmpname);
				return 1;
			}
		}

		/* Initialize first child lock structure */
		thislock = &thiscase->child_a;

		/* Initialize child counter and flags */
		alarm_flag = parent_flag = 0;
		child_flag1 = child_flag2 = 0;
		child_count = 0;

		/* spawn child processes */
		for (i = 0; i < 2; i++) {
			if (thislock->l_type != IGNORED) {
				if ((child = FORK_OR_VFORK()) == 0) {
#ifdef UCLINUX
					if (self_exec(argv0, "ddddd", i, parent,
						      test, thislock, fd) < 0) {
						perror("self_exec failed");
						return 1;
					}
#else
					dochild(i);
#endif
				}
				if (child < 0) {
					perror("Fork failed");
					return 1;
				}
				child_count++;
				child_pid[i] = child;
				flag[i] = thislock->l_pid;
			}
			/* Initialize second child lock structure */
			thislock = &thiscase->child_b;
		}
		/* parent process */

		/*
		 * Wait for children to signal they are ready. Set a timeout
		 * just in case they don't signal at all.
		 */
		alarm(TIME_OUT);

		while (!alarm_flag
		       && (child_flag1 + child_flag2 != child_count)) {
			pause();
		}

		/*
		 * Turn off alarm and unmask signals
		 */
		alarm((unsigned)0);

		if (child_flag1 + child_flag2 != child_count) {
			tst_resm(TFAIL, "Test case %d: kids didn't signal",
				 test + 1);
			fail = 1;
		}
		child_flag1 = child_flag2 = alarm_flag = 0;

		thislock = &thiscase->parent_c;

		/* set the third parent lock on the file */
		if ((fcntl(fd, F_SETLK, thislock)) < 0) {
			tst_resm(TFAIL, "Third parent lock failed");
			tst_resm(TFAIL, "Test case %d, errno = %d",
				 test + 1, errno);
			close(fd);
			unlink(tmpname);
			return 1;
		}

		/* Initialize fourth parent lock structure */
		thislock = &thiscase->parent_d;

		if ((thislock->l_type) != IGNORED) {	/*SKIPVAL */
			/* set the fourth parent lock */
			if ((fcntl(fd, F_SETLK, thislock)) < 0) {
				tst_resm(TINFO, "Fourth parent lock failed");
				tst_resm(TINFO, "Test case %d, errno = %d",
					 test + 1, errno);
				close(fd);
				unlink(tmpname);
				return 1;
			}
		}

		/*
		 * Wait for children to exit, or for timeout to occur.
		 * Timeouts are expected for testcases where kids are
		 * 'WILLBLOCK', In that case, send kids a wakeup interrupt
		 * and wait again for them. If a second timeout occurs, then
		 * something is wrong.
		 */
		alarm_flag = nexited = 0;
		while (nexited < child_count) {
			alarm(TIME_OUT);
			child = wait(&status);
			alarm(0);

			if (child == -1) {
				if (errno != EINTR || alarm_flag != 1) {
					/*
					 * Some error other than a timeout,
					 * or else this is the second
					 * timeout. Both cases are errors.
					 */
					break;
				}

				/*
				 * Expected timeout case. Signal kids then
				 * go back and wait again
				 */
				child_sig(SIGUSR1, child_count);
				continue;
			}

			for (i = 0; i < child_count; i++)
				if (child == child_pid[i])
					break;
			if (i == child_count) {
				/*
				 * Ignore unexpected kid, it could be a
				 * leftover from a previous iteration that
				 * timed out.
				 */
				continue;
			}

			/* Found the right kid, check his status */
			nexited++;

			expect_stat = (flag[i] == NOBLOCK) ? 0 : 1;

			if (!WIFEXITED(status)
			    || WEXITSTATUS(status) != expect_stat) {
				/* got unexpected exit status from kid */
				tst_resm(TFAIL, "Test case %d: child %d %s "
					 "or got bad status (x%x)", test + 1,
					 i, (flag[i] == NOBLOCK) ?
					 "BLOCKED unexpectedly" :
					 "failed to BLOCK", status);
				fail = 1;
			}
		}

		if (nexited != child_count) {
			tst_resm(TFAIL, "Test case %d, caught %d expected %d "
				 "children", test + 1, nexited, child_count);
			child_sig(SIGKILL, nexited);
			fail = 1;
		}
		close(fd);
	}
	unlink(tmpname);
	if (fail) {
		return 1;
	} else {
		return 0;
	}
	return 0;
}

int main(int ac, char **av)
{

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(dochild_uc, "ddddd", &kid_uc, &parent, &test,
			&thislock, &fd);
	argv0 = av[0];
#endif

	setup();		/* global setup */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

/* //block1: */
		/*
		 * Check file locks on an ordinary file without
		 * mandatory locking
		 */
		tst_resm(TINFO, "Entering block 1");
		if (run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, 0, 11)) {
			tst_resm(TINFO, "Test case 1: without mandatory "
				 "locking FAILED");
		} else {
			tst_resm(TINFO, "Test case 1: without manadatory "
				 "locking PASSED");
		}
		tst_resm(TINFO, "Exiting block 1");

/* //block2: */
		/*
		 * Check the file locks on a file with mandatory record
		 * locking
		 */
		tst_resm(TINFO, "Entering block 2");
		if (NO_NFS && run_test(O_CREAT | O_RDWR | O_TRUNC, S_ISGID |
			     S_IRUSR | S_IWUSR, 0, 11)) {
			tst_resm(TINFO, "Test case 2: with mandatory record "
				 "locking FAILED");
		} else {
			if (NO_NFS)
				tst_resm(TINFO, "Test case 2: with mandatory"
					 " record locking PASSED");
			else
				tst_resm(TCONF, "Test case 2: NFS does not"
					 " support mandatory locking");
		}
		tst_resm(TINFO, "Exiting block 2");

/* //block3: */
		/*
		 * Check file locks on a file with mandatory record locking
		 * and no delay
		 */
		tst_resm(TINFO, "Entering block 3");
		if (NO_NFS && run_test(O_CREAT | O_RDWR | O_TRUNC | O_NDELAY,
			     S_ISGID | S_IRUSR | S_IWUSR, 0, 11)) {
			tst_resm(TINFO, "Test case 3: mandatory locking with "
				 "NODELAY FAILED");
		} else {
			if (NO_NFS)
				tst_resm(TINFO, "Test case 3: mandatory"
					 " locking with NODELAY PASSED");
			else
				tst_resm(TCONF, "Test case 3: NFS does not"
					 " support mandatory locking");
		}
		tst_resm(TINFO, "Exiting block 3");
	}
	cleanup();
	tst_exit();
}
