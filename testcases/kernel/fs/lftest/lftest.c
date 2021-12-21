// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2001, International Business Machines  Corp.
 * Copyright (c) 2020, Joerg Vehlow <joerg.vehlow@aox-tech.de>
 *
 * The purpose of this test is to verify the file size limitations
 * of a filesystem. It writes one buffer at a time and lseeks from
 * the beginning of the file to the end of the last write position.
 * The intent is to test lseek64.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "tst_test.h"

static char *str_bufnum;
static int bufnum = 100;
static char buf[TST_MB];

static void setup(void)
{
	unsigned int i;

	if (str_bufnum) {
		if (tst_parse_int(str_bufnum, &bufnum, 0, INT_MAX)) {
			tst_brk(TBROK, "Invalid buffer count '%s'", str_bufnum);
		}
	}

	buf[0] = 'A';
	for (i = 1; i < ARRAY_SIZE(buf) - 1; i++)
		buf[i] = '0';
	buf[ARRAY_SIZE(buf) - 1] = 'Z';
}

static void run(void)
{
	time_t time1, time2;
	int i, fd, diff;

	time1 = time(NULL);
	tst_res(TINFO, "started building a %d megabyte file", bufnum);

	if ((fd = creat("large_file", 0755)) == -1)
		tst_brk(TBROK | TERRNO, "creat() failed");

	for (i = 0; i < bufnum; i++) {
		if (write(fd, buf, sizeof(buf)) == -1) {
			tst_brk(TFAIL | TERRNO, "write() failed");
		}
		fsync(fd);
		if (lseek(fd, (i + 1) * sizeof(buf), 0) == -1)
			tst_brk(TFAIL | TERRNO, "lseek failed");
	}
	close(fd);
	time2 = time(NULL);
	tst_res(TINFO, "finished building a %d megabyte file", bufnum);
	diff = time2 - time1;
	tst_res(TINFO, "total time for test to run: %d minute(s) and %d seconds",
	        diff / 60, diff % 60);

	tst_res(TPASS, "test successfull");
}

static struct tst_test test = {
	.options = (struct tst_option[]) {
		{"n:", &str_bufnum, "COUNT Number of megabytes to write (default 100)"},
		{}
	},
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = run
};
