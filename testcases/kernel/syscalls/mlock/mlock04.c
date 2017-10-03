/*
 * This is a reproducer copied from one of LKML patch submission,
 * which subject is
 *
 * [PATCH] mlock: revert the optimization for dirtying pages and triggering writeback.
 *
 * "In 5ecfda0, we do some optimization in mlock, but it causes
 * a very basic test case(attached below) of mlock to fail. So
 * this patch revert it with some tiny modification so that it
 * apply successfully with the lastest 38-rc2 kernel."
 *
 * Copyright (C) 2010  Red Hat, Inc.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include "test.h"
#include "safe_macros.h"
#include "config.h"

char *TCID = "mlock04";
int TST_TOTAL = 1;

#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

int fd, file_len = 40960;
char *testfile = "test_mlock";

static void setup(void);
static void cleanup(void);

int main(void)
{
	char *buf;
	int lc;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		buf = mmap(NULL, file_len, PROT_WRITE, MAP_SHARED, fd, 0);

		if (buf == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, cleanup, "mmap");

		if (mlock(buf, file_len) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "mlock");

		tst_resm(TINFO, "locked %d bytes from %p", file_len, buf);

		if (munlock(buf, file_len) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "munlock");

		SAFE_MUNMAP(cleanup, buf, file_len);
	}

	tst_resm(TPASS, "test succeeded.");

	cleanup();

	tst_exit();
}

static void setup(void)
{
	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, testfile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	SAFE_FTRUNCATE(cleanup, fd, file_len);

	TEST_PAUSE;
}

static void cleanup(void)
{
	close(fd);

	tst_rmdir();
}
