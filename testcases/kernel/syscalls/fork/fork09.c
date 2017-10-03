/*
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
 *
 * NAME
 *	fork09.c
 *
 * DESCRIPTION
 *	Check that child has access to a full set of files.
 *
 * ALGORITHM
 *	Parent opens a maximum number of files
 *	Child closes one and attempts to open another, it should be
 *	available
 *
 * USAGE
 *	fork09
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 *	10/2008 Suzuki K P <suzuki@in.ibm.com>
 *		Fix maximum number of files open logic.
 *
 * RESTRICTIONS
 *	None
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>		/* for _SC_OPEN_MAX */
#include "test.h"
#include "safe_macros.h"

char *TCID = "fork09";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

static char filname[40], childfile[40];
static int first;
static FILE **fildeses;		/* file streams */
static int mypid, nfiles;

#define OPEN_MAX (sysconf(_SC_OPEN_MAX))

int main(int ac, char **av)
{
	int pid, status, nf;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	fildeses = malloc((OPEN_MAX + 10) * sizeof(FILE *));
	if (fildeses == NULL)
		tst_brkm(TBROK, cleanup, "malloc failed");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		mypid = getpid();

		tst_resm(TINFO, "OPEN_MAX is %ld", OPEN_MAX);

		/* establish first free file */
		sprintf(filname, "fork09.%d", mypid);
		first = SAFE_CREAT(cleanup, filname, 0660);
		close(first);

		tst_resm(TINFO, "first file descriptor is %d ", first);

		SAFE_UNLINK(cleanup, filname);

		/*
		 * now open all the files for the test
		 */
		for (nfiles = first; nfiles < OPEN_MAX; nfiles++) {
			sprintf(filname, "file%d.%d", nfiles, mypid);
			fildeses[nfiles] = fopen(filname, "a");
			if (fildeses[nfiles] == NULL) {
				/* Did we already reach OPEN_MAX ? */
				if (errno == EMFILE)
					break;
				tst_brkm(TBROK, cleanup, "Parent: cannot open "
					 "file %d %s errno = %d", nfiles,
					 filname, errno);
			}
#ifdef DEBUG
			tst_resm(TINFO, "filname: %s", filname);
#endif
		}

		tst_resm(TINFO, "Parent reporting %d files open", nfiles - 1);

		pid = fork();
		if (pid == -1)
			tst_brkm(TBROK, cleanup, "Fork failed");

		if (pid == 0) {	/* child */
			nfiles--;
			if (fclose(fildeses[nfiles]) == -1) {
				tst_resm(TINFO, "Child could not close file "
					 "#%d, errno = %d", nfiles, errno);
				exit(1);
			} else {
				sprintf(childfile, "cfile.%d", getpid());
				fildeses[nfiles] = fopen(childfile, "a");
				if (fildeses[nfiles] == NULL) {
					tst_resm(TINFO, "Child could not open "
						 "file %s, errno = %d",
						 childfile, errno);
					exit(1);
				} else {
					tst_resm(TINFO, "Child opened new "
						 "file #%d", nfiles);
					unlink(childfile);
					exit(0);
				}
			}
		} else {	/* parent */
			wait(&status);
			if (status >> 8 != 0)
				tst_resm(TFAIL, "test 1 FAILED");
			else
				tst_resm(TPASS, "test 1 PASSED");
		}

		/* clean up things in case we are looping */
		for (nf = first; nf < nfiles; nf++) {
			fclose(fildeses[nf]);
			sprintf(filname, "file%d.%d", nf, mypid);
			unlink(filname);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	umask(0);

	TEST_PAUSE;
	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}
