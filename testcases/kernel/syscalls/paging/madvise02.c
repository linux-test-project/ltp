/***************************************************************************
 *            madvise2.c
 *
 *  Fri May 14 17:23:19 2004
 *	Copyright (c) International Business Machines  Corp., 2004
 *  Email	sumit@in.ibm.com
 ****************************************************************************/

/*
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
 
/**********************************************************
 * 
 *    OS Test - International Business Machines Corp. 2004.
 * 
 *    TEST IDENTIFIER	: madvise02
 * 
 *    EXECUTED BY		: anyone
 * 
 *    TEST TITLE		: Basic test for madvise(2)
 * 
 *    TEST CASE TOTAL	: 7
 * 
 *    CPU TYPES			: Intel(R) XEON(TM)
 * 
 *    AUTHOR			: Sumit Sharma
 * 
 *    CO-PILOT			: 
 * 
 *    DATE STARTED		: 14/05/2004
 * 
 *    TEST CASES
 * 
 * 	1.) madvise(2) error conditions...(See Description)
 *	
 *	INPUT SPECIFICATIONS
 * 		The standard options for system call tests are accepted.
 *		(See the parse_opts(3) man page).
 * 
 *	OUTPUT SPECIFICATIONS
 *		Output describing whether test cases passed or failed.
 * 	
 *	ENVIRONMENTAL NEEDS		(??)
 *		None 
 *
 *	SPECIAL PROCEDURAL REQUIREMENTS
 * 		None
 * 
 *	DETAILED DESCRIPTION
 *		This is a test for the madvise(2) system call. It is intended
 *		to provide a complete exposure of the system call. It tests madvise(2) for
 *		all error conditions to occur correctly.
 * 
 * 		(1) Test Case for EINVAL
 *       	a. length is negative
 *       	b. start is not page-aligned
 *       	c. advice is not a valid value
 *       	d. application is attempting to release
 *			   locked or shared pages (with MADV_DONTNEED)
 *
 * 		(2) Test Case for ENOMEM
 *       	a. addresses in the specified range are not currently mapped
 * 			   or are outside the address space of the process
 *		 	b. Not enough memory - paging in failed
 *
 * 		(3) Test Case for EIO
 *       	a. Paging in this area would exceed
 *			   the process's maximum resident size
 *
 * 		(4) Test Case for EBADF
 *       	a. the map exists,
 *			   but the area maps something that isn't a file.
 *
 * 		(5) Test Case for EAGAIN
 *       	a. a kernel resource was temporarily unavailable.
 *
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 * 
 *	Test:
 *		Loop if the proper options are given.
 *		Execute system call
 *		Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *		Otherwise, Issue a PASS message.
 * 
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 * 
 * 
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>		/* For fstat */
#include <sys/stat.h>		/* For fstat */
#include <unistd.h>			/* For fstat & getpagesize() */
#include <sys/mman.h>		/* For madvise */
#include <fcntl.h>
#include <sys/time.h>		/* For rlimit */
#include <sys/resource.h>	/* For rlimit */

#include "test.h"
#include "usctest.h"

/* Uncomment the following line in DEBUG mode */
//#define MM_DEBUG 1
#define MM_RLIMIT_RSS 5		/* Max limit for RSS (i.e. Max no of pages resident in RAM) */

void setup(void);
void cleanup(void);
void check_and_print(int expected_errno);

char *TCID = "madvise02";	/* Test program modifier */
int TST_TOTAL = 7;			/* Total no of test cases */
extern int Tst_count;		/* Test case counter for tst_* routines */

/* Global variables */
int i;		/* Loop Counters */

int main(int argc, char *argv[])
{
	int lc, fd, pagesize;
	char *file;
	struct stat stat;
	char *ptr_memory_allocated = NULL;
	char *tmp_memory_allocated = NULL;
	struct rlimit rlim;	/* For getting rlimit for DATA */
	int no_of_file_pages=0;	/* Total no of pages in file */

	char *msg=NULL;
	char filename[64];
	char *progname=NULL;
	char *str_for_file="abcdefghijklmnopqrstuvwxyz12345\n";	/* 32-byte string */
	
	
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) != (char *) NULL)
	{
		tst_brkm(TBROK, NULL,
			"OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}
	
	/**************************************************
	 *	Perform global setup for test
	 **************************************************/
	setup();
	
	/* Creating file in tmp directory for testing */
	progname = *argv;
	sprintf(filename, "%s-out.%d", progname, getpid());
	
	for(lc = 0; TEST_LOOPING(lc); lc++)
	{
		/* Reset Tst_count in case we are looping */
		Tst_count = 0;

		/* Create a temporary file for testing */
		if ((fd = open (filename, O_RDWR | O_CREAT, 0664)) < 0)
		{
			tst_brkm(TBROK, cleanup,
				"Could not open file \"%s\" with O_RDWR",
				filename);
		}
#ifdef MM_DEBUG
		tst_resm(TINFO,
			"filename = %s opened successfully",
			filename);
#endif

		/* Writing 40 KB of random data into this file
		   [32 * 1280 = 40960] */
		for(i=0; i<1280; i++)
		{	
			if(write(fd, str_for_file, strlen(str_for_file)) < 0)
			{
				tst_brkm(TBROK, cleanup,
					"Could not write data to file \"%s\"",
					filename);
			}
		}

		/* Get file status for its size */
		if(fstat(fd, &stat) < 0)
		{
			tst_brkm(TBROK, cleanup,
				"Could not stat file \"%s\"",
				filename);
		}

		/* Map the input file into memory */
		if ((file =
    	   (char *) mmap (NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char *)-1)
		{
			tst_brkm(TBROK, cleanup,
				"Could not mmap file \"%s\"",
				filename);
		}

		pagesize=getpagesize();

#ifdef MM_DEBUG
		tst_resm(TINFO,
			"The Page size is %d",
			pagesize);
#endif

		/* Test Case 1a */
        TEST(madvise(file, -100, MADV_NORMAL));
		check_and_print(EINVAL);
		
        /* Test Case 1b */
        TEST(madvise(file+100, stat.st_size, MADV_NORMAL));
		check_and_print(EINVAL);
        
        /* Test Case 1c */
        TEST(madvise(file,stat.st_size,1212));
		check_and_print(EINVAL);

        /* Test Case 1d */
		if(mlock((void *)file,stat.st_size)<0)
		{
			tst_brkm(TBROK, cleanup,
				"Error in getting memory "
				"lock for the requested page(s)");
		}
	
        TEST(madvise(file,stat.st_size,MADV_DONTNEED));
		check_and_print(EINVAL);

        if(munmap(file,stat.st_size) < 0)
        {
            tst_brkm(TBROK, cleanup,
				"Error %d in munmap : %s",
				errno, strerror(errno));
        }

        /* Test Case 2a */
        if ((file =
       	(char *) mmap (NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char *)-1)
        {
            tst_brkm(TBROK, cleanup,
				"Could not mmap file");
        }

        TEST(madvise(file,stat.st_size + 40960,MADV_NORMAL));
		check_and_print(ENOMEM);

		/* Test Case 3a */
		/* Set new Process RSS (in no of pages)
		   for the current process */
		if(stat.st_size == 0)
		{
			tst_brkm(TBROK, cleanup,
				"File size is 0, "
				"test can not proceed\n");
		}
		
		no_of_file_pages = (stat.st_size / pagesize);
		if(stat.st_size % pagesize != 0)
		{
			no_of_file_pages++;
		}
		
		if(getrlimit(RLIMIT_RSS, &rlim) < 0)
		{
			tst_brkm(TBROK, cleanup,
				"Error %d in getrlimit : %s",
				errno, strerror(errno));
		}

#ifdef MM_DEBUG
		tst_resm(TINFO,
			"RLIM_INFINITY :: %d\n",
			RLIM_INFINITY);
		tst_resm(TINFO,
			"Current RSS [soft limit :: %d, "
			"hard limit :: %d]",
			rlim.rlim_cur, rlim.rlim_max);
#endif
	
		/* Set the rlimit for RSS to half of the actual file size */
		rlim.rlim_cur = no_of_file_pages / 2;
	
		if(setrlimit(RLIMIT_RSS, &rlim) < 0)
		{
			tst_brkm(TBROK, cleanup,
				"Error %d in setrlimit : %s",
				errno, strerror(errno));
		}
		else
		{
#ifdef MM_DEBUG
			tst_resm(TINFO,
				"New RSS rlimit has been set successfully");
#endif
		}

		if(getrlimit(RLIMIT_RSS, &rlim) < 0)
		{
			tst_brkm(TBROK, cleanup,
				"Error %d in getrlimit : %s",
				errno, strerror(errno));
		}

#ifdef MM_DEBUG
		tst_resm(TINFO,
			"RLIM_INFINITY :: %d",
			RLIM_INFINITY);
		tst_resm(TINFO,
			"Current RSS [soft limit is :: %d, "
			"hard limit :: %d]",
			rlim.rlim_cur, rlim.rlim_max);
#endif

		/* Now give advice to kernel
		   for keeping more than max RSS limit for the process */
        TEST(madvise(file,stat.st_size,MADV_WILLNEED));
		check_and_print(EIO);
		
		
		/* Test Case 4a */
		/* Create one memory segment using malloc */
		ptr_memory_allocated = (char *) malloc(5 * sizeof(pagesize));
		/* Take temporary pointer for later freeing up the original one */
		tmp_memory_allocated = ptr_memory_allocated;
		tmp_memory_allocated = (char *)(((int) tmp_memory_allocated + pagesize-1) & ~(pagesize-1));
		
		TEST(madvise(tmp_memory_allocated, 5 * pagesize, MADV_NORMAL));
		check_and_print(EBADF);

		free((void *)ptr_memory_allocated);
		
		/* Finally Unmapping the whole file */
		if(munmap(file,stat.st_size) < 0)
        {
                tst_brkm(TBROK, cleanup,
					"Error %d in munmap : %s",
					errno, strerror(errno));
        }
		
		close(fd);
	}
    cleanup();
	return 0;
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{
    /* capture signals */
    tst_sig(NOFORK, DEF_HANDLER, cleanup);

    /* Pause if that option was specified */
    TEST_PAUSE;

	/* Create temp directory and change to that */
	tst_tmpdir();
}	/* End setup() */


/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{
    /*
     * print timing stats if that option was specified.
     * print errno log if that option was specified.
     */
    TEST_CLEANUP;
	
	/* Remove temp directory and files */
	tst_rmdir();

    /* exit with return code appropriate for results */
    tst_exit();

}	/* End cleanup() */

/***************************************************************
 * check_and_print(extected_errno) - checks the returned value of call
 *		and tests whether the returned errno is the expected errno or not
 *		and prints the appropriate messages.
 ***************************************************************/
void check_and_print(int expected_errno)
{
	if(TEST_RETURN == -1)
	{
       	if(TEST_ERRNO == expected_errno)
		{
            tst_resm(TPASS,
				"expected failure - errno = %d : %s",
				TEST_ERRNO, strerror(TEST_ERRNO));
		}
		else
		{
           	tst_resm(TFAIL,
				"madvise failed with wrong errno, "
				"expected errno = %d, "
				"got errno = %d : %s",
				EINVAL, TEST_ERRNO, strerror(TEST_ERRNO));
		}
	}
       else
	{
		tst_resm(TFAIL,
			"madvise failed, expected "
			"return value = -1, got %d",
			TEST_RETURN);
 	}
}
