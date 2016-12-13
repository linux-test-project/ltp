/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: stat06.c,v 1.10 2009/11/02 13:57:19 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: stat06
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: stat(2) negative path testcases
 *
 *    PARENT DOCUMENT	: None
 *
 *    TEST CASE TOTAL	: 7
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Richard Logan
 *
 *    CO-PILOT		: William Roske
 *
 *    DATE STARTED	: 03/30/94
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1-7) See Testcases structure below.
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *      -h  : print help and exit
 *
 *    OUTPUT SPECIFICATIONS
 *$
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 * 	None
 *
 *    ENVIRONMENTAL NEEDS
 * 	The libcuts.a and libsys.a libraries must be included in
 *	the compilation of this test.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    INTERCASE DEPENDENCIES
 * 	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the stat(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	stat(2).
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "stat06";

char *bad_addr = 0;

#if !defined(UCLINUX)
int high_address_setup();
char High_address[64];
#endif
int longpath_setup();
int no_setup();
int filepath_setup();
char Longpathname[PATH_MAX + 2];
struct stat statbuf;
jmp_buf sig11_recover;
void sig11_handler(int sig);

struct test_case_t {
	char *pathname;
	struct stat *stbuf;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	"nonexistfile", &statbuf, "non-existent file", ENOENT, no_setup}, {
	"", &statbuf, "path is empty string", ENOENT, no_setup}, {
	"nefile/file", &statbuf, "path contains a non-existent file",
		    ENOENT, no_setup}, {
	"file/file", &statbuf, "path contains a regular file",
		    ENOTDIR, filepath_setup}, {
	Longpathname, &statbuf, "pathname too long", ENAMETOOLONG,
		    longpath_setup},
#if !defined(UCLINUX)
	{
	High_address, &statbuf, "address beyond address space", EFAULT,
		    high_address_setup}, {
	(char *)-1, &statbuf, "negative address", EFAULT, no_setup},
#endif
	{
	NULL, NULL, NULL, 0, no_setup}
};

int TST_TOTAL = ARRAY_SIZE(Test_cases);

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;
	char *fname;
	char *desc;
	int ind;
	struct stat *stbuf;
	struct sigaction sa, osa;

    /***************************************************************
     * parse standard options
     ***************************************************************/
	tst_parse_opts(ac, av, NULL, NULL);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {

			fname = Test_cases[ind].pathname;
			desc = Test_cases[ind].desc;
			stbuf = Test_cases[ind].stbuf;

			if (stbuf == (struct stat *)-1) {
				/* special sig11 case */
				sa.sa_handler = &sig11_handler;
				sigemptyset(&sa.sa_mask);
				sa.sa_flags = 0;

				sigaction(SIGSEGV, NULL, &osa);
				sigaction(SIGSEGV, &sa, NULL);

				if (setjmp(sig11_recover)) {
					TEST_RETURN = -1;
					TEST_ERRNO = EFAULT;
				} else {
					TEST(stat(fname, stbuf));
				}
				sigaction(SIGSEGV, &osa, NULL);
			} else {
				/*
				 *  Call stat(2)
				 */

				TEST(stat(fname, stbuf));
			}

			/* check return code */
			if (TEST_RETURN == -1) {
				if (TEST_ERRNO ==
				    Test_cases[ind].exp_errno)
					tst_resm(TPASS,
						 "stat(<%s>, &stbuf) Failed, errno=%d",
						 desc, TEST_ERRNO);
				else
					tst_resm(TFAIL,
						 "stat(<%s>, &stbuf) Failed, errno=%d, expected errno:%d",
						 desc, TEST_ERRNO,
						 Test_cases
						 [ind].exp_errno);
			} else {
				tst_resm(TFAIL,
					 "stat(<%s>, &stbuf) returned %ld, expected -1, errno:%d",
					 desc, TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}

	}

	cleanup();
	tst_exit();
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{
	int ind;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	Test_cases[6].pathname = bad_addr;
#endif

	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}

}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{

	tst_rmdir();

}

/******************************************************************
 * no_setup() - does nothing
 ******************************************************************/
int no_setup(void)
{
	return 0;
}

#if !defined(UCLINUX)

/******************************************************************
 * high_address_setup() - generates an address that should cause a segfault
 ******************************************************************/
int high_address_setup(void)
{
	int ind;

	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		if (Test_cases[ind].pathname == High_address) {
			/*if (strcmp(Test_cases[ind].pathname, HIGH_ADDRESS) == 0) { ** */
			Test_cases[ind].pathname = (char *)(sbrk(0) + 5);
			break;
		}
	}
	return 0;

}
#endif

/******************************************************************
 * longpath_setup() - creates a filename that is too long
 ******************************************************************/
int longpath_setup(void)
{
	int ind;

	for (ind = 0; ind <= PATH_MAX + 1; ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;

}

/******************************************************************
 * filepath_setup() creates a file the exists that we will treat as a directory
 ******************************************************************/
int filepath_setup(void)
{
	int fd;

	if ((fd = creat("file", 0777)) == -1) {
		tst_brkm(TBROK, cleanup, "creat(file) failed, errno:%d %s",
			 errno, strerror(errno));
	}
	close(fd);
	return 0;
}

/******************************************************************
 * sig11_handler() - our segfault recover hack
 ******************************************************************/
void sig11_handler(int sig)
{
	longjmp(sig11_recover, 1);
}
