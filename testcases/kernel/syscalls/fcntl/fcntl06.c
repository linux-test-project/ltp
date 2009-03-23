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
 *	fcntl06.c
 *
 * DESCRIPTION
 *	Error checking conditions for remote locking of regions of a file.
 *
 * CALLS
 *	open(2), write(2), fcntl(2)
 *
 * ALGORITHM
 *	Test unlocking sections around a write lock using remote Lock/Unlock
 *	call which should all fail.
 *
 * USAGE
 *	fcntl06
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Currently Linux kernel doesn't implement R_GETLK/R_SETLK facility,
 *	but this facility seems to be present in other standard flavours of
 *	Unix. Currently this program has all the testing done under
 *	"#ifdef LINUX_FILE_REGION_LOCK", when Linux implements the regions
 *	locking then, this testcase should be recompiled accordingly with the
 *	"ifdef" removed.
 */

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

#define F_RGETLK 10		/* kludge code */
#define F_RSETLK 11		/* kludge code */

char *TCID = "fcntl06";
int TST_TOTAL = 1;
extern int Tst_count;

void setup();
void cleanup();

#define STRINGSIZE	27
#define	STRING		"abcdefghijklmnopqrstuvwxyz\n"

int fd;
void unlock_file();
int do_lock(int, short, short, int, int);

int main(int ac, char **av)
{
	int fail = 0;

	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* //block1:    *//* Error: when no lock is set */
	tst_resm(TINFO, "Enter block 1");
	fail = 0;

#ifdef LINUX_FILE_REGION_LOCK
	if (fcntl(fd, F_RGETLK, &tl) < 0) {
		if (errno == EINVAL) {
			tst_resm(TINFO, "fcntl remote locking feature not "
				 "implemented in the kernel: exitting");
			cleanup();
		 /*NOTREACHED*/} else {
			tst_resm(TPASS, "fcntl on file failed: Test " "PASSED");
		}
	}

	/*
	 * Add a write lock to the middle of the file and unlock a section
	 * just before the lock
	 */
	if (do_lock(F_RSETLK, (short)F_WRLCK, (short)0, 10, 5) < 0) {
		tst_resm(TFAIL, "F_RSETLK WRLCK failed");
		fail = 1;
	}

	if (do_lock(F_RSETLK, (short)F_UNLCK, (short)0, 5, 5) < 0) {
		tst_resm(TFAIL, "F_RSETLK UNLOCK failed");
		fail = 1;
	}

	unlock_file();
#endif

	if (fail) {
		tst_resm(TFAIL, "Block 1 FAILED");
	} else {
		tst_resm(TPASS, "Block 1 PASSED");
	}
	close(fd);

	tst_resm(TINFO, "Exit block 1");
	cleanup();
	return 0;
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup()
{
	char *buf = STRING;
	char template[PATH_MAX];

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	snprintf(template, PATH_MAX, "fcntl06XXXXXX");

	if ((fd = mkstemp(template)) < 0) {
		tst_resm(TFAIL, "Couldn't open temp file! errno = %d", errno);
	}

	if (write(fd, buf, STRINGSIZE) < 0) {
		tst_resm(TFAIL, "Couldn't write to temp file! errno = %d",
			 errno);
	}
}

int do_lock(int cmd, short type, short whence, int start, int len)
{
	struct flock fl;

	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;
	return (fcntl(fd, cmd, &fl));
}

void unlock_file()
{
	if (do_lock(F_RSETLK, (short)F_UNLCK, (short)0, 0, 0) < 0) {
		perror("");
		tst_resm(TINFO, "fcntl on file failed: Test PASSED");
	}
}

/*
 * cleanup()
 *	performs all the ONE TIME cleanup for this test at completion or
 * or premature exit.
 */
void cleanup()
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
