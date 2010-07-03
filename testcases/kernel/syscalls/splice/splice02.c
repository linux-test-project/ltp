/******************************************************************************/
/* Copyright (c) Jens Axboe <axboe@kernel.dk>, 2009                           */ 
/*                                                                            */
/* LKML Reference: http://lkml.org/lkml/2009/4/2/55                           */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        splice02.c                                                    */
/*                                                                            */
/* Description: This tests the splice() syscall                               */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* echo "Test splice()" > <outfile>; splice02 <outfile>                       */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   splice02                                                      */
/******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "splice02";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;                   /* total number of tests in this file.   */

static inline long ltp_splice(int fd_in, loff_t *off_in,
				int fd_out, loff_t *off_out,
				size_t len, unsigned int flags)
{
			return syscall(__NR_splice, fd_in, off_in, fd_out,
					off_out, len, flags);
}


/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup() {
        /* Remove tmp dir and all files in it */
        TEST_CLEANUP;
        tst_rmdir();

        /* Exit with appropriate return code. */
        tst_exit();
}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup() {
        /* Capture signals if any */
        /* Create temporary directories */
        TEST_PAUSE;
        tst_tmpdir();
}

#define SPLICE_SIZE (64*1024)

int main(int ac, char **av) {
	int fd = 0;
	int results = 0;

	/* Disable test if the version of the kernel is less than 2.6.17 */
	if (((results = tst_kvercmp(2, 6, 17)) < 0)) {
		tst_resm(TINFO, "This test can only run on kernels that are ");
		tst_resm(TINFO, "2.6.17 and higher");
		exit(0);
	}

        setup();

        if (ac < 2 ) {
            tst_resm(TFAIL, "%s failed - Usage: %s outfile", TCID, av[0]);
            tst_exit();
	}
	fd=open(av[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd < 0 ) {
		tst_resm(TFAIL, "open(%s) failed - errno = %d : %s", av[1], errno, strerror(errno));
	   cleanup();
	   tst_exit();
	}
			
        do {
		TEST(ltp_splice(STDIN_FILENO, NULL, fd, NULL, SPLICE_SIZE, 0));
	    if (TEST_RETURN < 0) {
            	tst_resm(TFAIL, "splice failed - errno = %d : %s", TEST_ERRNO, strerror(TEST_ERRNO));
	        cleanup();
		tst_exit();
	    } else
            if (TEST_RETURN == 0){
							tst_resm(TPASS, "splice() system call Passed");
		close(fd);
	        cleanup();
	        tst_exit();
	    }
	} while(1);
}

