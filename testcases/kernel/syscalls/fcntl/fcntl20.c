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
 *	fcntl20.c
 *
 * DESCRIPTION
 *	Check locking of regions of a file
 *
 * ALGORITHM
 *	Test unlocking sections around a read lock
 *
 * USAGE
 *	fcntl20
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <inttypes.h>
#include "test.h"
#include "safe_macros.h"

#define STRINGSIZE	27
#define STRING		"abcdefghijklmnopqrstuvwxyz\n"
#define STOP		0xFFF0

int parent_pipe[2];
int child_pipe[2];
int fd;
pid_t parent_pid, child_pid;

void parent_put();
void parent_get();
void child_put();
void child_get();
void stop_child();
void compare_lock(struct flock *, short, short, int, int, pid_t);
void unlock_file();
void do_test(struct flock *, short, short, int, int);
void catch_child();
char *str_type();
int do_lock(int, short, short, int, int);

char *TCID = "fcntl20";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

int fail = 0;

/*
 * setup
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	char *buf = STRING;
	char template[PATH_MAX];
	struct sigaction act;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	parent_pid = getpid();

	SAFE_PIPE(NULL, parent_pipe);
	SAFE_PIPE(NULL, child_pipe);

	tst_tmpdir();

	snprintf(template, PATH_MAX, "fcntl20XXXXXX");

	if ((fd = mkstemp(template)) == -1)
		tst_resm(TFAIL | TERRNO, "mkstemp failed");

	SAFE_WRITE(cleanup, SAFE_WRITE_ANY, fd, buf, STRINGSIZE);

	memset(&act, 0, sizeof(act));
	act.sa_handler = catch_child;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGCHLD);
	if (sigaction(SIGCHLD, &act, NULL) == -1)
		tst_brkm(TFAIL | TERRNO, cleanup, "SIGCHLD signal setup failed");
}

void cleanup(void)
{
	SAFE_CLOSE(NULL, fd);

	tst_rmdir();

}

void do_child(void)
{
	struct flock fl;

	close(parent_pipe[1]);
	close(child_pipe[0]);
	while (1) {
		child_get(&fl);
		if (fcntl(fd, F_GETLK, &fl) < 0) {
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");
			fail = 1;
		}
		child_put(&fl);
	}
}

int do_lock(int cmd, short type, short whence, int start, int len)
{
	struct flock fl;

	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;
	return (fcntl(fd, cmd, &fl));
}

void do_test(struct flock *fl, short type, short whence, int start, int len)
{
	fl->l_type = type;
	fl->l_whence = whence;
	fl->l_start = start;
	fl->l_len = len;
	fl->l_pid = (short)0;

	parent_put(fl);
	parent_get(fl);
}

void
compare_lock(struct flock *fl, short type, short whence, int start, int len,
	     pid_t pid)
{
	if (fl->l_type != type) {
		tst_resm(TFAIL, "lock type is wrong should be %s is %s",
			 str_type(type), str_type(fl->l_type));
		fail = 1;
	}

	if (fl->l_whence != whence) {
		tst_resm(TFAIL, "lock whence is wrong should be %d is %d",
			 whence, fl->l_whence);
		fail = 1;
	}

	if (fl->l_start != start) {
		tst_resm(TFAIL, "region starts in wrong place, should be"
			 "%d is %" PRId64, start, (int64_t) fl->l_start);
		fail = 1;
	}

	if (fl->l_len != len) {
		tst_resm(TFAIL,
			 "region length is wrong, should be %d is %" PRId64,
			 len, (int64_t) fl->l_len);
		fail = 1;
	}

	if (fl->l_pid != pid) {
		tst_resm(TFAIL, "locking pid is wrong, should be %d is %d",
			 pid, fl->l_pid);
		fail = 1;
	}
}

void unlock_file(void)
{
	struct flock fl;

	if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 0, 0) < 0) {
		tst_resm(TFAIL, "fcntl on file failed, errno =%d", errno);
		fail = 1;
	}
	do_test(&fl, F_WRLCK, 0, 0, 0);
	compare_lock(&fl, (short)F_UNLCK, (short)0, 0, 0, (pid_t) 0);
}

char *str_type(int type)
{
	static char buf[20];

	switch (type) {
	case 1:
		return ("F_RDLCK");
	case 2:
		return ("F_WRLCK");
	case 3:
		return ("F_UNLCK");
	default:
		sprintf(buf, "BAD VALUE: %d", type);
		return (buf);
	}
}

void parent_put(struct flock *l)
{
	if (write(parent_pipe[1], l, sizeof(*l)) != sizeof(*l)) {
		tst_resm(TFAIL, "couldn't send message to child");
		fail = 1;
	}
}

void parent_get(struct flock *l)
{
	if (read(child_pipe[0], l, sizeof(*l)) != sizeof(*l)) {
		tst_resm(TFAIL, "couldn't get message from child");
		fail = 1;
	}
}

void child_put(struct flock *l)
{
	if (write(child_pipe[1], l, sizeof(*l)) != sizeof(*l)) {
		tst_resm(TFAIL, "couldn't send message to parent");
		fail = 1;
	}
}

void child_get(struct flock *l)
{
	if (read(parent_pipe[0], l, sizeof(*l)) != sizeof(*l)) {
		tst_resm(TFAIL, "couldn't get message from parent");
		cleanup();
	} else if (l->l_type == (short)STOP) {
		exit(0);
	}
}

void stop_child(void)
{
	struct flock fl;

	signal(SIGCHLD, SIG_DFL);
	fl.l_type = STOP;
	parent_put(&fl);
	wait(0);
}

void catch_child(void)
{
	tst_resm(TFAIL, "Unexpected death of child process");
	cleanup();
}

int main(int ac, char **av)
{
	struct flock tl;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	maybe_run_child(&do_child, "ddddd", &parent_pipe[0], &parent_pipe[1],
			&child_pipe[0], &child_pipe[1], &fd);
#endif

	setup();		/* global setup */

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		if ((child_pid = FORK_OR_VFORK()) == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec
			    (av[0], "ddddd", parent_pipe[0], parent_pipe[1],
			     child_pipe[0], child_pipe[1], fd) < 0) {
				tst_resm(TFAIL, "self_exec failed");
				cleanup();
			}
#else
			do_child();
#endif
		}

		if (child_pid < 0) {
			tst_resm(TFAIL, "Fork failed");
			cleanup();
		}

		(void)close(parent_pipe[0]);
		(void)close(child_pipe[1]);

/* //block1: */
		tst_resm(TINFO, "Enter block 1");
		/*
		 * Add a read lock to the middle of the file and unlock a
		 * section just before the lock
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 5, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 10, 5, parent_pid);

		/*
		 * Test that the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail) {
			tst_resm(TINFO, "Test block 1: FAILED");
		} else {
			tst_resm(TINFO, "Test block 1: PASSED");
		}
		tst_resm(TINFO, "Exit block 1");

/* //block2: */
		tst_resm(TINFO, "Enter block 2");
		fail = 0;
		/*
		 * Set a read lock in the middle and do an unlock that
		 * ends at the first byte of the read lock.
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 5, 6) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 11, 4, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail)
			tst_resm(TINFO, "Test block 2: FAILED");
		else
			tst_resm(TINFO, "Test block 2: PASSED");
		tst_resm(TINFO, "Exit block 2");

/* //block3: */
		tst_resm(TINFO, "Enter block 3");
		fail = 0;

		/*
		 * Set a read lock on the middle of the file and do an
		 * unlock that overlaps the front of the read
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 5, 8) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 13, 2, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail)
			tst_resm(TINFO, "Test block 3: FAILED");
		else
			tst_resm(TINFO, "Test block 3: PASSED");
		tst_resm(TINFO, "Exit block 3");

/* //block4: */
		tst_resm(TINFO, "Enter blcok 4");
		fail = 0;

		/*
		 * Set a read lock in the middle of a file and unlock a
		 * section in the middle of it
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 10) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 13, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test the first read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 10, 3, parent_pid);

		/*
		 * Test the second read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 13, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 18, 2, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 20, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 20, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail)
			tst_resm(TINFO, "Test block 4: FAILED");
		else
			tst_resm(TINFO, "Test block 4: PASSED");
		tst_resm(TINFO, "Exit block 4");

/* //block5: */
		tst_resm(TINFO, "Enter block 5");
		fail = 0;

		/*
		 * Set a read lock in the middle of the file and do a
		 * unlock that overlaps the end
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 13, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 10, 3, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 13, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 13, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail)
			tst_resm(TINFO, "Test block 5: FAILED");
		else
			tst_resm(TINFO, "Test block 5: PASSED");
		tst_resm(TINFO, "Exit block 5");

/* //block6: */
		tst_resm(TINFO, "Enter block 6");
		fail = 0;

		/*
		 * Set read lock in the middle of the file and do an unlock
		 * starting at the last byte of the read lock
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 14, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 10, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 10, 4, parent_pid);

		/*
		 * Test to make sure the end of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 14, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 14, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail)
			tst_resm(TINFO, "Test block 6: FAILED");
		else
			tst_resm(TINFO, "Test block 6: PASSED");
		tst_resm(TINFO, "Exit block 6");

/* //block7: */
		tst_resm(TINFO, "Enter block 7");
		fail = 0;

		/*
		 * Set a read lock at the middle of the file and do an
		 * unlock that starts at the byte past the end of the read
		 * lock
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 10, 5) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 16, 0) < 0) {
			tst_resm(TFAIL, "fcntl on file failed, errno =%d",
				 errno);
			fail = 1;
		}

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 10, 5, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 16, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 16, 0, (pid_t) 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		if (fail)
			tst_resm(TINFO, "Test block 7: FAILED");
		else
			tst_resm(TINFO, "Test block 7: PASSED");

		tst_resm(TINFO, "Exit block 7");

		stop_child();
	}
	cleanup();
	tst_exit();
}
