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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
#include "usctest.h"
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* definitions for open()                               */
#include <sys/mman.h> /* definitions for mmap()                               */
#include <fcntl.h>    /* definition of open()                                 */
#include <sys/user.h>

#define FAILED       (-1) /* return status for all funcs indicating failure   */
#define SUCCESS      0    /* return status for all routines indicating success*/

char *TCID = "mem03";			/* Test program identifier. */
int TST_TOTAL = 1;			/* Total number of test cases. */
extern int Tst_count;			/* Test Case counter for tst_* routines */


/*--------------------------------------------------------------------*/
int main()
{
	int f1, f2;
	char tmp1[] = "./tmp.file.1";
	char tmp2[] = "./tmp.file.2";

	char str1[] = "testing 123";
	char str2[] = "my test mem";

	if ((f1 = open(tmp1, O_RDWR|O_CREAT, S_IREAD|S_IWRITE))
			== -1 )
        {
		tst_resm(TFAIL, "failed to open/create file %s\n", tmp1);
		return FAILED;
        }

	if ((f2 = open(tmp2, O_RDWR|O_CREAT, S_IREAD|S_IWRITE))
			== -1 )
        {
		tst_resm(TFAIL, "failed to open/create file %s\n", tmp1);
		return FAILED;
        }

	write(f1, str1, strlen(str1));
	write(f2, str2, strlen(str2));

	{
		char *mm1, *mm2;
		char *save_mm1, *save_mm2;

		mm1 = mmap(0, 64, PROT_READ, MAP_PRIVATE, f1, 0);
		mm2 = mmap(0, 64, PROT_READ, MAP_PRIVATE, f2, 0);

		if ((mm1 == (void*)-1) || (mm2 == (void*)-1))
		{
			tst_resm(TFAIL, "mmap failed\n");
			return FAILED;
		}

		//printf("mm1 = %p\n", mm1);
		//printf("mm2 = %p\n", mm2);

		save_mm1 = mm1;
		save_mm2 = mm2;

		if ( strncmp(str1, mm1, strlen(str1)) )
		{
			tst_resm(TFAIL, "failed on compare %s\n", tmp1);
			return FAILED;
		}

		if ( strncmp(str2, mm2, strlen(str2)) )
		{
			tst_resm(TFAIL, "failed on compare %s\n", tmp2);
			return FAILED;
		}

		munmap(mm1, 64);
		munmap(mm2, 64);

		mm1 = mmap(save_mm2, 64, PROT_READ, MAP_PRIVATE, f1, 0);
		mm2 = mmap(save_mm1, 64, PROT_READ, MAP_PRIVATE, f2, 0);

		if ((mm1 == (void*)-1) || (mm2 == (void*)-1))
		{
			tst_resm(TFAIL, "second mmap failed\n");
			return FAILED;
		}

		//printf("mm1 = %p\n", mm1);
		//printf("mm2 = %p\n", mm2);

		if (mm1 != save_mm2)
		{
			printf("mmap not using same address\n");
			return 0;
		}

		if (mm2 != save_mm1)
		{
			printf("mmap not using same address\n");
			return 0;
		}

		if ( strncmp(str1, mm1, strlen(str1)) )
		{
			tst_resm(TFAIL, "failed on compare %s\n", tmp1);
			return FAILED;
		}

		if ( strncmp(str2, mm2, strlen(str2)) )
		{
			tst_resm(TFAIL, "failed on compare %s\n", tmp2);
			return FAILED;
		}

		munmap(mm1, 64);
		munmap(mm2, 64);
	}

	close(f1);
	close(f2);

        if (unlink(tmp1))
        {
            perror("1 ulink()");
            return FAILED;
        }

        if (unlink(tmp2))
        {
            perror("2 ulink()");
            return FAILED;
        }

	tst_resm(TPASS,"%s memory test succeeded", TCID);
	return SUCCESS;
}

