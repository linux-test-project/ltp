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
 *	pipe07.c
 *
 * DESCRIPTION
 * 	Test the ability of pipe to open the maximum even number of file
 * 	descriptors permitted (or (maxfds - 3)/2 pipes)
 *
 * ALGORITHM
 * 	1. open pipes until EMFILE is returned
 * 	2. check to see that the number of pipes opened is
 * 	   (maxfds - 3) / 2
 *
 * USAGE:  <for command-line>
 *  pipe07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "test.h"
#include "usctest.h"

char *TCID = "pipe07";
int TST_TOTAL = 1;

int exp_enos[] = { EMFILE, 0 };

int pipe_ret, pipes[2];
char currdir[PATH_MAX];
char *tempdir = NULL;
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int min;		/* number of file descriptors */
	int usedfds;		/* number of currently used file descriptors */
	int npipes;		/* number of pipes opened */
	pid_t mypid;		/* process of id of test */
	char *cmdstring = NULL;	/* hold command string to execute using system() */
	FILE *f;		/* used for retrieving the used fds */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	setup();
	/* Get the currently used number of file descriptors */
	mypid = getpid();
	cmdstring = malloc(BUFSIZ);
	snprintf(cmdstring, BUFSIZ, "test -d /proc/%d/fd || exit 1 ; "
		 "ls -A -1 /proc/%d/fd | wc -l | awk {'print $1'} > pipe07.tmp",
		 mypid, mypid);
	if (system(cmdstring) == 0) {
		f = fopen("pipe07.tmp", "r");
		fscanf(f, "%d", &usedfds);
		fclose(f);
	} else
		usedfds = 3;	/* Could not get processes used fds, so assume 3 */
	unlink("pipe07.tmp");

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		min = getdtablesize();

		/* subtract used fds */
		min -= usedfds;

		for (npipes = 0;; npipes++) {
			pipe_ret = pipe(pipes);
			if (pipe_ret < 0) {
				if (errno != EMFILE) {
					tst_brkm(TFAIL, cleanup,
						 "got unexpected error - %d",
						 errno);
				}
				break;
			}
		}
		if (npipes == (min / 2))
			tst_resm(TPASS, "Opened %d pipes", npipes);
		else
			tst_resm(TFAIL, "Unable to open maxfds/2 pipes");

	}
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	char template[PATH_MAX];

	/* I had to do this, instead of tst_tmpdir() b/c I was receiving      *
	 * a SIGSEGV for some reason when I tried to use tst_tmpdir/tst_rmdir */

	/* Save current working directory */
	getcwd(currdir, PATH_MAX);

	/* Create temp directory and cd to it */
	snprintf(template, PATH_MAX, "%s/%.3sXXXXXX", TEMPDIR, TCID);
	tempdir = mkdtemp(template);
	chdir(tempdir);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* I had to do this, instead of tst_tmpdir() b/c I was receiving      *
	 * a SIGSEGV for some reason when I tried to use tst_tmpdir/tst_rmdir */

	/* Chdir back to original working directory */
	chdir(currdir);

	rmdir(tempdir);
}