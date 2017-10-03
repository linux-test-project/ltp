/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Red Hat Inc., 2007
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	Testcase to test that sendfile(2) system call updates file
 *	position of in_fd correctly when passing NULL as offset.
 *
 * HISTORY
 *	11/2007 Copyed from sendfile02.c by Masatake YAMATO
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
#include <string.h>
#include "test.h"
#include "safe_macros.h"

TCID_DEFINE(sendfile06);

#define IN_FILE		"infile"
#define OUT_FILE	"outfile"

static pid_t child_pid;
static int sockfd;
static struct sockaddr_in sin1;
static struct stat sb;

static void cleanup(void);
static void do_child(void);
static void setup(void);
static int create_server(void);

int TST_TOTAL = 1;

#ifdef UCLINUX
static char *argv0;
#endif

static void do_sendfile(void)
{
	int in_fd, out_fd;
	off_t after_pos;
	int wait_stat;

	out_fd = create_server();

	in_fd = SAFE_OPEN(cleanup, IN_FILE, O_RDONLY);

	TEST(sendfile(out_fd, in_fd, NULL, sb.st_size));
	if ((after_pos = lseek(in_fd, 0, SEEK_CUR)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "lseek after invoking sendfile failed: %d", errno);
	}

	/* Close the sockets */
	shutdown(sockfd, SHUT_RDWR);
	shutdown(out_fd, SHUT_RDWR);
	if (TEST_RETURN != sb.st_size) {
		tst_resm(TFAIL, "sendfile(2) failed to return "
			 "expected value, expected: %" PRId64 ", "
			 "got: %ld", (int64_t) sb.st_size, TEST_RETURN);
		SAFE_KILL(cleanup, child_pid, SIGKILL);
	} else if (after_pos != sb.st_size) {
		tst_resm(TFAIL, "sendfile(2) failed to update "
			 " the file position of in_fd, "
			 "expected file position: %" PRId64 ", "
			 "actual file position %" PRId64,
			 (int64_t) sb.st_size, (int64_t) after_pos);
		SAFE_KILL(cleanup, child_pid, SIGKILL);
	} else {
		tst_resm(TPASS, "functionality of sendfile() is "
			 "correct");
		waitpid(-1, &wait_stat, 0);
	}

	SAFE_CLOSE(cleanup, in_fd);
	SAFE_CLOSE(cleanup, out_fd);
	SAFE_CLOSE(cleanup, sockfd);
}

static void do_child(void)
{
	socklen_t length = sizeof(sin1);
	char rbuf[4096];
	ssize_t ret, bytes_total_received = 0;

	while (bytes_total_received < sb.st_size) {
		ret = recvfrom(sockfd, rbuf, 4096, 0, (struct sockaddr *)&sin1,
			       &length);
		if (ret < 0) {
			fprintf(stderr, "child process recvfrom failed: %s\n",
				strerror(errno));
			exit(1);
		}
		bytes_total_received += ret;
	}

	exit(0);
}

static void setup(void)
{
	int fd;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_CREAT(cleanup, IN_FILE, 0600);

	SAFE_WRITE(cleanup, 1, fd, "abcdefghijklmnopqrstuvwxyz", 26);

	SAFE_FSTAT(cleanup, fd, &sb);

	SAFE_CLOSE(cleanup, fd);
}

static void cleanup(void)
{
	tst_rmdir();
}

static int create_server(void)
{
	int s;
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

	if (!child_pid) {
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
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	argv0 = av[0];
	maybe_run_child(&do_child, "");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		do_sendfile();
	}

	cleanup();
	tst_exit();
}
