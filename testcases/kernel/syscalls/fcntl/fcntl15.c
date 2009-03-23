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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	fcntl15.c
 *
 * DESCRIPTION
 *	Check that file locks are removed when file closed
 *
 * ALGORITHM
 *	Use three testcases to check removal of locks when a file is closed.
 *
 *	Case 1: Parent opens a file and duplicates it, places locks using
 *	both file descriptors then closes one descriptor, all locks should
 *	be removed.
 *
 *	Case 2: Open same file twice using(open), place locks using both
 *	descriptors then close on descriptor, locks on the file should be
 *	lost
 *
 *	Case 3: Open file twice, one by each process, set the locks and have
 *	a child check the locks. Remove the first file and have the child
 *	check the locks. Remove the first file and have child check locks
 *	again. Only locks set on first file should have been removed
 *
 * USAGE
 *	fcntl15
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 * MODIFIED: - mridge@us.ibm.com -- changed getpid to syscall(get thread ID) for unique ID on NPTL threading
 *
 * RESTRICTIONS
 *	None
 */

#include <signal.h>
#include <fcntl.h>
#include <test.h>
#include <usctest.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#define	DATA	"ABCDEFGHIJ"
#define	DUP	0
#define	OPEN	1
#define	FORK_	2

char *TCID = "fcntl15";
int TST_TOTAL = 1;
extern int Tst_count;

static int parent, child1, child2, status;
static volatile sig_atomic_t parent_flag, child_flag, alarm_flag;
static char tmpname[40];
struct flock flock;

#ifdef UCLINUX
static char *argv0;		/* set by main, passed to self_exec */
#endif

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

void alarm_sig()
{
	signal(SIGALRM, (void (*)())alarm_sig);
	alarm_flag = 1;
	if ((syscall(__NR_gettid)) == parent) {
		tst_resm(TINFO, "Alarm caught by parent");
	} else {
		tst_resm(TINFO, "Alarm caught by child");
	}
}

void child_sig()
{
	signal(SIGUSR1, (void (*)())child_sig);
	child_flag++;
}

void parent_sig()
{
	signal(SIGUSR2, (void (*)())parent_sig);
	parent_flag++;
}

int dochild1(int file_flag, int file_mode)
{
	int fd_B;
	sigset_t newmask, zeromask, oldmask;

	if ((fd_B = open(tmpname, file_flag, file_mode)) < 0) {
		perror("open on child1 file failed");
		exit(1);
	}

	/* initialize lock structure for second 5 bytes of file */
	flock.l_type = F_WRLCK;
	flock.l_whence = 0;
	flock.l_start = 5L;
	flock.l_len = 5L;

	/* set lock on child file descriptor */
	if ((fcntl(fd_B, F_SETLK, &flock)) < 0) {
		perror("child lock failed should have succeeded");
		exit(1);
	}

	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);
	sigaddset(&newmask, SIGALRM);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
		perror("child1 sigprocmask SIG_BLOCK fail");
		exit(1);
	}
	/*
	 * send signal to parent here to tell parent we have locked the
	 * file, thus allowing parent to proceed
	 */
	if ((kill(parent, SIGUSR1)) < 0) {
		perror("child1 signal to parent failed");
		exit(1);
	}

	/*
	 * set alarm to break pause if parent fails to signal then spin till
	 * parent ready
	 */
	alarm(60);
	while (parent_flag == 0 && alarm_flag == 0)
		sigsuspend(&zeromask);
	alarm((unsigned)0);
	if (parent_flag != 1) {
		perror("pause in child1 terminated without "
		       "SIGUSR2 signal from parent");
		exit(1);
	}
	parent_flag = 0;
	alarm_flag = 0;
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		perror("child1 sigprocmask SIG_SETMASK fail");
		exit(1);
	}

	/* wait for child2 to complete then cleanup */
	sleep(10);
	close(fd_B);
	exit(0);
}

#ifdef UCLINUX
int uc_file_flag, uc_file_mode;

void dochild1_uc()
{
	dochild1(uc_file_flag, uc_file_mode);
}
#endif

int dofork(int file_flag, int file_mode)
{
	/* create child process */
	if ((child1 = FORK_OR_VFORK()) < 0) {
		perror("Fork failure");
		return 1;
	}

	/* child1 */
	if (child1 == 0) {
#ifdef UCLINUX
		if (self_exec(argv0, "nddds", 1, file_flag, file_mode,
			      parent, tmpname) < 0) {
			perror("self_exec failure");
			return 1;
		}
#else
		dochild1(file_flag, file_mode);
#endif
	} else {
		/*
		 * need to wait for child1 to open, and lock the area of the
		 * file prior to continuing on from here
		 */
		sigset_t newmask, zeromask, oldmask;
		sigemptyset(&zeromask);
		sigemptyset(&newmask);
		sigaddset(&newmask, SIGUSR1);
		sigaddset(&newmask, SIGUSR2);
		sigaddset(&newmask, SIGALRM);
		if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
			perror("parent sigprocmask SIG_BLOCK fail");
			exit(1);
		}

		/*
		 * set alarm to break pause if parent fails to signal then spin till
		 * parent ready
		 */
		alarm(60);
		while (child_flag == 0 && alarm_flag == 0)
			sigsuspend(&zeromask);
		alarm((unsigned)0);
		if (child_flag != 1) {
			perror("parent paused without SIGUSR1 " "from child");
			exit(1);
		}
		child_flag = 0;
		alarm_flag = 0;
		if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
			perror("parent sigprocmask SIG_SETMASK fail");
			exit(1);
		}
	}
	return 0;
}

int dochild2(int file_flag, int file_mode, int dup_flag)
{
	int fd_C;
	sigset_t newmask, zeromask, oldmask;

	if ((fd_C = open(tmpname, file_flag, file_mode)) < 0) {
		perror("open on child2 file failed");
		exit(1);
	}

	/* initialize lock structure for first 5 bytes of file */
	flock.l_type = F_WRLCK;
	flock.l_whence = 0;
	flock.l_start = 0L;
	flock.l_len = 5L;

	/* Set lock on child file descriptor */
	if ((fcntl(fd_C, F_SETLK, &flock)) >= 0) {
		tst_resm(TFAIL, "First child2 lock succeeded should "
			 "have failed");
		exit(1);
	}

	/* initialize lock structure for second 5 bytes of file */
	flock.l_type = F_WRLCK;
	flock.l_whence = 0;
	flock.l_start = 5L;
	flock.l_len = 5L;

	/* set lock on child file descriptor */
	if ((fcntl(fd_C, F_SETLK, &flock)) >= 0) {
		tst_resm(TFAIL, "second child2 lock succeeded should have "
			 "failed");
		exit(1);
	}

	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);
	sigaddset(&newmask, SIGALRM);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
		perror("child2 sigprocmask SIG_BLOCK fail");
		exit(1);
	}
	/*
	 * send signal to parent here to tell parent we have locked the
	 * file, thus allowing parent to proceed
	 */
	if ((kill(parent, SIGUSR1)) < 0) {
		perror("child2 signal to parent failed");
		exit(1);
	}

	/*
	 * set alarm to break pause if parent fails to signal then spin till
	 * parent ready
	 */
	alarm(60);
	while (parent_flag == 0 && alarm_flag == 0)
		sigsuspend(&zeromask);
	alarm((unsigned)0);
	if (parent_flag != 1) {
		perror("pause in child2 terminated without "
		       "SIGUSR2 signal from parent");
		exit(1);
	}
	parent_flag = 0;
	alarm_flag = 0;
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		perror("child2 sigprocmask SIG_SETMASK fail");
		exit(1);
	}

	/* initialize lock structure for first 5 bytes of file */
	flock.l_type = F_WRLCK;
	flock.l_whence = 0;
	flock.l_start = 0L;
	flock.l_len = 5L;

	/* set lock on child file descriptor */
	if ((fcntl(fd_C, F_SETLK, &flock)) < 0) {
		tst_resm(TFAIL, "third child2 lock failed should have "
			 "succeeded");
		exit(1);
	}

	/* Initialize lock structure for second 5 bytes of file */
	flock.l_type = F_WRLCK;
	flock.l_whence = 0;
	flock.l_start = 5L;
	flock.l_len = 5L;

	/* set lock on child file descriptor */
	if (dup_flag == FORK_) {
		if ((fcntl(fd_C, F_SETLK, &flock)) >= 0) {
			tst_resm(TFAIL, "fourth child2 lock succeeded "
				 "should have failed");
			exit(1);
		}
	} else {
		if ((fcntl(fd_C, F_SETLK, &flock)) < 0) {
			tst_resm(TFAIL, "fourth child2 lock failed "
				 "should have succeeded");
			exit(1);
		}
	}
	close(fd_C);
	exit(0);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

int run_test(int file_flag, int file_mode, int dup_flag)
{
	int fd_A, fd_B;
	sigset_t newmask, zeromask, oldmask;

	/* setup to catch SIGUSR1 signal from child process */
	if ((signal(SIGUSR1, child_sig)) == SIG_ERR) {
		perror("Signal setup for SIGUSR1 failed");
	}

	/* setup to catch SIGUSR2 signal from parent */
	if ((signal(SIGUSR2, parent_sig)) == SIG_ERR) {
		perror("Signal setup for SIGUSR1 failed");
	}

	parent = syscall(__NR_gettid);

	tst_tmpdir();
	/* setup temporary file name */
	sprintf(tmpname, "fcntl15.%d", parent);

	/* initialize signal flags */
	child_flag = parent_flag = alarm_flag = 0;

	if ((fd_A = open(tmpname, file_flag, file_mode)) < 0) {
		perror("open first parent file failed");
		tst_rmdir();
		return 1;
	}

	/* write some data to the file */
	(void)write(fd_A, DATA, 10);

	if (dup_flag) {
		if (dup_flag == FORK_) {
			dofork(file_flag, file_mode);
		} else {
			if ((fd_B = open(tmpname, file_flag, file_mode)) < 0) {
				perror("open second parent file failed");
				tst_rmdir();
				return 1;
			}
		}
	} else {
		/* create a second file descriptor from first file */
		if ((fd_B = fcntl(fd_A, F_DUPFD, 0)) < 0) {
			perror("dup of second parent file failed");
			tst_rmdir();
			return 1;
		}
	}

	/*
	 * initialize lock structure for first lock on first
	 * 5 bytes of file
	 */
	flock.l_type = F_WRLCK;
	flock.l_whence = 0;
	flock.l_start = 0L;
	flock.l_len = 5L;

	/* set lock on first file descriptor */
	if ((fcntl(fd_A, F_SETLK, &flock)) < 0) {
		perror("Attempt to set first parent lock failed");
		tst_rmdir();
		return 1;
	}

	if (dup_flag != FORK_) {
		/* initialize lock structure for last 5 bytes of file */
		flock.l_type = F_WRLCK;
		flock.l_whence = 0;
		flock.l_start = 5L;
		flock.l_len = 5L;

		/* set lock on second file descriptor */
		if ((fcntl(fd_B, F_SETLK, &flock)) < 0) {
			perror("Attempt to set second parent lock failed");
			tst_rmdir();
			return 1;
		}
	}

	/* create child process */
	if ((child2 = FORK_OR_VFORK()) < 0) {
		perror("Fork failure");
		tst_rmdir();
		return 1;
	} else if (child2 == 0) {	/* child */
		dochild2(file_flag, file_mode, dup_flag);
	}

	/* parent */

	/*
	 * Set alarm to break pause if child fails to signal then spin till
	 * child is ready
	 */

	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);
	sigaddset(&newmask, SIGALRM);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
		perror("parent sigprocmask SIG_BLOCK fail");
		exit(1);
	}

	/*
	 * set alarm to break pause if parent fails to signal then spin till
	 * parent ready
	 */
	alarm(60);
	while (child_flag == 0 && alarm_flag == 0)
		sigsuspend(&zeromask);
	alarm((unsigned)0);
	if (child_flag != 1) {
		perror("parent paused without SIGUSR1 " "from child");
		exit(1);
	}
	child_flag = 0;
	alarm_flag = 0;
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		perror("parent sigprocmask SIG_SETMASK fail");
		exit(1);
	}

	/* close the first file then signal child to test locks */
	close(fd_A);
	if ((kill(child2, SIGUSR2)) < 0) {
		perror("Signal to child2 failed");
		tst_rmdir();
		return 1;
	}

	if (dup_flag == FORK_) {
		if ((kill(child1, SIGUSR2)) < 0) {
			perror("Signal to child1 failed");
			tst_rmdir();
			return 1;
		}
	}
	/* wait for child to complete then cleanup */
	while ((wait(&status)) > 0) {
		if (status >> 8 != 0) {
			tst_resm(TFAIL, "Expected 0 got %d", status >> 8);
			tst_rmdir();
			return 1;
		}
	}
	if (dup_flag != FORK_) {
		close(fd_B);
	}
	unlink(tmpname);
	tst_rmdir();
	return 0;
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int fail = 0;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&dochild1_uc, "nddds", 1, &uc_file_flag,
			&uc_file_mode, &parent, tmpname);
	argv0 = av[0];
#endif

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* Set up to catch alarm signal */
		if ((signal(SIGALRM, alarm_sig)) == SIG_ERR) {
			perror("SIGALRM signal set up failed");
			exit(1);
		}

/* //block1: */
		tst_resm(TINFO, "Entering block 1");
		if (run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, DUP)) {
			tst_resm(TINFO, "Test 1: test with \"dup\" FAILED");
			fail = 1;
		} else {
			tst_resm(TINFO, "Test 1: test with \"dup\" PASSED");
		}
		tst_resm(TINFO, "Exiting block 1");

/* //block2: */
		tst_resm(TINFO, "Entering block 2");
		if (run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, OPEN)) {
			tst_resm(TINFO, "Test 2: test with \"open\" FAILED");
			fail = 1;
		} else {
			tst_resm(TINFO, "Test 2: test with \"open\" PASSED");
		}
		tst_resm(TINFO, "Exiting block 2");

/* //block3: */
		tst_resm(TINFO, "Entering block 3");
		if (run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, FORK_)) {
			tst_resm(TINFO, "Test 3: test with \"fork\" FAILED");
			fail = 1;
		} else {
			tst_resm(TINFO, "Test 3: test with \"fork\" PASSED");
		}
		tst_resm(TINFO, "Exiting block 3");
	}
	cleanup();
	return 0;
}
