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
 * 	fcntl17.c
 *
 * DESCRIPTION
 * 	Check deadlock detection for file locking
 *
 * ALGORITHM
 * 	The parent forks off 3 children. The parent controls the children
 * 	with messages via pipes to create a delayed deadlock between the
 * 	second and third child.
 *
 * USAGE
 * 	fcntl17
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      04/2002 Minor fixes by William Jay Huie (testcase name 
		fcntl05 => fcntl17, check signal return for SIG_ERR)
 *
 * RESTRICTIONS
 *	None
 */

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <test.h>
#include <usctest.h>

char *TCID = "fcntl17";
int TST_TOTAL = 1;
extern int Tst_count;

#define STRINGSIZE	27
#define STRING		"abcdefghijklmnopqrstuvwxyz\n"
#define STOP		0xFFF0
#define TIME_OUT	10

/* global variables */
int parent_pipe[2];
int child_pipe1[2];
int child_pipe2[2];
int child_pipe3[2];
int file_fd;
short parent_pid, child_pid1, child_pid2, child_pid3;
char *file;
struct flock lock1 = { (short)F_WRLCK, (short)0,  2,  5, (short)0 };
struct flock lock2 = { (short)F_WRLCK, (short)0,  9,  5, (short)0 };
struct flock lock3 = { (short)F_WRLCK, (short)0, 17,  5, (short)0 };
struct flock lock4 = { (short)F_WRLCK, (short)0, 17,  5, (short)0 };
struct flock lock5 = { (short)F_WRLCK, (short)0,  2, 14, (short)0 };
struct flock unlock = { (short)F_UNLCK, (short)0,  0,  0, (short)0 };

/* prototype declarations */
int setup();
void cleanup();
int parent_wait();
void parent_free();
void child_wait();
void child_free();
void do_child1();
void do_child2();
void do_child3();
int do_test(struct flock *, short);
void stop_children();
void catch_child();
void catch_alarm();
char *str_type();

main(int ac, char **av)
{
	int ans;
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int fail = 0;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	if (setup()) {			/* global testup */
		tst_resm(TINFO, "setup failed");
		cleanup();
		/*NOTREACHED*/
	}

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

preparation:
		tst_resm(TINFO, "Enter preparation phase");
		if ((child_pid1 = fork()) == 0) {	/* first child */
			do_child1();
		} else if (child_pid1 < 0) {
			perror("Fork failed: child 1");
			cleanup();
			/*NOTREACHED*/
		}

		/* parent */

		if ((child_pid2 = fork()) == 0) {	/* second child */
			do_child2();
		} else if (child_pid2 < 0) {
			perror("Fork failed: child 2");
			if ((kill(child_pid1, SIGKILL)) < 0) {
				tst_resm(TFAIL, "Attempt to signal child "
					 "1 failed");
			}
			cleanup();
			/*NOTREACHED*/
		}

		/* parent */

		if ((child_pid3 = fork()) == 0) {	/* third child */
			do_child3();
		} else if (child_pid3 < 0) {
			perror("Fork failed: child 3");
			if ((kill(child_pid1, SIGKILL)) < 0) {
				tst_resm(TFAIL, "Attempt to signal child "
					 "1 failed");
			}
			if ((kill(child_pid2, SIGKILL)) < 0) {
				tst_resm(TFAIL, "Attempt to signal child 2 "
					 "failed");
			}
			cleanup();
			/*NOTREACHED*/
		}
		/* parent */

		close(parent_pipe[1]);
		close(child_pipe1[0]);
		close(child_pipe2[0]);
		close(child_pipe3[0]);
		tst_resm(TINFO, "Exit preparation phase");

block1:
		tst_resm(TINFO, "Enter block 1");
		fail = 0;
		/*
		 * child 1 puts first lock (bytes 2-7)
		 */
		child_free(child_pipe1[1], 0);
		if (parent_wait()) {
			tst_resm(TFAIL, "didn't set first child's lock, "
				 "errno: %d", errno);
		}
		if (do_test(&lock1, child_pid1)) {
			tst_resm(TINFO, "do_test failed child 1");
			fail = 1;
		}

		/*
		 * child 2 puts second lock (bytes 9-14)
		 */
		child_free(child_pipe2[1], 0);
		if (parent_wait()) {
			tst_resm(TINFO, "didn't set second child's lock, "
				 "errno: %d", errno);
			fail = 1;
		}
		if (do_test(&lock2, child_pid2)) {
			tst_resm(TINFO, "do_test failed child 2");
			fail = 1;
		}

		/*
		 * child 3 puts third lock (bytes 17-22)
		 */
		child_free(child_pipe3[1], 0);
		if (parent_wait()) {
			tst_resm(TFAIL, "didn't set third child's lock, "
				 "errno: %d", errno);
			fail = 1;
		}
		if (do_test(&lock3, child_pid3)) {
			tst_resm(TINFO, "do_test failed child 3");
			fail = 1;
		}

		/*
		 * child 2 tries to lock same range as
		 * child 3's first lock.
		 */
		child_free(child_pipe2[1], 0);

		/*
		 * child 3 tries to lock same range as
		 * child 1 and child 2's first locks.
		 */
		child_free(child_pipe3[1], 0);

		/*
		 * Tell child 1 to release its lock. This should cause a
		 * delayed deadlock between child 2 and child 3.
		 */
		child_free(child_pipe1[1], 0);

		/*
		 * Setup an alarm to go off in case the deadlock is not
		 * detected
		 */
		alarm(TIME_OUT);

		/*
		 * should get a message from child 3 telling that its
		 * second lock EDEADLOCK
		 */
		if ((ans = parent_wait()) != EDEADLK) {
			tst_resm(TFAIL, "child 2 didn't deadlock, "
				 "returned: %d", ans);
			fail = 1;
		}

		/*
		 * Double check that lock 2 and lock 3 are still right
		 */
		do_test(&lock2, child_pid2);
		do_test(&lock3, child_pid3);

		stop_children();
		close(file_fd);
		if (fail) {
			tst_resm(TINFO, "Block 1 FAILED");
		} else {
			tst_resm(TINFO, "Block 1 PASSED");
		}
		tst_resm(TINFO, "Exit block 1");
	}
	cleanup();
}

setup()
{
	char *buf = STRING;

	tst_sig(FORK, DEF_HANDLER, NULL);	/* capture signals */
	umask(0);
	TEST_PAUSE;			/* Pause if that option is specified */
	tst_tmpdir();			/* make temp dir and cd to it */

	pipe(parent_pipe);
	pipe(child_pipe1);
	pipe(child_pipe2);
	pipe(child_pipe3);
	parent_pid = getpid();
	file = tempnam(".", NULL);

	if ((file_fd = open(file, O_RDWR|O_CREAT, 0777)) < 0) {
		tst_resm(TFAIL, "Couldn't open %s! errno = %d", file, errno);
		return(1);	
	}

	if (write(file_fd, buf, STRINGSIZE) < 0) {
		tst_resm(TFAIL, "Couldn't write %s! errno = %d", file, errno);
		return(1);
	}

	if ((int)(signal(SIGALRM, catch_alarm)) == SIG_ERR) {
		tst_resm(TFAIL, "SIGALRM signal setup failed, errno: %d",
			 errno);
		return(1);
	}

	if ((int)(signal(SIGCLD, catch_child)) == SIG_ERR) {
		tst_resm(TFAIL, "SIGCLD signal setup failed, errno: %d",
			 errno);
		return(1);
	}
	return(0);
}

void
cleanup()
{
	unlink(file);
	tst_rmdir();
	tst_exit();
}

void
do_child1()
{
	struct flock fl;

	close(parent_pipe[0]);
	close(child_pipe1[1]);
	close(child_pipe2[0]);
	close(child_pipe2[1]);
	close(child_pipe3[0]);
	close(child_pipe3[1]);

	child_wait(child_pipe1[0]);
	if (fcntl(file_fd, F_SETLK, &lock1) < 0) {
		parent_free(errno);
	} else {
		parent_free(0);
	}

	child_wait(child_pipe1[0]);
	fcntl(file_fd, F_SETLK, &unlock);

	child_wait(child_pipe1[0]);
	exit(1);
}

void
do_child2()
{
	struct flock fl;

	close(parent_pipe[0]);
	close(child_pipe1[0]);
	close(child_pipe1[1]);
	close(child_pipe2[1]);
	close(child_pipe3[0]);
	close(child_pipe3[1]);

	child_wait(child_pipe2[0]);
	if (fcntl(file_fd, F_SETLK, &lock2) < 0) {
		parent_free(errno);
	} else {
		parent_free(0);
	}

	child_wait(child_pipe2[0]);
	if (fcntl(file_fd, F_SETLKW, &lock4) < 0) {
		parent_free(errno);
	} else {
		parent_free(0);
	}

	child_wait(child_pipe2[0]);
	exit(1);
}

void
do_child3()
{
	close(parent_pipe[0]);
	close(child_pipe1[0]);
	close(child_pipe1[1]);
	close(child_pipe2[0]);
	close(child_pipe2[1]);
	close(child_pipe3[1]);

	child_wait(child_pipe3[0]);
	if (fcntl(file_fd, F_SETLK, &lock3) < 0) {
		parent_free(errno);
	} else {
		parent_free(0);
	}

	child_wait(child_pipe3[0]);
	if (fcntl(file_fd, F_SETLKW, &lock5) < 0) {
		parent_free(errno);
	} else {
		parent_free(0);
	}

	child_wait(child_pipe3[0]);
	exit(1);
}

int
do_test(struct flock *lock, short pid)
{
	struct flock fl;

	fl.l_type = lock->l_type;
	fl.l_whence = lock->l_whence;
	fl.l_start = lock->l_start;
	fl.l_len = lock->l_len;
	fl.l_pid = (short) 0;
	if (fcntl(file_fd, F_GETLK, &fl) < 0) {
		tst_resm(TFAIL, "fcntl on file %s failed, errno =%d", file,
			 errno);
		return(1);
	}

	if (fl.l_type != lock->l_type) {
		tst_resm(TFAIL, "lock type is wrong should be %s is %s",
			 str_type(lock->l_type), str_type(fl.l_type));
		return(1);
	}

	if (fl.l_whence != lock->l_whence) {
		tst_resm(TFAIL, "lock whence is wrong should be %d is %d",
			 lock->l_whence, fl.l_whence);
		return(1);
	}

	if (fl.l_start != lock->l_start) {
		tst_resm(TFAIL, "region starts in wrong place, "
			 "should be %d is %d", lock->l_start, fl.l_start);
		return(1);
	}

	if (fl.l_len != lock->l_len) {
		tst_resm(TFAIL, "region length is wrong, should be %d is %d",
			 lock->l_len, fl.l_len);
		return(1);
	}

	if (fl.l_pid != pid) {
		tst_resm(TFAIL, "locking pid is wrong, should be %d is %d",
			 pid, fl.l_pid);
		return(1);
	}
	return(0);
}

char *
str_type(int type)
{
	static char buf[20];

	switch (type) {
	case 1:
		return("F_RDLCK");
	case 2:
		return("F_WRLCK");
	case 3:
		return("F_UNLCK");
	default:
		sprintf(buf, "BAD VALUE: %d", type);
		return(buf);
	}
}

void
parent_free(int arg)
{
	if (write(parent_pipe[1], &arg, sizeof(arg)) != sizeof(arg)) {
		tst_resm(TFAIL, "couldn't send message to parent");
		exit(1);
	}
}

parent_wait()
{
	int arg;

	if (read(parent_pipe[0], &arg, sizeof(arg)) != sizeof(arg)) {
		tst_resm(TFAIL, "parent_wait() failed");
		return(errno);
	}
	return(arg);
}

void
child_free(int fd, int arg)
{
	if (write(fd, &arg, sizeof(arg)) != sizeof(arg)) {
		tst_resm(TFAIL, "couldn't send message to child");
		exit(1);
	}
}

void
child_wait(int fd)
{
	int arg;

	if (read(fd, &arg, sizeof(arg)) != sizeof(arg)) {
		tst_resm(TFAIL, "couldn't get message from parent");
		exit(1);
	} else if (arg == (short)STOP) {
		exit(0);
	}
}

void
stop_children()
{
	int arg;

	(void) signal(SIGCLD, (void (*)())SIG_DFL);
	arg = STOP;
	child_free(child_pipe1[1], arg);
	child_free(child_pipe2[1], arg);
	child_free(child_pipe3[1], arg);
	wait(0);
}

void
catch_child()
{
	tst_resm(TFAIL, "Unexpected death of child process");
	cleanup();
	/*NOTREACHED*/
}

void
catch_alarm()
{
	/*
	 * Timer has runout and the children have not detected the deadlock.
	 * Need to kill the kids and exit
	 */
	if ((kill(child_pid1, SIGKILL)) < 0) {
		tst_resm(TFAIL, "Attempt to signal child 1 failed.");
	}

	if ((kill(child_pid2, SIGKILL)) < 0) {
		tst_resm(TFAIL, "Attempt to signal child 2 failed.");
	}
	cleanup();
	/*NOTREACHED*/
}
