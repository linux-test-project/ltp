/*************************************************************************
 * Copyright (c) Crackerjack Project., 2007
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 ************************************************************************/
/************************************************************************/
/*									*/
/* File:	set_thread_area_02.c					*/
/*									*/
/* Description: This test checks for propper error code on syscalls	*/
/*		1) get__thread_area():					*/
/*		  get_thread_area returns an entry in the current	*/
/*		  thread's Thread Local Storage (TLS) array.		*/
/*		  get_thread_area returns 0 on success. Otherwise,	*/
/*		  it returns -1						*/
/*		2)  set_thread_area():					*/
/*		 set_thread_area() sets an entry in the current		*/
/*		 thread's Thread Local. When set_thread_area() is	*/
/*		 passed an entry_number of -1, it uses a free TLS	*/
/*		 entry. set_thread_area() returns 0 on success, and	*/
/*		 -1 on failure with errno set appropriately.		*/
/*									*/
/*									*/
/* Usage:  <for command-line>						*/
/* set_thread_area_02 [-c n] [-e][-i n] [-I x] [-p x] [-t]		*/
/*	  where,  -c n : Run n copies concurrently.			*/
/*		  -e   : Turn on errno logging.				*/
/*		  -i n : Execute test n times.				*/
/*		  -I x : Execute test for x seconds.			*/
/*		  -P x : Pause for x seconds between iterations.		*/
/*		  -t   : Turn on syscall timing.				*/
/*									*/
/* Total Tests: 3							*/
/*									*/
/* Test Name:   set_thread_area_02					*/
/* History:	 Porting from Crackerjack to LTP is done by		*/
/*		Manas Kumar Nayak maknayak@in.ibm.com>			*/
/************************************************************************/
#include "set_thread_area.h"

/* Global Variables */
char *TCID = "set_thread_area_02";  /* Test program identifier.*/
int  TST_TOTAL = 3;		   /* total number of tests in this file.   */

#if defined(HAVE_ASM_LDT_H) && defined(HAVE_STRUCT_USER_DESC)
/* Extern Global Variables */

extern void cleanup()
{
	TEST_CLEANUP;
	tst_rmdir();
}

void setup()
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{
	thread_area_s u_info;

	setup();

	u_info.entry_number = 6;

	TEST(syscall(__NR_get_thread_area, &u_info));	 //call get_thread_area()
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL|TTERRNO, cleanup, "call get_thread_area() failed");
	}

	u_info.entry_number = -2;
	TEST(syscall(__NR_set_thread_area, &u_info));	 //call set_thread_area()
	if (TEST_RETURN == -1) {

		if (TEST_ERRNO == EINVAL) {
			tst_resm(TPASS, "set_thread_area failed with EINVAL as expected");
		} else {
	 		tst_resm(TFAIL|TTERRNO, "set_thread_area didn't fail with EINVAL");
		}

	} else {
   		tst_brkm(TFAIL, cleanup, "set_thread_area succeeded unexpectedly");
	}

	TEST(syscall(__NR_set_thread_area,(struct user_desc *)-9));	 //call set_thread_area()
	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == EFAULT) {
			tst_resm(TPASS, "set_thread_area failed with EFAULT as expected");
		} else {
			tst_resm(TFAIL|TTERRNO, "set_thread_area didn't fail with EFAULT");
		}
	} else {
		tst_resm(TFAIL, "set_thread_area succeeded unexpectedly");
	}
	cleanup();
	tst_exit();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "set_thread_area isn't available for this architecture");
	tst_exit();
}
#endif
