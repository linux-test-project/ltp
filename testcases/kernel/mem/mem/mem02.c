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

/* IBM Corporation */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 10/30/2002	Port to LTP	dbarrera@us.ibm.com */

			/* mem02.c */
/*======================================================================
/	=================== TESTPLAN SEGMENT ===================
>KEYS:	< calloc, malloc, free, realloc, valloc
>WHAT:	< check that memory can be allocated and freed. check for zeroed
   	< memory.
>HOW:	< Allocate a big chunk of memory, verify it is available (zeroed
	< in the case of calloc).
	< Write into it and verify, free memory and verify free was
	< successful.
	< In the case of valloc, allocate memory and free it (do this for
	< several iterations). Check if valloc returns unaligned pointers.
	< If valloc causes a SIGSEGV, that means a failure has occured.
>BUGS:	< 
======================================================================*/


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
#include <sys/user.h>

#define MEMSIZE	8192*8192

void on_mem_fault(int sig);		/*Prototype for signal handler*/

char *TCID="mem02";			/* Test program identifier. */
int TST_TOTAL=1;			/* Total number of test cases. */
extern int Tst_count;			/* Test Case counter for tst_* routines */


/*--------------------------------------------------------------------*/
int main()					/***** BEGINNING OF MAIN. *****/
{
	int i;
	char *pm1,*pm2,*pm3,*pm4,*pm5;
	long pm6;
	void *memptr;		/* Pointer to valloced memory */
	long laddr;		/* Address of valloced memory */
	int iteration_count;
	int size;		/* Size to memory to be valloced */
	int pagesize = 12;	/* 2^12 = 4096, PAGESIZE      */

/*--------------------------------------------------------------------*/

	/* check out calloc/free */
	if((pm2=pm1=(char *)calloc(MEMSIZE,1)) == NULL) {

		tst_resm(TFAIL, "calloc - alloc of %dMB failed", MEMSIZE/1024/1024);
		             tst_exit();
	} 

	for(i=0; i<MEMSIZE; i++)
		if(*pm2++ != 0) {
  		  	tst_resm(TFAIL, "calloc returned non zero memory");
			             tst_exit();
		}

	pm2=pm1;
	for(i=0; i<MEMSIZE; i++)
		*pm2++ = 'X';
	pm2=pm1;
	for(i=0; i<MEMSIZE; i++)
		if(*pm2++ != 'X') {
			 tst_resm(TFAIL, "could not write/verify memory ");
			             tst_exit();
		}

	pm2=pm1;
	free(pm1);

	if((pm1=(char *)calloc(MEMSIZE,1)) == NULL) {
		tst_resm(TFAIL, "calloc did not alloc memory ");
		             tst_exit();
	}

	if(pm1 != pm2) {
		tst_resm(TFAIL, "free did not dealloc memory ");
		             tst_exit();
	}
	free(pm1);
	 
        tst_resm(TPASS,"calloc - calloc of %uMB of memory succeeded",
	                     MEMSIZE/1024/1024);
	 
/*--------------------------------------------------------------------*/

	/* check out malloc/free */
	if((pm2=pm1=(char *)malloc(MEMSIZE)) == NULL) {
		tst_resm(TFAIL, "malloc did not alloc memory ");
		             tst_exit();
	}	      

	for(i=0; i<MEMSIZE; i++)
		*pm2++ = 'X';
	pm2=pm1;
	for(i=0; i<MEMSIZE; i++)
		if(*pm2++ != 'X') {
			tst_resm(TFAIL, "could not write/verify memory ");
		             tst_exit();
		}

	pm2=pm1;
	free(pm1);

	if((pm1=(char *)calloc(MEMSIZE,1)) == NULL) {
		tst_resm(TFAIL, "malloc did not alloc memory ");
		             tst_exit();
	}

	if(pm1 != pm2) {
		tst_resm(TFAIL, "free did not dealloc memory ");
		             tst_exit();
	}
	free(pm1);

        tst_resm(TPASS,"malloc - malloc of %uMB of memory succeeded",
	                     MEMSIZE/1024/1024);
	 
/*--------------------------------------------------------------------*/

	/* check out realloc */

	pm4=pm3=(char *)malloc(10);
	for(i=0; i<10; i++)
		*pm4++ = 'X';
	
	/* realloc with reduced size */
	pm4=(char *)realloc(pm3,5);
	pm6=(long)pm4;
	pm3=pm4;
	/* verify contents did not change */
	for(i=0; i<5; i++) {
		if(*pm4++ != 'X') {
			tst_resm(TFAIL, "realloc changed memory contents");
		             tst_exit();
		}
	}

        tst_resm(TPASS,"realloc - realloc of %dMB succeeded",
	                     pm6/1024/1024);
	 
	/* realloc with increased size after fragmenting memory */
	pm5=(char *)malloc(1);
	pm4=(char *)realloc(pm3,15);
	pm6=(long)pm3;
	pm3=pm4;
	/* verify contents did not change */
	for(i=0; i<5; i++) {
		if(*pm3++ != 'X') {
			tst_resm(TFAIL, "realloc changed memory contents");
		             tst_exit();
		}
	}
	
        tst_resm(TPASS,"realloc - realloc of %dMB succeeded",
	                     pm6/1024/1024);

/*--------------------------------------------------------------------*/
	/*
	 * Check out for valloc failures
	 */

	/*
	 * Setup to catch the memory fault, otherwise the core might
	 * be dumped on failures.
	 */
	if ((signal(SIGSEGV, on_mem_fault)) == SIG_ERR) {
		tst_resm(TFAIL, "Could not get signal handler for SIGSEGV");
	        tst_exit();
	}

	srand(1);			/* Ensure Determinism         */

	for (iteration_count = 15000; iteration_count > 0; iteration_count--) {
		/*
		 * size is a fraction of 100000 and is determined by rand().
		 */
		size = (int)((rand() / (float)RAND_MAX) * 100000) + 1;
		memptr = valloc(size);

		/*
		 * Check to see if valloc returns unaligned data.
		 * This can be done by copying the memory address into
		 * a variable and the by diving and multipying the address
		 * by the pagesize and checking.
		 */
		laddr = (long)memptr;
		if (((laddr >> pagesize) << pagesize) != laddr) {
			tst_resm(TFAIL, "Valloc returned unaligned data");
		        tst_exit();
		}

		free(memptr);
	}

	return 0;
/*--------------------------------------------------------------------*/
}					/******** END OF MAIN. ********/
/*--------------------------------------------------------------------*/

/*
 * void
 * on_mem_fault(int sig)
 *
 *	on_mem_fault() is a signal handler used by the valloc test-case
 *	(block 3). This function will catch the signal, indicate a failure,
 *	write to the log file (a failure message) and exit the test.
 */
void
on_mem_fault(int sig)
{
	/*
	 * Put error messages in the log file.
	 */
		tst_resm(TFAIL, "\tTest failed on receipt of a SIGSEGV signal");
	        tst_exit();
}
