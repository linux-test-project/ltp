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
 *	sendfile02.c
 *
 * DESCRIPTION
 *	Testcase to test the basic functionality of the sendfile(2) system call.
 *
 * ALGORITHM
 *	1. call sendfile(2) with offset = 0
 *	2. call sendfile(2) with offset in the middle of the file
 *
 * USAGE:  <for command-line>
 *  sendfile02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	08/2002 Make it use a socket so it works with 2.5 kernel
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
#include <netinet/in.h>
#include "usctest.h"
#include "test.h"

char *TCID = "sendfile02";
int TST_TOTAL = 4;
extern int Tst_count;

char in_file[100];
char out_file[100];
struct sockaddr_in sin1;
int out_fd;

void cleanup(void);
void setup(void);
int create_server(void);

struct test_case_t {
	char *desc;
	int offset;
	int exp_retval;
} testcases[] = {
	{ "Test sendfile(2) with offset = 0", 0, 26 },
	{ "Test sendfile(2) with offset in the middle of file", 2, 24 },
	{ "Test sendfile(2) with offset in the middle of file", 4, 22 },
	{ "Test sendfile(2) with offset in the middle of file", 6, 20 }
};

main(int ac, char **av)
{
	int i;
	int lc;				/* loop counter */
	char *msg;			/* parse_opts() return message */

	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
	}

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			do_sendfile(testcases[i].offset, i);
		}
	}
	cleanup();

	/*NOTREACHED*/
}

do_sendfile(off_t offset, int i)
{
	int in_fd;
	struct stat sb;
	int wait_status;
	int * wait_stat;

	if ((in_fd = open(in_file, O_RDONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: %d", errno);
		/*NOTREACHED*/
	}
	if (stat(in_file, &sb) < 0) {
		tst_brkm(TBROK, cleanup, "stat failed: %d", errno);
		/*NOTREACHED*/
	}

	TEST(sendfile(out_fd, in_fd, &offset, sb.st_size - offset));

	if (STD_FUNCTIONAL_TEST) {
		if (TEST_RETURN != testcases[i].exp_retval) {
			tst_resm(TFAIL, "sendfile(2) failed to return "
				 "expected value, expected: %d, "
				 "got: %d", testcases[i].exp_retval,
				 TEST_RETURN);
		} else {
			tst_resm(TPASS, "functionality of sendfile() is "
					"correct");
			}
	} else {
		tst_resm(TPASS, "call succeeded");
	}

	close(in_fd);

        wait_status = waitpid(-1, wait_stat, 0);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void
setup()
{
	int fd;
	char buf[100];

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
	sin1.sin_family = AF_INET;
	sin1.sin_port = htons((getpid() % 32768) + 11000);
	sin1.sin_addr.s_addr = INADDR_ANY;

	out_fd = create_server();
	/* make a temporary directory and cd to it */
	tst_tmpdir();
	sprintf(in_file, "in.%d", getpid());
	if ((fd = creat(in_file, 00700)) < 0) {
		tst_brkm(TBROK, cleanup, "creat failed in setup, errno: %d",
			 errno);
		/*NOTREACHED*/
	}
	sprintf(buf, "abcdefghijklmnopqrstuvwxyz");
	if (write(fd, buf, strlen(buf)) < 0) {
		tst_brkm(TBROK, cleanup, "write failed, errno: %d", errno);
		/*NOTREACHED*/
	}
	close(fd);
	sprintf(out_file, "out.%d", getpid());
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	close(out_fd);
	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

int create_server(void) {
	pid_t mypid;
	int lc;
	int sockfd, s;
	int length, newfd;
	char rbuf[4096];

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		tst_brkm(TBROK, cleanup, "call to socket() failed: %s",
			strerror(errno));
		return -1;
	}
	if(bind(sockfd, (struct sockaddr*)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "call to bind() failed: %s",
			strerror(errno));
		return -1;
	}
	mypid = fork();
	if(mypid < 0) {
		tst_brkm(TBROK, cleanup, "client/server fork failed: %s",
			strerror(errno));
		return -1;
	}
	if(!mypid) { 
		for (lc = 0; TEST_LOOPING(lc); lc++) {
			recvfrom(sockfd, rbuf, 4096, 0, (struct sockaddr*)&sin1, sizeof(sin1));
		}
		exit(0);
	}

	s = socket(PF_INET, SOCK_DGRAM, 0);
	inet_aton("127.0.0.1", &sin1.sin_addr);
	if(s < 0) {
		tst_brkm(TBROK, cleanup, "call to socket() failed: %s",
			strerror(errno));
		return -1;
	}
	if (connect(s, (struct sockaddr*)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "call to connect() failed: %s",
			strerror(errno));
	}
	return s;

}
