/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 *	pipe02.c
 *
 * DESCRIPTION
 *	Check that if a child has a "broken pipe", this information
 *	is transmitted to the waiting parent.
 *
 * ALGORITHM
 * 	1. Create a pipe, fork child
 * 	2. Child writes to pipe, parent reads to verify the pipe is working
 * 	3. Parent closes the read end of the pipe, child writes to it
 * 	4. Parent checks to see that child was terminated with SIGPIPE
 *
 * USAGE:  <for command-line>
 *  pipe02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	12/2002 Ported by Paul Larson
 *
 * RESTRICTIONS
 *	None
 */
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe02";
int TST_TOTAL = 1;
extern int Tst_count;

int usrsig;

void do_child(void);
void setup(void);
void cleanup(void);
void catch_usr2(int);

ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

int pp[2];			/* pipe descriptor */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char rbuf[BUFSIZ], wbuf[BUFSIZ];
	int pid, ret, len, rlen, status;
	int sig = 0;

	usrsig = 0;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}
#ifdef UCLINUX
	maybe_run_child(&do_child, "dd", &pp[0], &pp[1]);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		ret = pipe(pp);
		if (ret == -1)
			tst_brkm(TFAIL, cleanup, "pipe() failed, errno %d",
				 errno);

		strcpy(wbuf, "abcd\0");
		len = strlen(wbuf);

		pid = FORK_OR_VFORK();
		if (pid < 0)
			tst_brkm(TFAIL, cleanup, "fork() failed, errno %d",
				 errno);
		if (pid == 0) {
			/* CHILD */
#ifdef UCLINUX
			if (self_exec(av[0], "dd", pp[0], pp[1]) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child();
#endif
		} else {
			/* PARENT */
			close(pp[1]);	/* close write end of pipe */
			memset(rbuf, 0, sizeof(rbuf));
			rlen = safe_read(pp[0], rbuf, len);
			if (memcmp(wbuf, rbuf, len) != 0)
				tst_resm(TFAIL, "pipe read data and pipe "
					 "write data didn't match");
			close(pp[0]);	/* close read end of pipe */
			kill(pid, SIGUSR2);
			wait(&status);

			if (WIFSIGNALED(status))
				sig = WTERMSIG(status);
			if (sig != SIGPIPE)
				tst_resm(TFAIL, "SIGPIPE not returned by "
					 "child process");
			else
				tst_resm(TPASS, "child process returned "
					 "expected SIGPIPE");
		}
	}
	cleanup();

	/* NOT REACHED */
	return 0;
}

void catch_usr2(int sig)
{
	usrsig = 1;
}

/*
 * do_child()
 */
void do_child()
{
	char wbuf[BUFSIZ];
	int len;

	strcpy(wbuf, "abcd\0");
	len = strlen(wbuf);

	if (signal(SIGUSR2, catch_usr2) == SIG_ERR)
		tst_resm(TWARN, "signal setup for SIGUSR2 " "failed");
	if (signal(SIGPIPE, SIG_DFL) == SIG_ERR)
		tst_resm(TWARN, "signal setup for SIGPIPE " "failed");
	close(pp[0]);		/* close read end of pipe */
	write(pp[1], wbuf, len);
	while (!usrsig)
		sleep(1);
	write(pp[1], wbuf, len);
	exit(1);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

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

	/* exit with return code appropriate for results */
	tst_exit();
}
