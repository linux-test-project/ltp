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
 * 	write05-sctp-udp.c
 *
 * DESCRIPTION
 *	Check the return value, and errnos of write(2)
 * 	- when the file descriptor is invalid - EBADF
 *	- when the buf parameter is invalid - EFAULT or EINVAL
 *	- on an attempt to write to a UDP-style socket - EINVAL
 *	- on an attempt to write to a non-blocking UDP-style socket 
 *     		- EWOULDBLOCK | EAGAIN | EINVAL
 *
 * ALGORITHM
 * 	Attempt to write on a file with negative file descriptor, check for -1
 *	Attempt to write on a file with invalid buffer, check for -1
 * 	Open a socket and attempt to write to it, check for -1.
 *	Open a socket, set it to non-blocking mode. Write() should return 
 *	-l.
 * USAGE:  <for command-line>
 *      write05-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 Mingqin Liu
 *             -Adapted for UDP-style SCTP.
 *
 * Restrictions
 * 	None
 */

#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);
int set_nonblock(int s);

/* 0 terminated list of expected errnos */

char *TCID = "write05-sctp-udp";		/* Test program identifier */
int TST_TOTAL = 4;			/* Total number of test cases */
extern int Tst_count;
int s; 					/* socket descriptor */


main(int argc, char **argv)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */

	char pbuf[BUFSIZ];
	int status, pid, fd;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *)NULL, NULL)) !=
	    (char *) NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
	}

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

block1:
		/*tst_resm(TINFO, "Enter Block 1: test with bad fd"); */
		if (write(-1, pbuf, 1) != -1) {
			tst_resm(TFAIL, "write of invalid fd passed");
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != EBADF) {
				tst_resm(TFAIL, "expected EBADF got %d", errno);
			}
			tst_resm(TPASS, "received EBADF as expected.");
		}
		/*tst_resm(TINFO, "Exit Block 1"); */

block2:
		/*tst_resm(TINFO, "Enter Block 2: test with a bad address"); */

		if (write(s, (void *)-1, 10) != -1) {
			tst_resm(TFAIL, "write() on an invalid buffer "
				 "succeeded, but should have failed");
			/*NOTREACHED*/
		} else {
			TEST_ERROR_LOG(errno);
			if (errno != EFAULT && errno != EINVAL) {
				tst_resm(TFAIL, "write() returned illegal "
				"errno: expected EFAULT or EINVAL, got %d",
					 errno);
				/*NOTREACHED*/
			}
			tst_resm(TPASS, 
				"received EFAULT or EINVAL as expected.");
		}
		/* tst_resm(TINFO, "Exit Block 2"); */

block3:
		/* tst_resm(TINFO, "Enter Block 3: test with EINVAL"); */
		if (write(s, pbuf, 1) != -1) {
			tst_resm(TFAIL, "write on UDP-style socket"
				 " succeeded, but should have failed.");
		} else {
			if (errno != EINVAL) {
				tst_resm(TFAIL, "write() failed to set"
					 " errno to EINVAL, got: %d",
					 errno);
			} else {
				tst_resm(TPASS, 
					"received EINVAL as expected.");
			}
		}
block4:
		/* tst_resm(TINFO, "Enter Block 4: test with EWOULDBLOCK"); */
		if (set_nonblock(s) < 0) {
			tst_brkm(TBROK, 
				 cleanup, 
				 "socket ioctl failed. %s", 
				 strerror(errno));
		}
		if (write(s, pbuf, sizeof(pbuf)) != -1) {
			tst_resm(TFAIL, "write on UDP-style socket"
				 " succeeded, but should have failed.");
			exit(-1);
		} else {
			if (errno != EAGAIN 
				&& errno != EWOULDBLOCK
				&& errno != EINVAL) {
				tst_resm(TFAIL, "write() failed to set"
					 " errno to EAGAIN, got: %d",
					 errno);
				exit(errno);
			} else {
				tst_resm(TPASS, 
				"received EAGAIN|EWOULDBLOCK|EINVAL "
				"as expected.");
				exit(0);
			}
		}
	}
	cleanup();
	/*NOTREACHED*/
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void
setup(void)
{
	struct sockaddr_in local_addr;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);


	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/*
	 * Create a socket so that we can issue write to. 
	 */
	s = socket (PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = INADDR_ANY;	

	bind(s, (struct sockaddr *) (&local_addr), sizeof(local_addr));

}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 * 	completion or premature exit
 */
void
cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
	/*NOTREACHED*/
}

int set_nonblock(int s)
{
        int nonblock_on = 1;
#ifdef O_NONBLOCK
        int flags = fcntl(s, F_GETFL, 0);
        if (-1 == flags) {
                return -1;
        }

        return fcntl(s, F_SETFL, flags | O_NONBLOCK);
#else

        return ioctl(s, FIONBIO, &nonblock_on);
#endif
}

