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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	sendfile04.c
 *
 * DESCRIPTION
 *	Testcase to test that sendfile(2) system call returns EFAULT
 *	when passing wrong buffer.
 *
 * ALGORITHM
 *     Given wrong address or protected buffer as OFFSET argument to sendfile.
 *     A wrong address is created by munmap a buffer allocated by mmap.
 *     A protected buffer is created by mmap with specifying protection.
 *
 * USAGE:  <for command-line>
 *  sendfile04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "test.h"
#include "safe_macros.h"

#ifndef OFF_T
#define OFF_T off_t
#endif /* Not def: OFF_T */

TCID_DEFINE(sendfile04);

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

#define PASS_MAPPED_BUFFER 0
#define PASS_UNMAPPED_BUFFER 1

struct test_case_t {
	int protection;
	int pass_unmapped_buffer;
} testcases[] = {
	{
	PROT_NONE, PASS_MAPPED_BUFFER}, {
	PROT_READ, PASS_MAPPED_BUFFER}, {
	PROT_EXEC, PASS_MAPPED_BUFFER}, {
	PROT_EXEC | PROT_READ, PASS_MAPPED_BUFFER}, {
PROT_READ | PROT_WRITE, PASS_UNMAPPED_BUFFER},};

int TST_TOTAL = sizeof(testcases) / sizeof(testcases[0]);

#ifdef UCLINUX
static char *argv0;
#endif

void do_sendfile(int prot, int pass_unmapped_buffer)
{
	OFF_T *protected_buffer;
	int in_fd;
	struct stat sb;

	protected_buffer = mmap(NULL,
				sizeof(*protected_buffer),
				prot, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (protected_buffer == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed: %d", errno);
	}

	out_fd = create_server();

	if ((in_fd = open(in_file, O_RDONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: %d", errno);
	}
	SAFE_STAT(cleanup, in_file, &sb);

	if (pass_unmapped_buffer) {
		SAFE_MUNMAP(cleanup, protected_buffer,
			    sizeof(*protected_buffer));
	}

	TEST(sendfile(out_fd, in_fd, protected_buffer, sb.st_size));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
	} else {
		if (TEST_ERRNO != EFAULT) {
			tst_resm(TFAIL, "sendfile returned unexpected "
				 "errno, expected: %d, got: %d",
				 EFAULT, TEST_ERRNO);
		} else {
			tst_resm(TPASS, "sendfile() returned %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}
	}

	shutdown(sockfd, SHUT_RDWR);
	shutdown(s, SHUT_RDWR);
	kill(child_pid, SIGKILL);
	close(in_fd);

	if (!pass_unmapped_buffer) {
		/* Not unmapped yet. So do it now. */
		munmap(protected_buffer, sizeof(*protected_buffer));
	}
}

/*
 * do_child
 */
void do_child(void)
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
void setup(void)
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
void cleanup(void)
{

	close(out_fd);
	/* delete the test directory created in setup() */
	tst_rmdir();

}

int create_server(void)
{
	static int count = 0;
	socklen_t slen = sizeof(sin1);

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		tst_brkm(TBROK, cleanup, "call to socket() failed: %s",
			 strerror(errno));
		return -1;
	}
	sin1.sin_family = AF_INET;
	sin1.sin_port = 0; /* pick random free port */
	sin1.sin_addr.s_addr = INADDR_ANY;
	count++;
	if (bind(sockfd, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "call to bind() failed: %s",
			 strerror(errno));
		return -1;
	}
	SAFE_GETSOCKNAME(cleanup, sockfd, (struct sockaddr *)&sin1, &slen);

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
	SAFE_CONNECT(cleanup, s, (struct sockaddr *)&sin1, sizeof(sin1));
	return s;

}

int main(int ac, char **av)
{
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	argv0 = av[0];
	maybe_run_child(&do_child, "");
#endif

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			do_sendfile(testcases[i].protection,
				    testcases[i].pass_unmapped_buffer);
		}
	}
	cleanup();

	tst_exit();
}
