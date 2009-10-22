/******************************************************************************
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
 ******************************************************************************
 * File:	set_thread_area01.c
 * Description: This test two syscalls
 *		1) get__thread_area():
 *		   get_thread_area returns an entry in the current thread's
 *		   Thread Local Storage (TLS) array.get_thread_area returns 0
 *		   on success. Otherwise, it returns -1
 *		2) set_thread_area():
 *		   set_thread_area() sets an entry in the current thread's
 *		   Thread Local.When  set_thread_area() is passed an
 *		   entry_number of -1, it uses a free TLS entry.
 *		   set_thread_area() returns 0 on success, and -1 on failure
 *
 * Usage:  <for command-line>
 * set_tid_address01 [-c n] [-e][-i n] [-I x] [-p x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * Total Tests: 2
 * Test Name:   set_thread_area01
 * History:     Porting from Crackerjack to LTP is done by
 *		Manas Kumar Nayak maknayak@in.ibm.com>
 *****************************************************************************/

#include "set_thread_area.h"

/* Global Variables */
char *TCID = "set_thread_area01"; /* Test program identifier.		  */
int  testno;
int  TST_TOTAL = 2;		  /* total number of tests in this file.  */

#if defined(HAVE_ASM_LDT_H) && defined(HAVE_STRUCT_USER_DESC)
/* Extern Global Variables */
extern int Tst_count;	   /* counter for tst_xxx routines.	 */
extern char *TESTDIR;	   /* temporary dir created by tst_tmpdir() */

/* Extern Global Functions */
/************************************************************************/
/*									*/
/* Function:    cleanup							*/
/*									*/
/* Description: Performs all one time clean up for this test on		*/
/* successful completion, premature exit or failure. Closes all		*/
/* temporary files, removes all temporary directories exits the		*/
/* test with appropriate return code by calling tst_exit() function.	*/
/*									*/
/* Input:       None.							*/
/*									*/
/* Output:      None.							*/
/*									*/
/* Return:      On failure - Exits calling tst_exit(). Non '0' return	*/
/* 		code. On success - Exits calling tst_exit(). With '0'	*/
/*  		return code.						*/
/*									*/
/************************************************************************/
extern void cleanup() {
	/* Remove tmp dir and all files in it */
	TEST_CLEANUP;
	tst_rmdir();

	/* Exit with appropriate return code. */
	tst_exit();
}

/* Local  Functions */
/************************************************************************/
/*									*/
/* Function:    setup							*/
/*									*/
/* Description: Performs all one time setup for this test.		*/
/* This function is typically used to capture signals, create		*/
/* temporary dirs and temporary files that may be used in the		*/
/* course of this test.							*/
/*									*/
/* Input:       None.							*/
/*									*/
/* Output:      None.							*/
/*									*/
/* Return:      On failure - Exits by calling cleanup().		*/
/*			  On success - returns 0.			*/
/*									*/
/************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av) {

	thread_area_s u_info;
	int lc;		/* loop counter */
	char *msg;	/* message returned from parse_opts */
	
	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		 tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		 tst_exit();
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			u_info.entry_number = 6;
	
			/*
			 * This call to get_thread_area function should be
			 * successful.
			 */

			/* Call get_thread_area(). */
			TEST(syscall(__NR_get_thread_area, &u_info));
			if(TEST_RETURN == 0) {
				tst_resm(TPASS, "get_thread_area call succeeded");
			} else {
				tst_resm(TFAIL, "failed - errno = %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
				cleanup();
				tst_exit();
			}
		     
			u_info.entry_number = -1;
			/*
			 * This call to set_thread_area function should be sucessful.
			 */

			/* Call set_thread_area(). */
			TEST(syscall(__NR_set_thread_area, &u_info));

			if(TEST_RETURN == 0) {
				tst_resm(TPASS, "set_thread_area call succeeded");
			} else {
			     	tst_resm(TFAIL, "failed - errno = %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
				cleanup();
	  			tst_exit();
			}

		}

	}	

	cleanup();

	tst_exit();

}

#else
int main(void) {
	tst_resm(TCONF, "[gs]et_thread_area is not available on this "
			"architecture");
	tst_exit();
}
#endif
