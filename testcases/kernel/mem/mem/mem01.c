/*
 * mem01.c - Basic memory and swapper stress test
 *
 * Copyright (C) 2001 Stephane Fillod <f4cfe@free.fr>
 * 	Original idea from Rene Cougnenc (on t'a pas oublié mec)
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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 * 
 *    TEST IDENTIFIER	: mem01
 * 
 *    EXECUTED BY	: anyone
 * 
 *    TEST TITLE	: Basic memory and swapper stress test
 * 
 *    PARENT DOCUMENT	: ??
 * 
 *    TEST CASE TOTAL	: 1
 * 
 *    WALL CLOCK TIME	: 1
 * 
 *    CPU TYPES		: ALL
 * 
 *    AUTHOR		: Stephane Fillod
 * 
 *    CO-PILOT		: ??
 * 
 *    DATE STARTED	: 03/24/01
 * 
 *    INITIAL RELEASE	: Linux 2.2
 * 
 *    TEST CASES
 * 
 * 	1.) malloc(3) returns...(See Description)
 *	
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 * 
 *    OUTPUT SPECIFICATIONS
 * 	
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
 *      No run-time environmental needs.
 * 
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 * 
 *    INTERCASE DEPENDENCIES
 * 	None
 * 
 *    DETAILED DESCRIPTION
 *	This is a Phase ?? test for exercising virtual memory allocation
 *	and usage. It is intended to provide a limited exposure of the 
 *	system swapper, for now.
 * 
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 * 
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute memory allocation (sbrk(2) involved)
 *	  Check return pointer, if allocation failed (return=NULL)
 *		Log the failure and Issue a FAIL message.
 *	  Otherwise, dirty all the virtual pages to force physical allocation
 *	  And issue a PASS message, provided loop completed.
 * 
 * 	Cleanup:
 * 	  Print timing stats if options given
 * 
 * 
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

/* TODO: catch signal when "Processus arrete!" (OOM killer?) */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>    
#include <sys/user.h>	/* getpagesize() */
#include <time.h>

#include <sys/utsname.h>

#include "test.h"
#include "usctest.h"

/* in KB */
#define PROGRESS_LEAP 100


void setup();
void cleanup();
size_t get_memsize();

/*
 * TODO:
 *  - add option for growing direction, when doing linear touching
 *  - add option for touch running time (or infinite loop?)
 *  - make it multithreaded with random access to test r/w mm_sem
 */


char *TCID="mem01";		/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static int m_opt = 0;	/* memsize */
static char *m_copt;

static int r_opt = 0;	/* random access versus linear */
static int v_opt = 0;	/* verbose progress indication */


/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void 
setup()
{
    /* capture signals */
    tst_sig(NOFORK, DEF_HANDLER, cleanup);

    /* Pause if that option was specified */
    TEST_PAUSE;

    /* make a temp dir and cd to it */
    tst_tmpdir();
}	/* End setup() */


/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void 
cleanup()
{
    /*
     * print timing stats if that option was specified.
     * print errno log if that option was specified.
     */
    TEST_CLEANUP;

    /* remove files and temp dir */
    tst_rmdir();

    /* exit with return code appropriate for results */
    tst_exit();
}	/* End cleanup() */



void help()
{
  printf("  -m x    size of malloc in MB (default from /proc/meminfo)\n");
  printf("  -r      random touching versus linear\n");
  printf("  -v      verbose progress indication\n"); 
}


/*
 * return MemFree+SwapFree, from /proc/meminfo
 * returned value is in bytes.
 */
#define BUFF_SIZE 384
size_t get_memsize()
{
	long unsigned int res;
	long unsigned int unused;
	long unsigned int freesize;
	FILE *f;
	int retcode;
	int ictl;
	char buff[BUFF_SIZE];
	struct utsname *buf;

        /* allocate some space for buf */

        if ((buf = (struct utsname *)malloc((size_t)
                        sizeof(struct utsname))) == NULL) {
		tst_resm(TFAIL, "malloc failed for buf");
		return 0;
        }

        if (uname(buf) < 0) {
		tst_resm(TFAIL, "uname failed getting release number");
		return 0;
	}

	f = fopen("/proc/meminfo", "r");
	if (!f)
		return 0;

	/* ignore first line(titles or MemTotal) */
	if (!fgets(buff, BUFF_SIZE, f)) {
		fclose(f);
		return 0;
	}

        if ((strncmp(buf->release, "2.5", 3)) < 0) {

	/* FIXME: check return code! */
	retcode = fscanf(f, "Mem: %lu %lu %lu %lu %lu %lu", &unused, &unused,
 			&freesize, &unused, &unused, &unused);
	if (retcode != 6) {
		fclose(f);
		return 0;
	}

	printf("Free Mem: %ld bytes\n",freesize);
	res = freesize;

	if (!fgets(buff, BUFF_SIZE, f))	{ /* flush end of line */
		fclose(f);
		return 0;
	}

	retcode = fscanf(f, "Swap: %lu %lu %lu", &unused, &unused, &freesize);
	if (retcode != 3) {
		fclose(f);
		return 0;
	}

	printf("Free Swap: %ld bytes\n",freesize);
	res += freesize;
	res = res / 1024;

	 } else {

        	/* FIXME: check return code! */
	        retcode = fscanf(f, "MemFree:  %lu ", &freesize);
	        if (retcode != 1) {
	                fclose(f);
	                return 0;
	        }
        
		printf("Free Mem: %ld Kb\n",freesize);
		res = freesize;

	        if (!fgets(buff, BUFF_SIZE, f)) { /* flush end of line */
	                fclose(f);
	                return 0;
	        }

		/* ignore next 10 lines */
	for (ictl = 0; ictl < 10; ictl++) {
	       	if (!fgets(buff, BUFF_SIZE, f)) { 
        	      	 fclose(f);
               		 return 0;
	       	}
	}

	        retcode = fscanf(f, "SwapFree:  %lu ", &freesize);
		printf("Free Swap: %ld Kb\n",freesize);
	        if (retcode != 1) {
        	       	 fclose(f);
               		 return 0;
	       }

        	res += freesize;

	 } /* end check release */
	printf("Total Free: %ld Kb\n",res);
	/*Safety section*/				
        if (res > 4*1024*1024)			/* >4Gb free mem then  */
                res -= (1*1024*1024);	 	/* subtract 1Gb        */
        else {
        	if (res > 4*128*1024)   	/* >512Mb free mem then*/
                        res -= 1*100*1024;     	/* subtract 100MB      */
		else {
        		if (res > 4*1024)	/* >4Mb free mem then  */
                        	res -= 1*512;     	 /* subtract 512k       */
			else
				res -= 1;	/* <4Mb free mem, so subtract 1Kb */
		}
	 }
        res = res * 1024;
	free(buf);
	fclose(f);
	return res;
}

/*
 * add the -m option whose parameter is the
 * memory size (MB) to allocate.
 */
option_t options[] = 
{
  { "m:", &m_opt, &m_copt },
  { "r", &r_opt, NULL },
  { "v", &v_opt, NULL },
  { NULL, NULL, NULL }
};

int
main(int argc, char *argv[])
{
size_t memsize=0;	/* at first in MB, limited to 4Go on 32 bits */
int pagesize;

  int i;
    int lc;		/* loop counter */
    char *msg;		/* message returned from parse_opts */
	char *p, *bigmalloc;
	int loop_count;	/* limited to 16Go on 32 bits systems */


	pagesize = getpagesize();

    /***************************************************************
     * parse standard options
     ***************************************************************/
  if ( (msg=parse_opts(argc, argv, options, help)) != (char *) NULL )
   tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

  if ( m_opt )
  {
    memsize = atoi( m_copt )*1024*1024;

    if (memsize < 1)
    {
      tst_brkm(TBROK, cleanup, "Invalid arg for -m: %s",m_copt);
    }
  } 

	if (r_opt)
		srand(time(NULL));	/* fair enough */

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
    setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
    for (lc=0; TEST_LOOPING(lc); lc++) {

	/* reset Tst_count in case we are looping. */
	Tst_count=0;

  	if ( !m_opt ) {
			/* find out by ourselves! */
		memsize = get_memsize();
    		if (memsize < 1)
		    {
		      tst_brkm(TBROK, cleanup, "Unable to guess maxmemsize from /proc/meminfo");
		    }
  	}

	/* 
	 * TEST CASE:
	 * Allocate (virtual) memory (thru sbrk)
	 */

	/* virtual memory, it's magic :) */
	bigmalloc = p = (char*)malloc(memsize);

	if ( !p ) {
	    tst_resm(TFAIL, "malloc - alloc of %dMB failed", memsize/1024/1024);
    	    cleanup();
	}
	
	/* 
	 * TEST CASE:
	 * Dirty all the pages, to force physical RAM allocation
	 * and exercise eventually the swapper
	 */
      tst_resm(TINFO,"touching %uMB of malloc'ed memory (%s)",
      			memsize/1024/1024, r_opt?"random":"linear");


	loop_count = memsize/pagesize;

	for (i=0; i<loop_count; i++) {
		if (v_opt && (i%(PROGRESS_LEAP*1024/pagesize) == 0) ) {
#if 0
			printf("%dKB ",i*pagesize/1024);
#else
			printf(".");
#endif
			fflush(stdout);
		}
		/*
		 * Make the page dirty,
		 * and make sure compiler won't optimize it away
		 * Touching more than one word per page is useless
		 * because of cache.
		 */
		*(int*)p = 0xdeadbeef^i;

		if (r_opt) {
			p = bigmalloc + 
				(size_t) ((double)(memsize-sizeof(int))*rand()/(RAND_MAX+1.0) );
		} else {
			p += pagesize;
		}
	}

	if (v_opt)
		printf("\n");


	/* This is not mandatory (except in a loop), but it exercise mm again */
	free(bigmalloc);

      /*
       * seems that if the malloc'ed area was bad, we'd get SEGV (or kicked
       * somehow by the OOM killer?), hence we can indicate a PASS.
       */
	tst_resm(TPASS, "malloc - alloc of %dMB succeeded", memsize/1024/1024);

    }	/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
    cleanup();

    return 0;
}	/* End main */
