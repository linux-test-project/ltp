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
 * 	read02-sctp-udp.c
 *
 * DESCRIPTION
 * 	test 1:
 *	Does read return -1 if file descriptor is not valid, check for EBADF
 *
 * 	test 2:
 * 	Check if read sets EFAULT or EINVAL, if buf is -1.
 *	
 *      test 3:
 *	Check if read succeeds if it is a zero-length buffer.
 *
 *	test 4: 
 * 	Check if read succeeds or get interrupted if it is a one-byte buf.
 *	
 *	test 5:
 *	Check if read succeeds or get interrupted if it is big buf.
 * 	
 * ALGORITHM
 * 	test 1:
 * 	Read with an invalid file descriptor, and expect an EBADF
 *
 * 	test 2:
 * 	Pass buf = -1 as a parameter to read, and expect an EFAULT or EINVAL.
 *	
 *      test 3:
 *	Pass count = 0 as a parameter to read. Expect it to return 0.
 *
 *	test 4: 
 * 	Pass count = 1 as a parameter to read. Expect it to either succeed or 
 *	get interrupted. 
 *	
 *	test 5:
 *	Pass count = 4096 as a parameter to read. Expect it to either succeed or
 *	get interrupted.
 * 	
 * 	
 * USAGE:  <for command-line>
 *  read02-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *      07/2001 Ported by Wayne Boyer
 *	04/2002 Adapted for SCTP by Mingqin Liu 
 *
 * RESTRICTIONS
 * 	None
 */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

#define BIG_INT         0xffff
#define RUN_TIME	10	 /* timer value */

void cleanup(void);
void setup(void);

char *TCID = "read02-sctp-udp";
extern int Tst_count;

struct sigaction new_act;

struct sockaddr_in6 local_addr;

int badfd = -1;
int s;
char buf[BUFSIZ];
char bigbuf[BIG_INT];
struct test_case_t {
	int *fd;
	void *buf;
	int count;
	int return_code;
	int error;
	char *desc;
} TC[] = {
	/* the file descriptor is invalid - EBADF */
	{&badfd, buf, 1, -1, EBADF, "bad file descriptor"},
	/* the buffer is invalid - EFAULT */
	{&s, (void *)-1, 1, -1, EFAULT, "invalid buffer"},
	{&s, buf, 0, 0, 0, "zero buf length"},
	{&s, buf, 1, 0, 0, "one byte buf"},
	{&s, bigbuf, BIG_INT, 0, 0, "INT_MAX buf"},
};

int TST_TOTAL=sizeof(TC)/sizeof(TC[0]); /* Total number of test cases. */


void alarm_action();


main(int ac, char **av)
{
	int i;
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	new_act.sa_handler = alarm_action;

	sigaction(SIGALRM, &new_act, NULL);
	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {
			
			/* set a timer in case read blocks. */
			alarm(RUN_TIME);

			TEST(read(*TC[i].fd, TC[i].buf, TC[i].count));

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "%s ", TC[i].desc);
			} else if (TEST_ERRNO == EINTR 
				&& TC[i].return_code == 0) {
				tst_resm(TPASS, "%s test case got "
					"interrupted ",
					 TC[i].desc);

			} else {
				tst_resm(TFAIL, 
					"%s unexpected error - %d : %s - "
					"expected %d", TC[i].desc, TEST_ERRNO,
					strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
	cleanup();
	/*NOTREACHED*/
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void
setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

 	/*
         * Create a socket so that we can issue write to. 
         */
        s = socket (PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
        
        local_addr.sin6_family = AF_INET6;
        local_addr.sin6_port = 0;
        local_addr.sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

        bind(s, (struct sockaddr *) (&local_addr), sizeof(local_addr));
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void
cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
	close (s);
	tst_exit();
	
}

/* 
 * alarm_action() - reset the alarm.
 *
 */
void alarm_action() {

	new_act.sa_handler = alarm_action;
	sigaction (SIGALRM, &new_act, NULL);
	
	alarm(RUN_TIME);
	return; 
}
