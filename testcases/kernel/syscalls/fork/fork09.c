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
 * 	fork09.c
 *
 * DESCRIPTION
 *	Check that child has access to a full set of files.
 *
 * ALGORITHM
 * 	Parent opens a maximum number of files
 * 	Child closes one and attempts to open another, it should be
 * 	available
 *
 * USAGE
 * 	fork09
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 *	10/2008 Suzuki K P <suzuki@in.ibm.com>
 *		Fix maximum number of files open logic.
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>		/* for _SC_OPEN_MAX */
#include "test.h"
#include "usctest.h"

char *TCID = "fork09";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

char filname[40], childfile[40];
int first;
FILE **fildeses;		/* file streams */
int mypid, nfiles;

#define OPEN_MAX (sysconf(_SC_OPEN_MAX))

int main(int ac, char **av)
{
	int pid, status, nf;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/*
	 * perform global setup for the test
	 */
	setup();

	fildeses = (FILE **) malloc((OPEN_MAX + 10) * sizeof(FILE *));
	if (fildeses == NULL) {
		tst_brkm(TBROK, cleanup, "malloc failed");
	 /*NOTREACHED*/}

	/*
	 * check looping state if -i option is given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/*
		 * reset Tst_count in case we are looping.
		 */
		Tst_count = 0;

		mypid = getpid();

		tst_resm(TINFO, "OPEN_MAX is %ld", OPEN_MAX);

		/* establish first free file */
		sprintf(filname, "fork09.%d", mypid);
		if ((first = creat(filname, 0660)) == -1) {
			tst_brkm(TBROK, cleanup, "Cannot open first file %s, "
				 "errno = %d", filname, errno);
		 /*NOTREACHED*/}
		close(first);

		tst_resm(TINFO, "first file descriptor is %d ", first);

		if (unlink(filname) == -1) {
			tst_brkm(TBROK, cleanup, "Cannot unlink file %s, "
				 "errno = %d", filname, errno);
		 /*NOTREACHED*/}

		/*
		 * now open all the files for the test
		 */
		for (nfiles = first; nfiles < OPEN_MAX; nfiles++) {
			sprintf(filname, "file%d.%d", nfiles, mypid);
			if ((fildeses[nfiles] = fopen(filname, "a")) == NULL) {
				/* Did we already reach OPEN_MAX ? */
				if (errno == EMFILE)
					break;
				tst_brkm(TBROK, cleanup, "Parent: cannot open "
					 "file %d %s errno = %d", nfiles,
					 filname, errno);
			 /*NOTREACHED*/}
#ifdef DEBUG
			tst_resm(TINFO, "filname: %s", filname);
#endif
		}

		tst_resm(TINFO, "Parent reporting %d files open", nfiles - 1);

		if ((pid = fork()) == -1) {
			tst_brkm(TBROK, cleanup, "Fork failed");
		 /*NOTREACHED*/}

		if (pid == 0) {	/* child */
			nfiles--;
			if (fclose(fildeses[nfiles]) == -1) {
				tst_resm(TINFO, "Child could not close file "
					 "#%d, errno = %d", nfiles, errno);
				exit(1);
			 /*NOTREACHED*/} else {
				sprintf(childfile, "cfile.%d", getpid());
				if ((fildeses[nfiles] =
				     fopen(childfile, "a")) == NULL) {
					tst_resm(TINFO, "Child could not open "
						 "file %s, errno = %d",
						 childfile, errno);
					exit(1);
				 /*NOTREACHED*/} else {
					tst_resm(TINFO, "Child opened new "
						 "file #%d", nfiles);
					unlink(childfile);
					exit(0);
				}
			}
		} else {	/* parent */
			wait(&status);
			if (status >> 8 != 0) {
				tst_resm(TFAIL, "test 1 FAILED");
			} else {
				tst_resm(TPASS, "test 1 PASSED");
			}
		}

		/* clean up things in case we are looping */
		for (nf = first; nf < nfiles; nf++) {
			fclose(fildeses[nf]);
			sprintf(filname, "file%d.%d", nf, mypid);
			unlink(filname);
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup()
{
	/*
	 * capture signals
	 */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/*
	 * Pause if that option was specified
	 */
	TEST_PAUSE;

	/*
	 * make a temp directory and cd to it
	 */
	tst_tmpdir();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or at premature exit
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * remove tmp dir and all files in it
	 */
	tst_rmdir();

	tst_exit();
}
