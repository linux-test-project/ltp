/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 * NAME
 *	mincore02.c
 *
 * DESCRIPTION
 *	Testcase to check the error conditions for mincore
 *
 * ALGORITHM
 *	test1:
 *		This test case provides a functional validation for mincore system call.
 *      we memory map a file and lock it in memory, we spin off a child process, 
 *      which shares the memory mapped file with the parent process. When the 
 *      child process executes the mincore system call , then it should read the 
 *      the same number of pages which were mapped and locked by parent process. 
 *
 * USAGE:  <for command-line>
 *  ./mincore02
 *
 * HISTORY
 *  Author: Rajeev Tiwari: rajeevti@in.ibm.com
 *	08/2004 Rajeev Tiwari : Provides a functional validation of mincore system
 *                          call.
 *
 * RESTRICTIONS
 *	None
 */


/* Standard Include Files */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
 
/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* comment out if you need to debug */

/* #define DEBUG_MODE 1 */

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "mincore02"; 			/* test program identifier.          */
int  TST_TOTAL = 1;                  	/* total number of tests in this file.   */
int LOOP = 0;							/* this is needed to make the parent process run in tight loop */
int file_desc=0;						/* this is for the file descriptor */
int status=0;							/* this is for getting the final test case status */
struct stat fst;						/* this is for capturing the file status eg size etc.. */
char *position=NULL;				
int pid=0;
int p_size = 4096;
int num_pages=0;
int counter=0;
int lock_pages=0;
char * vec=NULL;


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
void cleanup()
{

    /* Close all open file descriptors. */
    /* Exit with appropriate return code. */
	
	TEST_CLEANUP;
	close(file_desc);
	remove("./mincore02.txt");

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
/*                                                     */
/*                                                                            */
/******************************************************************************/

void setup()
{
	char *buf = malloc(4096*4);
	int counter;
	int size = 4096*4;
	int status;
	char *msg;   
	
	for (counter =0; counter < size ; counter++)
	{
		*(buf+counter) = 'x';
	}
	
	
	/* create a temporary file to be used */
	
	if((file_desc = open("./mincore02.txt",O_RDWR|O_CREAT))==-1)
	{
		tst_brkm(TBROK, cleanup,  "Unable to open file for read/write.  Error:%d, %s\n" , errno, strerror(errno));
	} 
        
	
	/* fill the temporary file with two pages of data */
	
	if((status=write(file_desc,buf,size))==-1)
	{
		tst_brkm(TBROK, cleanup, "Error in writing to the file - %s", msg);
	}
	
	
	/* get the size of the file using fstat */
                                                                                                                                             
     if((status = fstat(file_desc,&fst))==-1)
     {
         tst_brkm(TBROK, cleanup,  "Unable to get file status   Error:%d, %s\n", errno, strerror(errno));
      }
                                                                                                                                             
     /* mmap the file in virtual address space in read , write and execute mode , the mapping should be shared  */
                                                                                                                                             
                                                                                                                                             
    if((position = (char *)mmap(0,fst.st_size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_SHARED,file_desc,0))==-1)
	{
        tst_brkm(TBROK, cleanup,  "Unable to map file for read/write.  Error:%d, %s\n", errno, strerror(errno));
	}

    vec = malloc(4096*2);
	return;
}

/* this is a function executed by the parent process to wait for a terminating 
 * signal from the child process 
 */

void handler(int SIG){
	LOOP=1;
}

/* main function */

int
main(int   argc,    /* number of command line parameters                      */
     char **argv)   /* pointer to the array of the command line parameters.   */
{

    /* perform global test setup, call setup() function. */
    setup();

    /* Print test Assertion using tst_resm() function with argument TINFO. */
    tst_resm(TINFO, "Testing mlock system lock if it works\n");

    /* Test Case Body. */

     /* this time the file has been mapped to the virtual address space */
     /* we can now lock specific portions of the file */
     /* int mlock(const void *addr, size_t len) */
                                                                                                                                             
	if((status = mlock(position,fst.st_size))==-1)
	{
		tst_brkm(TBROK, cleanup,  "Unable to lock the file - %d, %s\n", errno, strerror(errno));
	}
     
     /* at this point create two process */

     /* register a signal handler */

	//signal(SIGINT, handler);
	
	pid = fork(); 
    if(pid!=0)
	{
		printf("Parent process with PID = %d\n", getpid());
		signal(SIGINT, handler);
	/* RUN IN A INFINITE LOOP to be killed on receiving the signal  */                                                                                                                                    
  		while(LOOP==0);
        	cleanup();
	}
	else
	{
		printf("Child process with PID = %d\n", getpid());
		sleep(3);
		/* code executed by the child process */
		/* call the global setup function */
		setup();
		/* run mincore to verify if the page resides in the memory or not 	*/
        /* int mincore(void *start, size_t length, unsigned char *vec);		*/
        /* vec must be large enough to contain length/PAGE_SIZE  bytes. 	*/
        p_size = 4096;                                                                                                                                 
        
		if(fst.st_size != 0)
        {
                num_pages = fst.st_size/p_size;
                if(fst.st_size % p_size != 0)
                {
                        num_pages++;
                }
        }
                                                                                                                                             
#ifdef DEBUG_MODE
		printf("fst.st_size = %d\n", fst.st_size);
        printf("num_pages = %d\n", num_pages);
#endif
		
        vec = (char *)malloc(num_pages);
        if((status = mincore((void *)position,fst.st_size,vec))==-1)
        {
               tst_brkm(TBROK, cleanup, "Unable to execute mincore system call \n"); 
        }
                                                                                                                                             
        /* the above vec should be filled with the page status		*/
	/* check the status of the last bit for all the bytes / pages	*/
                                                                                                                                             
        lock_pages = 0;
                                                                                                                                             
        for(counter = 0; counter < num_pages; counter++)
        {
                status =((int )(*vec))&1;
                if(status==1)
                lock_pages++;
                vec++;
        }
                                                                                                                                             
        if(lock_pages == num_pages)
		tst_resm(TPASS, "\n\n  pages locked in memory are %d \n\n no of pages calculated by mincore are %d:",num_pages,lock_pages);
	else
		tst_resm(TFAIL,"mincore didn't work, not all the pages locked by parent process are there\n\n");
	#ifdef DEBUG_MODE
		printf("Sending signal to parent\n");
	#endif
		kill(getppid(),SIGINT);
	}

}
