/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "test.h"

static void setup(void);
static void cleanup(void);

static char fname[255];
static int fd;
static int whences[] = { 5, -1, 7 };

char *TCID = "lseek03";
int TST_TOTAL = ARRAY_SIZE(whences);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* Call lseek(2) */
			TEST(lseek(fd, (off_t) 1, whences[i]));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == EINVAL) {
					tst_resm(TPASS,
						 "lseek(%s, 1, %d) Failed, errno=%d : %s",
						 fname, whences[i],
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "lseek(%s, 1, %d) Failed, errno=%d %s, expected %d(EINVAL)",
						 fname, whences[i],
						 TEST_ERRNO,
						 strerror(TEST_ERRNO),
						 EINVAL);
				}
			} else {
				tst_resm(TFAIL, "lseek(%s, 1, %d) returned %ld",
					 fname, whences[i], TEST_RETURN);
			}
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "tfile_%d", getpid());

	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
			 fname, errno, strerror(errno));
	}
}

void cleanup(void)
{
	if (close(fd) == -1) {
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s", fname, errno,
			 strerror(errno));
	}

	tst_rmdir();
}
