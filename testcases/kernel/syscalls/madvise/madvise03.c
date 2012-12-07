/*
 *  Copyright (c) International Business Machines  Corp., 2004
 *  Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
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

/*
 * This is a test case for madvise(2) system call. No error should be returned.
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

char *TCID = "madvise03";

#ifdef MADV_REMOVE

/* Uncomment the following line in DEBUG mode */
//#define MM_DEBUG 1

int TST_TOTAL = 3;

static void setup(void);
static void cleanup(void);
static void check_and_print(char *advice);
static long get_shmmax(void);

static int shmid1;

int main(int argc, char *argv[])
{
	int lc, fd;
	int i;
	char *file = NULL;
	struct stat stat;
	void *addr1;
	long shm_size = 0;

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
				tst_brkm(TBROK | TERRNO, cleanup,
					 "write failed");

		if (fstat(fd, &stat) == -1)
			tst_brkm(TBROK, cleanup, "fstat failed");

		file = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (file == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, cleanup, "mmap failed");

		/* Allocate shared memory segment */
		shm_size = get_shmmax();

#define min(a, b) ((a) < (b) ? (a) : (b))
		shmid1 = shmget(IPC_PRIVATE, min(1024 * 1024, shm_size),
				IPC_CREAT | IPC_EXCL | 0701);
		if (shmid1 == -1)
			tst_brkm(TBROK, cleanup, "shmget failed");

		/* Attach shared memory segment to an address selected by the system */
		addr1 = shmat(shmid1, NULL, 0);
		if (addr1 == (void *)-1)
			tst_brkm(TBROK, cleanup, "shmat error");

		/* (1) Test case for MADV_REMOVE */
		TEST(madvise(addr1, 4096, MADV_REMOVE));
		check_and_print("MADV_REMOVE");

		/* (2) Test case for MADV_DONTFORK */
		TEST(madvise(file, (stat.st_size / 2), MADV_DONTFORK));
		check_and_print("MADV_DONTFORK");

		/* (3) Test case for MADV_DOFORK */
		TEST(madvise(file, (stat.st_size / 2), MADV_DOFORK));
		check_and_print("MADV_DOFORK");

		/* Finally Unmapping the whole file */
		if (munmap(file, stat.st_size) < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "munmap failed");

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
	if (shmid1 != -1)
		if (shmctl(shmid1, IPC_RMID, 0) < 0)
			tst_resm(TBROK | TERRNO,
				 "shmctl(.., IPC_RMID, ..) failed");

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

static long get_shmmax(void)
{
	long maxsize;

	SAFE_FILE_SCANF(cleanup, "/proc/sys/kernel/shmmax", "%ld", &maxsize);

	return maxsize;
}
#else
int main(void)
{
	/* "Requires 2.6.16+" were the original comments */
	tst_brkm(TCONF, NULL,
		 "this system doesn't have required madvise support");
}
#endif
