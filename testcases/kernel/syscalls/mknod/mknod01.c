/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: William Roske
 *  CO-PILOT		: Dave Fenner
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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "mknod01";

#define PATH "test_node"

int tcases[] = {		/* modes to give nodes created (1 per text case) */
	S_IFREG | 0777,		/* ordinary file with mode 0777 */
	S_IFIFO | 0777,		/* fifo special with mode 0777 */
	S_IFCHR | 0777,		/* character special with mode 0777 */
	S_IFBLK | 0777,		/* block special with mode 0777 */

	S_IFREG | 04700,	/* ordinary file with mode 04700 (suid) */
	S_IFREG | 02700,	/* ordinary file with mode 02700 (sgid) */
	S_IFREG | 06700,	/* ordinary file with mode 06700 (sgid & suid) */
};

int TST_TOTAL = ARRAY_SIZE(tcases);

int main(int ac, char **av)
{
	int lc, i;
	dev_t dev;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/*
			 * overlayfs doesn't support mknod char device with
			 * major 0 and minor 0, which is known as whiteout_dev
			 */
			if (S_ISCHR(tcases[i]))
				dev = makedev(1, 3);
			else
				dev = 0;
			TEST(mknod(PATH, tcases[i], dev));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "mknod(%s, %#o, %lu) failed, errno=%d : %s",
					 PATH, tcases[i], dev, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS,
					 "mknod(%s, %#o, %lu) returned %ld",
					 PATH, tcases[i], dev, TEST_RETURN);
			}

			SAFE_UNLINK(cleanup, PATH);
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root();
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();
}
