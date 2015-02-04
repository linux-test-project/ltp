/*
 *
 *   Copyright (c) National ICT Australia, 2006
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

/* NICTA */
/* 13/02/2006	Implemented	carl.vanschaik at nicta.com.au */

/* mem03.c */
/*
 * NAME
 * 	mem03.c
 *
 * DESCRIPTION
 *      - create two files, write known data to the files.
 *      - mmap the files, verify data
 *      - unmap files
 *      - remmap files, swap virtual addresses ie: file1 at file2's address, etc
 *
 * REASONING
 *      - If the kernel fails to correctly flush the TLB entry, the second mmap
 *        will not show the correct data.
 *
 *
 * RESTRICTIONS
 * 	None
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "test.h"
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>		/* definitions for open()                               */
#include <sys/mman.h>		/* definitions for mmap()                               */
#include <fcntl.h>		/* definition of open()                                 */
#include <sys/user.h>

#define FAILED       (-1)	/* return status for all funcs indicating failure   */
#define SUCCESS      0		/* return status for all routines indicating success */

static void setup();
static void cleanup();

char *TCID = "mem03";
int TST_TOTAL = 1;

int f1 = -1, f2 = -1;
char *mm1 = NULL, *mm2 = NULL;

/*--------------------------------------------------------------------*/
int main(void)
{
	char tmp1[] = "./tmp.file.1";
	char tmp2[] = "./tmp.file.2";

	char str1[] = "testing 123";
	char str2[] = "my test mem";

	setup();

	if ((f1 = open(tmp1, O_RDWR | O_CREAT, S_IREAD | S_IWRITE)) == -1)
		tst_brkm(TFAIL, cleanup, "failed to open/create file %s", tmp1);

	if ((f2 = open(tmp2, O_RDWR | O_CREAT, S_IREAD | S_IWRITE)) == -1)
		tst_brkm(TFAIL, cleanup, "failed to open/create file %s", tmp2);

	write(f1, str1, strlen(str1));
	write(f2, str2, strlen(str2));

	{
		char *save_mm1, *save_mm2;

		mm1 = mmap(0, 64, PROT_READ, MAP_PRIVATE, f1, 0);
		mm2 = mmap(0, 64, PROT_READ, MAP_PRIVATE, f2, 0);

		if ((mm1 == (void *)-1) || (mm2 == (void *)-1))
			tst_brkm(TFAIL, cleanup, "mmap failed");

		save_mm1 = mm1;
		save_mm2 = mm2;

		if (strncmp(str1, mm1, strlen(str1)))
			tst_brkm(TFAIL, cleanup, "failed on compare %s", tmp1);

		if (strncmp(str2, mm2, strlen(str2)))
			tst_brkm(TFAIL, cleanup, "failed on compare %s", tmp2);

		munmap(mm1, 64);
		munmap(mm2, 64);

		mm1 = mmap(save_mm2, 64, PROT_READ, MAP_PRIVATE, f1, 0);
		mm2 = mmap(save_mm1, 64, PROT_READ, MAP_PRIVATE, f2, 0);

		if ((mm1 == (void *)-1) || (mm2 == (void *)-1))
			tst_brkm(TFAIL, cleanup, "second mmap failed");

		if (mm1 != save_mm2) {
			printf("mmap not using same address\n");

		}

		if (mm2 != save_mm1) {
			printf("mmap not using same address\n");

		}

		if (strncmp(str1, mm1, strlen(str1)))
			tst_brkm(TFAIL, cleanup, "failed on compare %s", tmp1);

		if (strncmp(str2, mm2, strlen(str2)))
			tst_brkm(TFAIL, cleanup, "failed on compare %s", tmp2);

		munmap(mm1, 64);
		munmap(mm2, 64);
	}

	tst_resm(TPASS, "%s memory test succeeded", TCID);

	/* clean up and exit */
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/*
	 * Create a temporary directory and cd into it.
	 */
	tst_tmpdir();
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	if (mm1)
		munmap(mm1, 64);
	if (mm2)
		munmap(mm2, 64);

	if (f1 != -1)
		close(f1);
	if (f2 != -1)
		close(f2);

	tst_rmdir();

}
