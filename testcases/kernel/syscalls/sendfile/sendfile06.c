/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) Red Hat Inc., 2007
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
 *	sendfile06.c
 *
 * DESCRIPTION
 *	Testcase to test that sendfile(2) system call updates file
 *	position of in_fd correctly when passing NULL as offset.
 *
 * USAGE:  <for command-line>
 *  sendfile06 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	11/2007 Copyed from sendfile02.c by Masatake YAMATO
 *
 * RESTRICTIONS
 *	NONE
 */
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "usctest.h"
#include "test.h"

#ifndef OFF_T
#define OFF_T off_t
#endif /* Not def: OFF_T */

TCID_DEFINE(sendfile06);

char in_file[100];
char out_file[100];
int out_fd;
pid_t child_pid;
static int sockfd, s;
static struct sockaddr_in sin1;	/* shared between do_child and create_server */

void cleanup(void);
void do_child(void);
void setup(void);
int create_server(void);

int TST_TOTAL = 1;

#ifdef UCLINUX
static char *argv0;
#endif

void do_sendfile(void)
{
	int in_fd;
	struct stat sb;
	off_t after_pos;
	int wait_status;
	int wait_stat;

	out_fd = create_server();

	if ((in_fd = open(in_file, O_RDONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: %d", errno);
	 }
	if (stat(in_file, &sb) < 0) {
		tst_brkm(TBROK, cleanup, "stat failed: %d", errno);
	 }

	TEST(sendfile(out_fd, in_fd, NULL, sb.st_size));
	if ((after_pos = lseek(in_fd, 0, SEEK_CUR)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "lseek after invoking sendfile failed: %d", errno);
	 }

	if (STD_FUNCTIONAL_TEST) {
		/* Close the sockets */
		shutdown(sockfd, SHUT_RDWR);
		shutdown(s, SHUT_RDWR);
		if (TEST_RETURN != sb.st_size) {
			tst_resm(TFAIL, "sendfile(2) failed to return "
				 "expected value, expected: %"PRId64", "
				 "got: %ld", (int64_t)sb.st_size, TEST_RETURN);
			kill(child_pid, SIGKILL);
		} else if (after_pos != sb.st_size) {
			tst_resm(TFAIL, "sendfile(2) failed to update "
				 " the file position of in_fd, "
				 "expected file position: %"PRId64", "
				 "actual file position %"PRId64,
				 (int64_t)sb.st_size, (int64_t)after_pos);
			kill(child_pid, SIGKILL);
		} else {
			tst_resm(TPASS, "functionality of sendfile() is "
				 "correct");
			wait_status = waitpid(-1, &wait_stat, 0);
		}
	} else {
		tst_resm(TPASS, "call succeeded");
		/* Close the sockets */
		shutdown(sockfd, SHUT_RDWR);
		shutdown(s, SHUT_RDWR);
		if (TEST_RETURN != sb.st_size) {
			kill(child_pid, SIGKILL);
		} else {
			wait_status = waitpid(-1, &wait_stat, 0);
		}
	}

	close(in_fd);
}

/*
 * do_child
 */
void do_child()
{
	int lc;
	socklen_t length;
	char rbuf[4096];

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		length = sizeof(sin1);
		recvfrom(sockfd, rbuf, 4096, 0, (struct sockaddr *)&sin1,
			 &length);
	}
	exit(0);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	int fd;
	char buf[100];

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();
	sprintf(in_file, "in.%d", getpid());
	if ((fd = creat(in_file, 00700)) < 0) {
		tst_brkm(TBROK, cleanup, "creat failed in setup, errno: %d",
			 errno);
	 }
	sprintf(buf, "abcdefghijklmnopqrstuvwxyz");
	if (write(fd, buf, strlen(buf)) < 0) {
		tst_brkm(TBROK, cleanup, "write failed, errno: %d", errno);
	 }
	close(fd);
	sprintf(out_file, "out.%d", getpid());
}

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

	close(out_fd);
	/* delete the test directory created in setup() */
	tst_rmdir();

}

int create_server(void)
{
	static int count = 0;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		tst_brkm(TBROK, cleanup, "call to socket() failed: %s",
			 strerror(errno));
		return -1;
	}
	sin1.sin_family = AF_INET;
	sin1.sin_port = htons((getpid() % 32768) + 11000 + count);
	sin1.sin_addr.s_addr = INADDR_ANY;
	count++;
	if (bind(sockfd, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "call to bind() failed: %s",
			 strerror(errno));
		return -1;
	}
	child_pid = FORK_OR_VFORK();
	if (child_pid < 0) {
		tst_brkm(TBROK, cleanup, "client/server fork failed: %s",
			 strerror(errno));
		return -1;
	}
	if (!child_pid) {	/* child */
#ifdef UCLINUX
		if (self_exec(argv0, "") < 0) {
			tst_brkm(TBROK, cleanup, "self_exec failed");
			return -1;

		}
#else
		do_child();
#endif
	}

	s = socket(PF_INET, SOCK_DGRAM, 0);
	inet_aton("127.0.0.1", &sin1.sin_addr);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "call to socket() failed: %s",
			 strerror(errno));
		return -1;
	}
	if (connect(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "call to connect() failed: %s",
			 strerror(errno));
	}
	return s;

}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* parse_opts() return message */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }
#ifdef UCLINUX
	argv0 = av[0];
	maybe_run_child(&do_child, "");
#endif

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		do_sendfile();
	}
	cleanup();

	tst_exit();
}