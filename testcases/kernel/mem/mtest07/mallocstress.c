/******************************************************************************/
/*									      */
/* Copyright (c) International Business Machines  Corp., 2001		      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,	      */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	              */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License	      */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*									      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* History:     Nov - 04 - 2001 Created - Manoj Iyer, IBM Austin TX.          */
/*                               email:manjo@austin.ibm.com                   */
/*                                                                            */
/*		Nov - 06 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- added function alloc_mem()                  */
/*    								              */
/*		Nov - 08 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- added logic to allocate memory in the size  */
/*				  of fibanocci numbers.                       */
/*				- fixed segmetation fault.                    */
/*									      */
/*		Nov - 09 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- separated alocation logic to allocate_free()*/
/*				  function.                                   */
/*				- introduced logic to randomly pick allocation*/
/*				  scheme. size = fibannoci number, pow of 2 or*/
/*				  power of 3.                                 */
/*				- changed comments.                           */
/*				- Added test to LTP.                          */
/*                                                                            */
/*		Nov - 09 - 2001 Modified - Manoj Iyer,IBM Austin TX.	      */
/*				- Removed compile errors.                     */
/*				- too many missing arguments.                 */
/*								              */
/*		Nov - 19 - 2001 Modified - Manoj Iyer, IBM Austin TX.	      */
/*				- fixed segmentation fault. 		      */
/*				  changed variable th_status from dynamic to  */
/*				  static array.			              */
/*                                                                            */
/*		May - 15 - 2002 Dan Kegel (dank@kegel.com)                    */
/*		                - Fixed crash on > 30 threads                 */
/*		                - Cleaned up, fixed compiler warnings         */
/*		                - Removed mallocs that could fail             */
/*		                - Note that pthread_create fails with EINTR   */
/*                                                                            */
/* File:	mallocstress.c						      */
/*									      */
/* Description:	This program stresses the VMM and C library                   */
/*              by spawning N threads which                                   */
/*              malloc blocks of increasing size until malloc returns NULL.   */
/******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAXL    100		/* default number of loops to do malloc and free      */
#define MAXT     60		/* default number of threads to create.               */

#ifdef DEBUG
#define dprt(args)	printf args
#else
#define dprt(args)
#endif

#define OPT_MISSING(prog, opt)   do{\
			       fprintf(stderr, "%s: option -%c ", prog, opt); \
                               fprintf(stderr, "requires an argument\n"); \
                               usage(prog); \
                                   } while (0)

int num_loop = MAXL;		/* number of loops to perform                     */
int semid;

/* Define SPEW_SIGNALS to tickle thread_create bug (it fails if interrupted). */
#define SPEW_SIGNALS

/******************************************************************************/
/*								 	      */
/* Function:	my_yield						      */
/*									      */
/* Description:	Yield control to another thread.                              */
/*              Generate a signal, too.                                       */
/*									      */
/******************************************************************************/
static void my_yield()
{
#ifdef SPEW_SIGNALS
	/* usleep just happens to use signals in glibc at moment.
	 * This is good because it allows us to test whether pthread_create
	 * improperly returns EINTR (which would violate SUSv3)
	 */
	usleep(0);
#else
	/* If you want this test to pass, don't define SPEW_SIGNALS,
	 * as pthread_create is broken at moment, and fails if interrupted
	 */
	static const struct timespec t0 = { 0, 0 };
	nanosleep(&t0, NULL);
#endif
}

/******************************************************************************/
/*								 	      */
/* Function:	usage							      */
/*									      */
/* Description:	Print the usage message.				      */
/*									      */
/* Input:	char *progname - name of this program                         */
/*									      */
/* Return:	exits with -1						      */
/*									      */
/******************************************************************************/
static void usage(char *progname)
{				/* name of this program                       */
	fprintf(stderr,
		"Usage: %s -d NUMDIR -f NUMFILES -h -t NUMTHRD\n"
		"\t -h Help!\n"
		"\t -l Number of loops:               Default: 1000\n"
		"\t -t Number of threads to generate: Default: 30\n", progname);
	exit(-1);
}

/******************************************************************************/
/* Function:	allocate_free				                      */
/*								              */
/* Description:	This function does the allocation and free by calling malloc  */
/*		and free fuctions. The size of the memory to be malloced is   */
/*		determined by the caller of this function. The size can be    */
/*		a number from the fibannoaci series, power of 2 or 3 or 5     */
/*									      */
/* Input:	int repeat - number of times the alloc/free is repeated.      */
/*		int scheme  - 0 to 3; selects how fast memory size grows      */
/*								              */
/* Return:	1 on failure						      */
/*		0 on success						      */
/******************************************************************************/
int allocate_free(int repeat,	/* number of times to repeat allocate/free    */
		  int scheme)
{				/* how fast to increase block size            */
	int loop;
	const int MAXPTRS = 50;	/* only 42 or so get used on 32 bit machine */

	dprt(("pid[%d]: allocate_free: repeat %d, scheme %d\n", getpid(),
	      repeat, scheme));

	for (loop = 0; loop < repeat; loop++) {
		size_t oldsize = 5;	/* remember size for fibannoci series     */
		size_t size = sizeof(long);	/* size of next block in ptrs[]           */
		long *ptrs[MAXPTRS];	/* the pointers allocated in this loop    */
		int num_alloc;	/* number of elements in ptrs[] so far    */
		int i;

		dprt(("pid[%d]: allocate_free: loop %d of %d\n", getpid(), loop,
		      repeat));

		/* loop terminates in one of three ways:
		 * 1. after MAXPTRS iterations
		 * 2. if malloc fails
		 * 3. if new size overflows
		 */
		for (num_alloc = 0; num_alloc < MAXPTRS; num_alloc++) {
			size_t newsize = 0;

			dprt(("pid[%d]: loop %d/%d; num_alloc=%d; size=%u\n",
			      getpid(), loop, repeat, num_alloc, size));

			/* Malloc the next block */
			ptrs[num_alloc] = malloc(size);
			if (ptrs[num_alloc] == NULL) {
				/* terminate loop if malloc couldn't give us the memory we asked for */
				break;
			}
			ptrs[num_alloc][0] = num_alloc;

			/* Increase size according to one of four schedules. */
			switch (scheme) {
			case 0:
				newsize = size + oldsize;
				oldsize = size;
				break;
			case 1:
				newsize = size * 2;
				break;
			case 2:
				newsize = size * 3;
				break;
			case 3:
				newsize = size * 5;
				break;
			default:
				assert(0);
			}
			/* terminate loop on overflow */
			if (newsize < size)
				break;
			size = newsize;

			my_yield();
		}

		for (i = 0; i < num_alloc; i++) {
			dprt(("pid[%d]: freeing ptrs[i] %p\n", getpid(),
			      ptrs[i]));
			if (ptrs[i][0] != i) {
				fprintf(stderr,
					"pid[%d]: fail: bad sentinel value\n",
					getpid());
				return 1;
			}
			free(ptrs[i]);
			my_yield();
		}

		my_yield();
	}
	/* Success! */
	return 0;
}

/******************************************************************************/
/* Function:	alloc_mem				                      */
/*								              */
/* Description:	Decide how fast to increase block sizes, then call            */
/*              allocate_free() to actually to the test.                      */
/*								              */
/* Input:	threadnum is the thread number, 0...N-1                       */
/*              global num_loop is how many iterations to run                 */
/*								              */
/* Return:	pthread_exit -1	on failure				      */
/*		pthread_exit  0 on success			              */
/*								              */
/******************************************************************************/
void *alloc_mem(void *threadnum)
{
	struct sembuf sop[1];
	sop[0].sem_num = 0;
	sop[0].sem_op = 0;
	sop[0].sem_flg = 0;
	/* waiting for other threads starting */
	if (semop(semid, sop, 1) == -1) {
		if (errno != EIDRM)
			perror("semop");
		return (void *)-1;
	}

	/* thread N will use growth scheme N mod 4 */
	int err = allocate_free(num_loop, ((uintptr_t) threadnum) % 4);
	fprintf(stdout,
		"Thread [%d]: allocate_free() returned %d, %s.  Thread exiting.\n",
		(int)(uintptr_t) threadnum, err,
		(err ? "failed" : "succeeded"));
	return (void *)(uintptr_t) (err ? -1 : 0);
}

/******************************************************************************/
/*								 	      */
/* Function:	main							      */
/*									      */
/* Description:	This is the entry point to the program. This function will    */
/*		parse the input arguments and set the values accordingly. If  */
/*		no arguments (or desired) are provided default values are used*/
/*		refer the usage function for the arguments that this program  */
/*		takes. It also creates the threads which do most of the dirty */
/*		work. If the threads exits with a value '0' the program exits */
/*		with success '0' else it exits with failure '-1'.             */
/*									      */
/* Return:	-1 on failure						      */
/*		 0 on success						      */
/*									      */
/******************************************************************************/
int main(int argc,		/* number of input parameters                 */
	 char **argv)
{				/* pointer to the command line arguments.     */
	int c;			/* command line options                       */
	int num_thrd = MAXT;	/* number of threads to create                */
	int thrd_ndx;		/* index into the array of thread ids         */
	pthread_t *thrdid;	/* the threads                                */
	extern int optopt;	/* options to the program                     */
	struct sembuf sop[1];
	int ret = 0;

	while ((c = getopt(argc, argv, "hl:t:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 'l':
			if ((num_loop = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_loop < 1) {
				fprintf(stdout,
					"WARNING: bad argument. Using default\n");
				num_loop = MAXL;
			}
			break;
		case 't':
			if ((num_thrd = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_thrd < 1) {
				fprintf(stdout,
					"WARNING: bad argument. Using default\n");
				num_thrd = MAXT;
			}
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	dprt(("number of times to loop in the thread = %d\n", num_loop));

	thrdid = malloc(sizeof(pthread_t) * num_thrd);
	if (thrdid == NULL) {
		perror("main(): allocating space for thrdid[] malloc()");
		return 1;
	}

	semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid < 0) {
		perror("Semaphore creation failed  Reason:");
	}

	sop[0].sem_num = 0;
	sop[0].sem_op = 1;
	sop[0].sem_flg = 0;
	if (semop(semid, sop, 1) == -1) {
		perror("semop");
		ret = -1;
		goto out;
	}

	for (thrd_ndx = 0; thrd_ndx < num_thrd; thrd_ndx++) {
		if (pthread_create(&thrdid[thrd_ndx], NULL, alloc_mem,
				   (void *)(uintptr_t) thrd_ndx)) {
			int err = errno;
			if (err == EINTR) {
				fprintf(stderr,
					"main(): pthread_create failed with EINTR!\n");
				ret = -1;
				goto out;
			}
			perror("main(): pthread_create()");
			ret = -11;
			goto out;
		}
	}
	my_yield();

	sop[0].sem_op = -1;
	if (semop(semid, sop, 1) == -1) {
		perror("semop");
		ret = -1;
		goto out;
	}

	for (thrd_ndx = 0; thrd_ndx < num_thrd; thrd_ndx++) {
		void *th_status;	/* exit status of LWP */
		if (pthread_join(thrdid[thrd_ndx], &th_status) != 0) {
			perror("main(): pthread_join()");
			ret = -1;
			goto out;
		} else {
			if ((intptr_t) th_status != 0) {
				fprintf(stderr,
					"main(): thread [%d] - exited with errors\n",
					thrd_ndx);
				ret = -1;
				goto out;
			}
			dprt(("main(): thread [%d]: exited without errors\n",
			      thrd_ndx));
		}
		my_yield();
	}
	printf("main(): test passed.\n");
out:
	if (semctl(semid, 0, IPC_RMID) == -1) {
		perror("semctl\n");
		ret = -1;
	}
	if (thrdid) {
		free(thrdid);
		thrdid = NULL;
	}
	exit(ret);
}
