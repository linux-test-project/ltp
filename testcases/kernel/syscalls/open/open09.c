/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *   Copyright (c) 2013 Wanlong Gao <gaowanlong@cn.fujitsu.com>
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
 * Description:
 *	1. open an O_WRONLY file, test if read failed.
 *	2. open an O_RDONLY file, test if write failed.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "test.h"

char *TCID = "open09";
int TST_TOTAL = 2;

#define PASSED 1
#define FAILED 0

static char tempfile[40] = "";

static void setup(void);
static void cleanup(void);

int main(int ac, char *av[])
{
	int fildes;
	int ret = 0;
	char pbuf[BUFSIZ];

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		fildes = open(tempfile, O_WRONLY);
		if (fildes == -1)
			tst_brkm(TFAIL, cleanup, "\t\t\topen failed");

		ret = read(fildes, pbuf, 1);
		if (ret != -1)
			tst_resm(TFAIL, "read should not succeed");
		else
			tst_resm(TPASS, "Test passed in O_WRONLY.");

		close(fildes);

		fildes = open(tempfile, O_RDONLY);
		if (fildes == -1) {
			tst_resm(TFAIL, "\t\t\topen failed");
		} else {
			ret = write(fildes, pbuf, 1);
			if (ret != -1)
				tst_resm(TFAIL, "write should not succeed");
			else
				tst_resm(TPASS, "Test passed in O_RDONLY.");
		}
		close(fildes);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fildes;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
	tst_tmpdir();

	sprintf(tempfile, "open09.%d", getpid());

	fildes = creat(tempfile, 0600);
	if (fildes == -1) {
		tst_brkm(TBROK, cleanup, "\t\t\tcan't create '%s'",
			 tempfile);
	} else {
		close(fildes);
	}
}

static void cleanup(void)
{
	unlink(tempfile);
	tst_rmdir();
}
