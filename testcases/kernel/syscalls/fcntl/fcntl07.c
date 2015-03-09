/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: Glen Overby
 *  CO-PILOT		: William Roske
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
/*
 * TEST CASES
 *
 * 1. test close-on-exec with a regular file
 * 2. test close-on-exec with a pipe
 * 3. test close-on-exec with a fifo
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <limits.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);
static void help(void);

char *TCID = "fcntl07";

static char *t_opt;

option_t options[] = {
	{"T:", NULL, &t_opt},
	{NULL, NULL, NULL}
};

static int file_fd, pipe_fds[2], fifo_fd;

#define FIFONAME "fifo"

static struct tcase {
	int *fd;
	const char *msg;
} tcases[] = {
	{&file_fd, "regular file"},
	{pipe_fds, "pipe (write end)"},
	{pipe_fds+1, "pipe (read end)"},
	{&fifo_fd, "fifo"},
};

int TST_TOTAL = ARRAY_SIZE(tcases);

static int test_open(char *arg);

static void verify_cloexec(struct tcase *tc)
{
	int fd = *(tc->fd);
	char pidname[255];
	int status, pid;

	TEST(fcntl(fd, F_SETFD, FD_CLOEXEC));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO,
			 "fcntl(%s[%d], F_SETFD, FD_CLOEXEC) failed",
			 tc->msg, fd);
		return;
	}

	sprintf(pidname, "%d", fd);

	switch (pid = FORK_OR_VFORK()) {
	case -1:
		tst_resm(TBROK | TERRNO, "fork() failed");
		return;
	case 0:
		execlp(TCID, TCID, "-T", pidname, NULL);

		/* the ONLY reason to do this is to get the errno printed out */
		fprintf(stderr, "exec(%s, %s, -T, %s) failed.  Errno %s [%d]\n",
			TCID, TCID, pidname, strerror(errno), errno);
		exit(2);
	default:
	break;
	}

	waitpid(pid, &status, 0);

	if (!WIFEXITED(status)) {
		tst_resm(TBROK, "waitpid return was 0%o", status);
		return;
	}

	switch ((WEXITSTATUS(status))) {
	case 2:
		tst_resm(TBROK, "exec failed");
	break;
	case 0:
		tst_resm(TPASS, "%s CLOEXEC fd was closed after exec()",
			 tc->msg);
	break;
	default:
		tst_resm(TFAIL, "%s child exited non-zero, %d",
			 tc->msg, WEXITSTATUS(status));
	}
}

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, options, &help);

	if (t_opt)
		exit(test_open(t_opt));

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			verify_cloexec(tcases + i);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	file_fd = SAFE_OPEN(cleanup, "test_file", O_CREAT | O_RDWR, 0666);
	SAFE_PIPE(cleanup, pipe_fds);
	SAFE_MKFIFO(cleanup, FIFONAME, 0666);
	fifo_fd = SAFE_OPEN(cleanup, FIFONAME, O_RDWR, 0666);
}

void cleanup(void)
{
	if (file_fd > 0 && close(file_fd))
		tst_resm(TWARN | TERRNO, "close(file_fd) failed");

	if (pipe_fds[0] > 0 && close(pipe_fds[0]))
		tst_resm(TWARN | TERRNO, "close(pipe_fds[0]) failed");

	if (pipe_fds[1] > 0 && close(pipe_fds[1]))
		tst_resm(TWARN | TERRNO, "close(pipe_fds[1]) failed");

	if (fifo_fd > 0 && close(fifo_fd))
		tst_resm(TWARN | TERRNO, "close(fifo_fd) failed");

	tst_rmdir();
}

void help(void)
{
	printf("  -T fd   The program runs as 'test_open()'\n");
}

int test_open(char *arg)
{
	int fd, rc;
	int status;

	fd = atoi(arg);

	rc = fcntl(fd, F_GETFD, &status);

	if (rc == -1 && errno == EBADF)
		return 0;

	fprintf(stderr, "fcntl() returned %i, errno %s(%i)\n",
		rc, tst_strerrno(errno), errno);

	return 1;
}
