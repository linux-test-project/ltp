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
 *	mincore01.c
 *
 * DESCRIPTION
 *	Testcase to check the error conditions for mincore
 *
 * ALGORITHM
 *	test1:
 *		Invoke mincore() when the start address is not multiple of page size.
 *      EINVAL
 *  test2: 
 *      Invoke mincore()when the length has a non-positive value. EINVAL
 *	test3:
 *		Invoke mincore() when the vector points to an invalid address. EFAULT
 *	test4:
 *		Invoke mincore() when the starting address + length contained unmapped 
 *		memory. ENOMEM
 *  test5:
 *		Invoke mincore() when the length contained memory is not part of file. 
 *      ENOMEM
 *
 * USAGE:  <for command-line>
 *  mincore01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *  Author: Rajeev Tiwari: rajeevti@in.ibm.com
 *	08/2004 Rajeev Tiwari : does a basic sanity check for the various error 
 *  conditions possible with the mincore system call.
 *
 * RESTRICTIONS
 *	None
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <limits.h>		/* for PAGESIZE */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

void cleanup(void);
void setup(void);
void setup1(void);
void setup2(void);
void setup3(void);
void setup4(void);
void setup5(void);

char *TCID= "mincore01";
int TST_TOTAL = 5;
extern int Tst_count;


char* file_pointer=NULL;
char* global_pointer=NULL;
char* global_vec=NULL;
int len = 0;
char *vec = NULL;
int file_desc =0;
struct stat *fst=NULL;
	
char * file_name = NULL;

int exp_enos[]={EINVAL,EINVAL,EFAULT, ENOMEM,ENOMEM};

struct test_case_t {
        char **addr;
        int len;
        int error;
		char **vector;
        void (*setupfunc)();
} TC[] = {
		  { &file_pointer,0,0,&vec,setup1 },
		  { &file_pointer,0,0,&vec,setup2 },
		  { &file_pointer,0,0,&vec,setup3 },
		  { &file_pointer,0,0,&vec,setup4 },
		  { &file_pointer,0,0,&vec,setup5 }
}; 

/* Uncomment the macro below to debug */
/* #define DEBUG_MODE 1 */

int main(int ac, char **av)
{
	int lc;                         /* loop counter */
	int i;
	char *msg;                      /* message returned from parse_opts */

        /* parse standard options */
        if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
        }

        setup();                        /* global setup */

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

                /* reset Tst_count in case we are looping */
                Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			/* perform test specific setup */
			if (TC[i].setupfunc != NULL) {
				TC[i].setupfunc();
			}
#ifdef DEBUG_MODE
			printf("Before Calling mincore -->> Address : %x, len : %x, vector : %x\n", *(TC[i].addr), TC[i].len, *(TC[i].vector));
#endif
			TEST(mincore(*(TC[i].addr), TC[i].len, *(TC[i].vector)));

                        if (TEST_RETURN != -1) {
                                tst_resm(TFAIL, "call succeeded unexpectedly");
                                continue;
                        }

                        TEST_ERROR_LOG(TEST_ERRNO);

                        if (TEST_ERRNO == TC[i].error) {
                                tst_resm(TPASS, "expected failure - "
                                         "errno = %d : %s", TEST_ERRNO,
                                         strerror(TEST_ERRNO));
                        } else {
                                tst_resm(TFAIL, "unexpected error - %d : %s - "
                                         "expected %d", TEST_ERRNO,
                                         strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
        cleanup();
	return(0);
	/*NOTREACHED*/
}

/*
 * setup1() - sets up conditions for the first test. the start address is not 
 * multiple of page size
 */
void
setup1()
{
	/* resetting the value of file pointer and bitmap vector */
	file_pointer = global_pointer;
	vec = global_vec;
	
	int size = 4096*2;
	*(TC[0].addr) = file_pointer + 1;
	TC[0].len = size;
	*(TC[0].vector) = vec;
	TC[0].error = EINVAL;
	
}

/*
 * setup2() - sets up conditions for the second test. the length has a 
 * non-positive value
 */
void
setup2()
{
	/* resetting the value of file pointer and bitmap vector */
	file_pointer = global_pointer;
	vec = global_vec;
	
	int size = 4096*2;
	*(TC[1].addr) = file_pointer;
	TC[1].len = -size;
	*(TC[1].vector) = vec;
	TC[1].error = EINVAL;

	
}

/*
 * setup3() - sets up conditions for the third test. the vector points to an 
 * invalid address.
 */
void
setup3()
{
	/* resetting the value of file pointer and bitmap vector */
	file_pointer = global_pointer;
	vec = global_vec;
	
	int size = 4096*2;
	*(TC[2].addr) = file_pointer;
	TC[2].len = size;
	*(TC[2].vector) = 0xC0000000;
	TC[2].error = EFAULT;

}

/*
 *  setup4() - performs the setup for test4(the starting address + length 
 *  contained unmapped memory). we give the length of mapped file equal to 5 
 *  times the mapped file size.
 */
void 
setup4()
{
	/* resetting the value of file pointer and bitmap vector */
	file_pointer = global_pointer;
	vec = global_vec;
	
	int size = 4096*10;
	*(TC[3].addr) = file_pointer;
	TC[3].len = size;
	*(TC[3].vector) = vec;
	TC[3].error = ENOMEM;

}

/*
 * setup5() - performs the setup related to the test5
 * length contained memory is not part of file. The file size is 2 pages,
 * we give a offset of 2 pages as the starting address.
 */
void
setup5()
{	
	int size = 4096;
	
	/* resetting the value of file pointer and bitmap vector */
	file_pointer = global_pointer;
	vec = global_vec;
	
	*(TC[4].addr) = file_pointer+2*size;
	TC[4].len = size;
	*(TC[4].vector) = vec;
	TC[4].error = ENOMEM;

}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void
setup()
{
	/* create a temporary buffer to fill two pages of data */
	
	char *buf = malloc(4096*2);
	int counter;
	int size = 4096*2;
	int status;
	char *msg;   
	
	for (counter =0; counter < size ; counter++)
	{
		*(buf+counter) = 'x';
	}
	
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
	
	file_name = tmpnam(NULL);
	
	/* create a temporary file to be used */
	
	if((file_desc = open(file_name,O_RDWR|O_CREAT))==-1)
	{
		tst_brkm(TBROK, cleanup, "Error in opening file - %s", msg);
	}
	
	
	/* fill the temporary file with two pages of data */
	
	if((status=write(file_desc,buf,size))==-1)
	{
		tst_brkm(TBROK, cleanup, "Error in writing to the file - %s", msg);
	}
	
	/* collect the stats on the file to get the size */
	
	fst = (struct stat *) malloc(sizeof(struct stat));
	
	if((status=fstat(file_desc,fst))==-1)
	{
		tst_brkm(TBROK, cleanup, "Error in getting file stats - %s", msg);
	}
	
	/* map the file in memory */
	
	if((file_pointer = (char *)mmap(NULL,(*fst).st_size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_SHARED,file_desc,NULL))==-1)
	{
		tst_brkm(TBROK, cleanup, "File cannot be mapped in memory - %s", msg);
	}
	/* Align to a multiple of PAGESIZE, assumed to be power of two */
	 
	 file_pointer = (char *)(((int)file_pointer + PAGESIZE-1) & ~(PAGESIZE-1));
	
	/* set the global pointer */
	
	global_pointer = file_pointer;

#ifdef DEBUG_MODE
		printf("After alignment file_pointer : %x\n", file_pointer);	
#endif
	 
	 /* initialize the vector buffer to collect the page info */
	 
	vec = malloc(4096*2);	
	
	/* set the global vector */
	global_vec = vec;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void
cleanup()
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;
	
	close(file_desc);
	remove(file_name);

	/* exit with return code appropriate for results */
	tst_exit();
}
