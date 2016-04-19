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
 *	fcntl13.c
 *
 * DESCRIPTION
 *	Testcase to test that fcntl() sets errno correctly.
 *
 * USAGE
 *	fcntl13
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <fcntl.h>
#include <errno.h>
#include "test.h"

#define F_BADCMD 99999

char *TCID = "fcntl13";
int TST_TOTAL = 1;

void setup(void);

int main(int ac, char **av)
{
	int lc;

	struct flock flock;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if (fcntl(1, F_BADCMD, 1) != -1)
			tst_resm(TFAIL, "fcntl(2) failed to FAIL");
		else if (errno != EINVAL)
			tst_resm(TFAIL, "Expected EINVAL got %d", errno);
		else
			tst_resm(TPASS, "got EINVAL");

#ifndef UCLINUX
		if (fcntl(1, F_SETLK, (void *)-1) != -1) {
			tst_resm(TFAIL, "F_SETLK: fcntl(2) failed to FAIL");
		} else if (errno != EFAULT) {
			tst_resm(TFAIL, "F_SETLK: Expected EFAULT got %d",
				 errno);
		} else {
			tst_resm(TPASS, "F_SETLK: got EFAULT");
		}

		if (fcntl(1, F_SETLKW, (void *)-1) != -1) {
			tst_resm(TFAIL, "F_SETLKW: fcntl(2) failed to FAIL");
		} else if (errno != EFAULT) {
			tst_resm(TFAIL, "F_SETLKW: Expected EFAULT got %d",
				 errno);
		} else {
			tst_resm(TPASS, "F_SETLKW: got EFAULT");
		}

		if (fcntl(1, F_GETLK, (void *)-1) != -1) {
			tst_resm(TFAIL, "F_GETLK: fcntl(2) failed to FAIL");
		} else if (errno != EFAULT) {
			tst_resm(TFAIL, "F_GETLK: Expected EFAULT got %d",
				 errno);
		} else {
			tst_resm(TPASS, "F_GETLK: got EFAULT");
		}

#else
		tst_resm(TCONF, "Skip EFAULT on uClinux");
#endif
		flock.l_whence = -1;
		flock.l_type = F_WRLCK;
		flock.l_start = 0L;
		flock.l_len = 0L;

		if (fcntl(1, F_SETLK, &flock) != -1)
			tst_resm(TFAIL, "fcntl(2) failed to FAIL");
		else if (errno != EINVAL)
			tst_resm(TFAIL, "Expected EINVAL, got %d", errno);
		else
			tst_resm(TPASS, "got EINVAL");

		if (fcntl(-1, F_GETLK, &flock) != -1)
			tst_resm(TFAIL, "fcntl(2) failed to FAIL");
		else if (errno != EBADF)
			tst_resm(TFAIL, "Expected EBADF, got %d", errno);
		else
			tst_resm(TPASS, "got EBADFD");
	}

	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, NULL);

	TEST_PAUSE;
}
