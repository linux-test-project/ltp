/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * DESCRIPTION
 *	Check sbrk() with error condition that should produce ENOMEM.
 */

#include <errno.h>
#include <unistd.h>
#include "test.h"

#define INC 16*1024*1024

char *TCID = "sbrk02";
int TST_TOTAL = 1;

static void setup(void);
static void sbrk_verify(void);
static void cleanup(void);

static long increment = INC;

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			sbrk_verify();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	void *ret = NULL;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* call sbrk until it fails or increment overflows */
	while (ret != (void *)-1 && increment > 0) {
		ret = sbrk(increment);
		increment += INC;
	}
	tst_resm(TINFO | TERRNO, "setup() bailing inc: %ld, ret: %p, sbrk: %p",
		increment, ret, sbrk(0));

	errno = 0;
}

static void sbrk_verify(void)
{
	void *tret;

	tret = sbrk(increment);
	TEST_ERRNO = errno;

	if (tret != (void *)-1) {
		tst_resm(TFAIL,
			 "sbrk(%ld) returned %p, expected (void *)-1, errno=%d",
			 increment, tret, ENOMEM);
		return;
	}

	if (TEST_ERRNO == ENOMEM) {
		tst_resm(TPASS | TTERRNO, "sbrk(%ld) failed as expected",
			 increment);
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "sbrk(%ld) failed unexpectedly; expected: %d - %s",
			 increment, ENOMEM, strerror(ENOMEM));
	}
}

static void cleanup(void)
{
}
