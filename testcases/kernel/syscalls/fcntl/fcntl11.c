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
 *	fcntl11.c
 *
 * DESCRIPTION
 *	Testcase to check locking of regions of a file
 *
 * ALGORITHM
 *	Test changing lock sections around a write lock
 *
 * USAGE
 *	fcntl11
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
#include <sys/types.h>
#include <sys/stat.h>
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

char *TCID = "fcntl11";
int TST_TOTAL = 1;

int fail;

void cleanup(void)
{
	tst_rmdir();

}

void setup(void)
{
	char *buf = STRING;
	char template[PATH_MAX];
	struct sigaction act;

	tst_sig(FORK, DEF_HANDLER, cleanup);
	tst_tmpdir();

	umask(0);

	TEST_PAUSE;

	SAFE_PIPE(cleanup, parent_pipe);
	SAFE_PIPE(cleanup, child_pipe);
	parent_pid = getpid();
	snprintf(template, PATH_MAX, "fcntl11XXXXXX");

	if ((fd = mkstemp(template)) < 0)
		tst_resm(TFAIL, "Couldn't open temp file! errno = %d", errno);

	SAFE_WRITE(cleanup, SAFE_WRITE_ANY, fd, buf, STRINGSIZE);

	memset(&act, 0, sizeof(act));
	act.sa_handler = catch_child;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGCHLD);
	if ((sigaction(SIGCHLD, &act, NULL)) < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "sigaction(SIGCHLD, ..) failed");
}

void do_child(void)
{
	struct flock fl;

	close(parent_pipe[1]);
	close(child_pipe[0]);
	while (1) {
		child_get(&fl);
		if (fcntl(fd, F_GETLK, &fl) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");
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
	if (fl->l_type != type)
		tst_resm(TFAIL, "lock type is wrong should be %s is %s",
			 str_type(type), str_type(fl->l_type));

	if (fl->l_whence != whence)
		tst_resm(TFAIL, "lock whence is wrong should be %d is %d",
			 whence, fl->l_whence);

	if (fl->l_start != start)
		tst_resm(TFAIL, "region starts in wrong place, should be "
			 "%d is %" PRId64, start, (int64_t) fl->l_start);

	if (fl->l_len != len)
		tst_resm(TFAIL,
			 "region length is wrong, should be %d is %" PRId64,
			 len, (int64_t) fl->l_len);

	if (fl->l_pid != pid)
		tst_resm(TFAIL, "locking pid is wrong, should be %d is %d",
			 pid, fl->l_pid);
}

void unlock_file(void)
{
	struct flock fl;

	if (do_lock(F_SETLK, (short)F_UNLCK, (short)0, 0, 0) < 0)
		tst_resm(TFAIL | TERRNO, "fcntl on file failed");
	do_test(&fl, F_WRLCK, 0, 0, 0);
	compare_lock(&fl, (short)F_UNLCK, (short)0, 0, 0, (pid_t) 0);
}

char *str_type(int type)
{
	static char buf[20];

	switch (type) {
	case F_RDLCK:
		return ("F_RDLCK");
	case F_WRLCK:
		return ("F_WRLCK");
	case F_UNLCK:
		return ("F_UNLCK");
	default:
		sprintf(buf, "BAD VALUE: %d", type);
		return (buf);
	}
}

void parent_put(struct flock *l)
{
	SAFE_WRITE(cleanup, SAFE_WRITE_ALL, parent_pipe[1], l, sizeof(*l));
}

void parent_get(struct flock *l)
{
	SAFE_READ(cleanup, 1, child_pipe[0], l, sizeof(*l));
}

void child_put(struct flock *l)
{
	SAFE_WRITE(NULL, SAFE_WRITE_ALL, child_pipe[1], l, sizeof(*l));
}

void child_get(struct flock *l)
{
	SAFE_READ(NULL, 1, parent_pipe[0], l, sizeof(*l));
	if (l->l_type == (short)STOP)
		exit(0);
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
	tst_brkm(TFAIL, cleanup, "Unexpected death of child process");
}

int main(int ac, char **av)
{
	struct flock tl;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		if ((child_pid = tst_fork()) == 0) /* parent */
			do_child();
		else if (child_pid == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "fork failed");

		SAFE_CLOSE(cleanup, parent_pipe[0]);
		SAFE_CLOSE(cleanup, child_pipe[1]);

/* //block1: */
		tst_resm(TINFO, "Enter block 1");

		/*
		 * Add a write lock to the middle of the file and a read
		 * at the begining
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 1, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test read lock
		 */
		do_test(&tl, F_WRLCK, 0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 1, 5, parent_pid);

		/*
		 * Test write lock
		 */
		do_test(&tl, F_WRLCK, 0, 6, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 5, parent_pid);

		/*
		 * Test that the rest of the file is unlocked
		 */
		do_test(&tl, F_WRLCK, 0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 1");

/* //block2: */
		tst_resm(TINFO, "Enter block 2");

		/*
		 * Set a write lock at the middle of the file and a
		 * read lock just before
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 5, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 5, 5, parent_pid);

		/*
		 * Test the write lock.
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 10, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 5, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked.
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 2");

/* //block3: */
		tst_resm(TINFO, "Enter block 3");

		/*
		 * Set a write lock in the middle and a read lock that
		 * ends at the first byte of the write lock
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 5, 6) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 5, 6, parent_pid);

		/*
		 * Test write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 11, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 11, 4, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked.
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 3");

/* //block4: */
		tst_resm(TINFO, "Enter block 4");

		/*
		 * Set a write lock on the middle of the file and a read
		 * lock that overlaps the front of the write.
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 5, 8) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 5, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 5, 8, parent_pid);

		/*
		 * Test the write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 13, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 13, 2, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked.
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 4");

/* //block5: */
		tst_resm(TINFO, "Enter block 5");

		/*
		 * Set a write lock in the middle of a file and a read
		 * lock in the middle of it
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 10) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 13, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test the first write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 3, parent_pid);

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 13, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 13, 5, parent_pid);

		/*
		 * Test the second write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 18, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 18, 2, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 20, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 20, 0, 0);

		/*
		 * remove all the locks set above.
		 */
		unlock_file();
		tst_resm(TINFO, "Exit block 5");

/* //block6: */
		tst_resm(TINFO, "Enter block 6");
		/*
		 * Set a write lock in the middle of the file and a read
		 * lock that overlaps the end
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Set a read lock on the whole file
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 13, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test the write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 3, parent_pid);

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 13, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 13, 5, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 18, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 18, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 6");

/* //block7: */
		tst_resm(TINFO, "Enter block 7");

		/*
		 * Set a write lock in the middle of the file and a read
		 * lock starting at the last byte of the write lock
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Set a read lock on the whole file.
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 14, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 4, parent_pid);

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 14, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 14, 5, parent_pid);

		/*
		 * Test to make sure the end of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 19, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 19, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 7");

/* //block8: */
		tst_resm(TINFO, "Enter block 8");

		/*
		 * Set a write lock in the middle of the file and a read
		 * lock that starts just after the last byte of the
		 * write lock.
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Set a read lock on the whole file
		 */
		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 15, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test the write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 5, parent_pid);

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 15, 5, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 20, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 20, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 8");

/* //block9: */
		tst_resm(TINFO, "Enter block 9");

		/*
		 * Set a write lock at the middle of the file and a read
		 * lock that starts past the end of the write lock.
		 */
		if (do_lock(F_SETLK, (short)F_WRLCK, (short)0, 10, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		if (do_lock(F_SETLK, (short)F_RDLCK, (short)0, 16, 5) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl on file failed");

		/*
		 * Test the write lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 0, 0);
		compare_lock(&tl, (short)F_WRLCK, (short)0, 10, 5, parent_pid);

		/*
		 * Test that byte in between is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 15, 1);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 15, 1, 0);

		/*
		 * Test the read lock
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 16, 0);
		compare_lock(&tl, (short)F_RDLCK, (short)0, 16, 5, parent_pid);

		/*
		 * Test to make sure the rest of the file is unlocked
		 */
		do_test(&tl, (short)F_WRLCK, (short)0, 21, 0);
		compare_lock(&tl, (short)F_UNLCK, (short)0, 21, 0, 0);

		/*
		 * remove all the locks set above
		 */
		unlock_file();

		tst_resm(TINFO, "Exit block 9");

		stop_child();
		SAFE_CLOSE(cleanup, fd);
	}
	cleanup();
	tst_exit();
}
