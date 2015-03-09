/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *    ported from SPIE, section2/iosuite/dup1.c, by Airong Zhang
 *   Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
  WHAT:  Does dup return -1 on the 21st file?
  HOW:   Create up to _NFILE (20) files and check for -1 return on the
         next attempt
         Should check NOFILE as well as _NFILE.  19-Jun-84 Dale.
*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "test.h"

char *TCID = "dup06";
int TST_TOTAL = 1;

static int cnt_free_fds(int maxfd)
{
	int freefds = 0;

	for (maxfd--; maxfd >= 0; maxfd--)
		if (fcntl(maxfd, F_GETFD) == -1 && errno == EBADF)
			freefds++;

	return (freefds);
}

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int *fildes, i;
	int min;
	int freefds;
	int lc;
	const char *pfilname = "dup06";

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	min = getdtablesize();
	freefds = cnt_free_fds(min);
	fildes = malloc((min + 5) * sizeof(int));

	for (i = 0; i < min + 5; i++)
		fildes[i] = 0;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		unlink(pfilname);

		if ((fildes[0] = creat(pfilname, 0666)) == -1) {
			tst_resm(TFAIL, "Cannot open first file");
		} else {
			for (i = 1; i < min + 5; i++) {
				if ((fildes[i] = dup(fildes[i - 1])) == -1)
					break;
			}
			if (i < freefds) {
				tst_resm(TFAIL, "Not enough files duped");
			} else if (i > freefds) {
				tst_resm(TFAIL, "Too many files duped");
			} else {
				tst_resm(TPASS, "Test passed.");
			}
		}

		unlink(pfilname);

		for (i = 0; i < min + 5; i++) {
			if (fildes[i] != 0 && fildes[i] != -1)
				close(fildes[i]);

			fildes[i] = 0;
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}
