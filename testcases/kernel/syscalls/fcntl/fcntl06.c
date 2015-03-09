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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define F_RGETLK 10		/* kludge code */
#define F_RSETLK 11		/* kludge code */

char *TCID = "fcntl06";
int TST_TOTAL = 1;

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

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	fail = 0;

#ifdef LINUX_FILE_REGION_LOCK
	if (fcntl(fd, F_RGETLK, &tl) == -1) {
		if (errno == EINVAL)
			tst_brkm(TCONF, cleanup,
				 "fcntl remote locking feature not implemented in "
				 "the kernel");
		else {
			/*
			 * FIXME (garrcoop): having it always pass on
			 * non-EINVAL is a bad test.
			 */
			tst_resm(TPASS, "fcntl on file failed");
		}
	}

	/*
	 * Add a write lock to the middle of the file and unlock a section
	 * just before the lock
	 */
	if (do_lock(F_RSETLK, F_WRLCK, 0, 10, 5) == -1)
		tst_resm(TFAIL, "F_RSETLK WRLCK failed");

	if (do_lock(F_RSETLK, F_UNLCK, 0, 5, 5) == -1)
		tst_resm(TFAIL | TERRNO, "F_RSETLK UNLOCK failed");

	unlock_file();
#else
	tst_resm(TCONF, "system doesn't have LINUX_LOCK_FILE_REGION support");
#endif

	cleanup();
	tst_exit();
}

void setup(void)
{
	char *buf = STRING;
	char template[PATH_MAX];

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	tst_tmpdir();

	snprintf(template, PATH_MAX, "fcntl06XXXXXX");

	if ((fd = mkstemp(template)) == -1)
		tst_resm(TBROK | TERRNO, "mkstemp failed");

	if (write(fd, buf, STRINGSIZE) == -1)
		tst_resm(TBROK | TERRNO, "write failed");
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

void unlock_file(void)
{
	if (do_lock(F_RSETLK, (short)F_UNLCK, (short)0, 0, 0) == -1) {
		/* Same as FIXME comment above. */
		tst_resm(TPASS | TERRNO, "fcntl on file failed");
	}
}

void cleanup(void)
{

	if (close(fd) == -1)
		tst_resm(TWARN | TERRNO, "close failed");

	tst_rmdir();

}
