/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 11/21/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME :
 * sysconf01 :  test for sysconf( get configurable system variables) sys call.
 *
 * USAGE :
 *      sysconf01 
 *
 * RESTRICTIONS
 * MUST RUN AS ROOT
 *
 */

/*
 * Revision 1.2  2000/04/26  07:34:00  hegde
 * Added more test conditions for ssysconf(2) as it supports more config
 * variables due to unix98 changes.
 * PR#252257 scn:jgeorge11
 *
 * Revision 1.1  1996/05/14  22:01:18  vkohli
 * Initial revision
 *
*/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#define INVAL_FLAG	-1

/** LTP Port **/
#include "test.h"
#include "usctest.h"

char *TCID="sysconf01";            /* Test program identifier.    */
int TST_TOTAL=57;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */
/**************/


int main()
{

	int retval;


/*--------------------------------------------------------------------------*/

	retval = sysconf(_SC_CLK_TCK);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_CLK_TCK failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_ARG_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_ARG_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_CHILD_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_CHILD_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_OPEN_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_OPEN_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_JOB_CONTROL);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_JOB_CONTROL failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_SAVED_IDS);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_SAVED_IDS failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_VERSION);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_VERSION failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_PASS_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_PASS_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_LOGIN_NAME_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_LOGIN_NAME_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_XOPEN_VERSION);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_VERSION failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_TZNAME_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_TZNAME_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_STREAM_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_STREAM_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_XOPEN_CRYPT);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_CRYPT failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_XOPEN_ENH_I18N);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_ENH_I18N failed, error=%d",
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_XOPEN_SHM);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_SHM failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_XOPEN_XCU_VERSION);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_XCU_VERSION failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_ATEXIT_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_ATEXIT_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");
 
	retval = sysconf(_SC_2_C_BIND);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_C_BIND failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_C_DEV);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_C_DEV failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_C_VERSION);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_C_VERSION failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_CHAR_TERM);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_CHAR_TERM failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_FORT_DEV);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_FORT_DEV failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_FORT_RUN);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_FORT_RUN failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_LOCALEDEF);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_LOCALEDEF failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_SW_DEV);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_SW_DEV failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_UPE);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_UPE  failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_2_VERSION);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_2_VERSION failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_BC_BASE_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_BC_BASE_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_BC_DIM_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_BC_DIM_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_BC_SCALE_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_BC_SCALE_MAXd failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_BC_STRING_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_BC_STRING_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_COLL_WEIGHTS_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_COLL_WEIGHTS_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_EXPR_NEST_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_EXPR_NEST_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_LINE_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_LINE_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_RE_DUP_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_RE_DUP_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_XOPEN_UNIX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_UNIX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_PAGESIZE);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_PAGESIZE failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_PHYS_PAGES);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_PHYS_PAGES failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_AVPHYS_PAGES);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_AVPHYS_PAGES failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_AIO_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_AIO_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_AIO_PRIO_DELTA_MAX);
	if (errno != 0) {
		tst_resm(TFAIL,
			"sysconf _SC_AIO_PRIO_DELTA_MAX failed, error=%d",
			errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_SEMAPHORES);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_SEMAPHORES failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_SEM_NSEMS_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_SEM_NSEMS_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_SEM_VALUE_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_SEM_VALUE_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_MEMORY_PROTECTION);
	if (errno != 0) {
		tst_resm(TFAIL,
			"sysconf _SC_MEMORY_PROTECTION failed, error=%d",
			errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_FSYNC);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_FSYNC failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_MEMORY_PROTECTION);
	if (errno != 0) {
		tst_resm(TFAIL,
			"sysconf _SC_MEMORY_PROTECTION failed, error=%d",
			errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_TIMERS);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_TIMERS failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_TIMER_MAX);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_TIMER_MAX failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_MAPPED_FILES);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_MAPPED_FILES failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_THREAD_PRIORITY_SCHEDULING);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_THREAD_PRIORITY_SCHEDULING failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_XOPEN_LEGACY);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_XOPEN_LEGACY failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_MEMLOCK);
	if (errno != 0) {
		tst_resm(TFAIL, "sysconf _SC_MEMLOCK failed, error=%d", 
						 errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_XBS5_ILP32_OFF32);
	if (errno != 0) {
		tst_resm(TFAIL,
			"sysconf _SC_XBS5_ILP32_OFF32 failed, error=%d",
			errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(_SC_XBS5_ILP32_OFFBIG);
	if (errno != 0) {
		tst_resm(TFAIL,
			"sysconf _SC_XBS5_ILP32_OFFBIG failed, error=%d", 
			errno);
	} else tst_resm(TPASS,"PASS");

	retval = sysconf(INVAL_FLAG);
	if ((retval != -1) && (errno != EINVAL)) {
		tst_resm(TFAIL,
			"sysconf succeeds for invalid flag value = %d, errno = %d",
			retval, errno);
	} else tst_resm(TPASS,"PASS");

/*--------------------------------------------------------------------------*/
	tst_exit() ;

	/**NOT REACHED**/
	return(0);
}
