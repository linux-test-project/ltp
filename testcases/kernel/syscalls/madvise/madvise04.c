/*
 *  Copyright (C) 2012 FUJITSU LIMITED.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"

char *TCID = "madvise04";

#ifdef MADV_DONTDUMP

/* Uncomment the following line in DEBUG mode */
//#define MM_DEBUG 1

int TST_TOTAL = 2;

#define BUFFER_SIZE  256

static void setup(void);
static void cleanup(void);
static void check_and_print(char *advice);

int main(int argc, char *argv[])
{
	int lc, fd;
	int i;
	char *file = NULL;
	struct stat stat;

	char *msg = NULL;
	char filename[64];
	char *progname = NULL;
	char *str_for_file = "abcdefghijklmnopqrstuvwxyz12345\n";

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	progname = *argv;
	sprintf(filename, "%s-out.%d", progname, getpid());

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		fd = open(filename, O_RDWR | O_CREAT, 0664);
		if (fd < 0)
			tst_brkm(TBROK, cleanup, "open failed");
#ifdef MM_DEBUG
		tst_resm(TINFO, "filename = %s opened successfully", filename);
#endif

		/* Writing 40 KB of random data into this file
		   [32 * 1280 = 40960] */
		for (i = 0; i < 1280; i++)
			if (write(fd, str_for_file, strlen(str_for_file)) == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "write failed");

		if (fstat(fd, &stat) == -1)
			tst_brkm(TBROK, cleanup, "fstat failed");

		file = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (file == MAP_FAILED)
			tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");

		/* (1) Test case for MADV_DONTDUMP */
		TEST(madvise(file, stat.st_size, MADV_DONTDUMP));
		check_and_print("MADV_DONTDUMP");

		/* (2) Test case for MADV_DODUMP */
		TEST(madvise(file, stat.st_size, MADV_DODUMP));
		check_and_print("MADV_DODUMP");

		/* Finally Unmapping the whole file */
		if (munmap(file, stat.st_size) < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "munmap failed");

		close(fd);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
	tst_tmpdir();
}

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();
}

static void check_and_print(char *advice)
{
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL,
			 "madvise test for %s failed with "
			 "return = %ld, errno = %d : %s",
			 advice, TEST_RETURN, TEST_ERRNO, strerror(TEST_ERRNO));
	} else if (STD_FUNCTIONAL_TEST) {
		tst_resm(TPASS, "madvise test for %s PASSED", advice);
	}
}

#else
int main(void)
{
	/* Requires kernel version >= 3.4 */
	tst_brkm(TCONF, NULL,
		 "This system doesn't have required madvise support, "
		 "MADV_DONTDUMP and MADV_DODUMP were added from 3.4. "
		 "If your kernel version >= 3.4, maybe you need updating "
		 "your glibc-headers");
}
#endif
