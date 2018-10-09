/*****************************************************************************/
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
/*									      */
/* History:	July - 16 - 2001 Created by Manoj Iyer, IBM Austin TX.	      */
/*			         email:manjo@austin.ibm.com		      */
/*                                                                            */
/*		July - 30 - 2001 Modified - Added function write_to_mem.      */
/*							                      */
/*              Aug  - 14 - 2001 Modified - Added code to remove the shared   */
/*			         memory segment ids.                          */
/*							                      */
/*              Aug  - 15 - 2001 Modified - Added for loop to run the test    */
/*			         repeatedly.                                  */
/*                                                                            */
/*		Oct  - 22 - 2001 Modified - Fixed bad code in main().         */
/*				 removed stray code and options. Pthread_join */
/*			         part fixed, older version was completely bad */
/*                                                                            */
/*		Nov  - 09 - 2001 Modified - Removed compile errors	      */
/*				 - added missing header file string.h         */
/*				 - removed unused variables.                  */
/*				 - made read_ndx and write_ndx static variable*/
/*                                                                            */
/*		Nov - 91 - 2001	Modified - Changed scope of status variable   */
/*				 - change the status of status variable from  */
/*				   int *status to int status[1]               */
/*                                                                            */
/* File:	shmat1.c						      */
/*			         					      */
/* Description: Test the LINUX memory manager. The program is aimed at        */
/*              stressing the memory manager by repeaded shmat/write/read/    */
/*		shmatd of file/memory of random size (maximum 1000 * 4096)    */
/*		done by multiple processes.				      */
/*			         					      */
/*		Create a file of random size upto 1000 times 4096. 	      */
/*		process X shmats and un-shmats this file in memory.	      */
/*		process Y changes content of the file to Y, ie writes to it.  */
/*		process Z reads from this memory location, and varifies the   */
/*		the content of the file.			              */
/*			         					      */
/******************************************************************************/

/* Include Files							      */
#include <stdio.h>		/* definitions for standard I/O               */
#include <unistd.h>		/* required by usleep()                       */
#include <errno.h>		/* definitions for errno                      */
#include <sched.h>		/* definitions for sched_yield()              */
#include <stdlib.h>		/* definitions for WEXIT macros               */
#include <signal.h>		/* required by sigaction & sig handling fncs  */
#include <sys/time.h>		/* definitions of settimer()                  */
#include <sys/wait.h>		/* definitions of itimer structure            */
#include <pthread.h>		/* definitions for pthread_create etc         */
#include <setjmp.h>		/* required by setjmp longjmp                 */
#include <sys/ucontext.h>	/* required by the signal handler             */
#include <sys/ipc.h>		/* required by shmat shmget etc               */
#include <sys/shm.h>		/* required by shmat shmget etc               */
#include <string.h>		/* required by strncmp                        */

/* Defines								      */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define prtln() printf(" I AM HERE ==> %s %d\n", __FILE__, __LINE__);

#define STR_SHMAT  "  "
#define STR_WRITER "    "
#define STR_READER "      "

/* Global Variables						              */
void *map_address;		/* pointer to file in memory                  */
sigjmp_buf jmpbuf;		/* argument to setjmp and longjmp             */
int fsize;			/* size of the file to be created.            */
int done_shmat = 0;		/* disallow read and writes before shmat      */

/******************************************************************************/
/*									      */
/* Function:	sig_handler						      */
/*									      */
/* Description:	handle SIGALRM raised by set_timer(), SIGALRM is raised when  */
/*		the timer expires. If any other signal is received exit the   */
/*		test.							      */
/*									      */
/* Input:	signal - signal number, intrested in SIGALRM!		      */
/*									      */
/* Return:	exit -1 if unexpected signal is received		      */
/*		exit 0 if SIGALRM is received			              */
/*									      */
/******************************************************************************/
static void sig_handler(int signal,	/* signal number, set to handle SIGALRM       */
			int code, ucontext_t *ut)
{				/* contains pointer to sigcontext structure   */
#ifdef __i386__
	unsigned long except;	/* exception type.                            */
	int ret = 0;		/* exit code from signal handler.             */
	struct sigcontext *scp =	/* pointer to sigcontext structure            */
	    (struct sigcontext *)&ut->uc_mcontext;
#endif

	if (signal == SIGALRM) {
		fprintf(stdout, "Test ended, success\n");
		exit(0);
	}
#ifdef __i386__
	else {
		except = scp->trapno;
		fprintf(stderr, "signal caught - [%d] ", signal);
	}

	switch (except) {
	case 10:
		fprintf(stderr,
			"Exception - invalid TSS, exception #[%ld]\n", except);
		break;
	case 11:
		fprintf(stderr,
			"Exception - segment not present, exception #[%ld]\n",
			except);
		break;
	case 12:
		fprintf(stderr,
			"Exception - stack segment not present, exception #[%ld]\n",
			except);
		break;
	case 13:
		fprintf(stderr,
			"Exception - general protection, exception #[%ld]\n",
			except);
		break;
	case 14:
		fprintf(stderr,
			"Exception - page fault, exception #[%ld]\n", except);
		ret = 1;
		break;
	default:
		fprintf(stderr,
			"Exception type not handled... unknown exception #[%ld]\n",
			except);
		break;
	}

	if (ret) {
		if (scp->edi == (int)map_address) {
			fprintf(stdout,
				"page fault at [%#lx] - ignore\n", scp->edi);
			siglongjmp(jmpbuf, 1);
		} else if (scp->esi == (int)map_address) {
			fprintf(stdout,
				"page fault at [%#lx] - ignore\n", scp->esi);
			siglongjmp(jmpbuf, 1);
		} else {
			fprintf(stderr,
				"address at which sigfault occured: [%lx]\n"
				"address at which sigfault occured: [%lx]\n"
				"address at which memory was shmat: [%p]\n",
				(unsigned long)scp->edi,
				(unsigned long)scp->esi, map_address);
			fprintf(stderr, "bad page fault exit test\n");
			exit(-1);
		}
	} else
		exit(-1);
#else
	fprintf(stderr, "caught signal %d -- exiting.\n", signal);
	exit(-1);
#endif
}

										/******************************************************************************//*                                                                            */
/* Function:	usage							      */
/*									      */
/* Description:	Print the usage message.				      */
/*									      */
/* Return:	exits with -1						      */
/*									      */
/******************************************************************************/
static void usage(char *progname)
{				/* name of this program                       */
	fprintf(stderr,
		"Usage: %s -h -l -x\n"
		"\t -h help, usage message.\n"
		"\t -l number of map - write - unmap.    default: 1000\n"
		"\t -x time for which test is to be run. default: 24 Hrs\n",
		progname);
	exit(-1);
}

/******************************************************************************/
/*									      */
/* Function:	shmat_shmdt						      */
/*									      */
/* Description:	Thread X function.					      */
/*		shmat a random size file and shm-detach this file, this is    */
/*		done for user defined number of times.			      */
/*									      */
/* Input:	arg[0]		   number of times shmat shmdt is done        */
/*									      */
/* Return:	-1 on error.				                      */
/*               0 on errorless completion of the loop.                       */
/*									      */
/******************************************************************************/
void *shmat_shmdt(void *args)
{				/* arguments to the thread X function.          */
	int shm_ndx = 0;	/* index to number of shmat/shmdt             */
	key_t shmkey = 0;	/* IPC_PRIVATE (key for shmget)               */
	int shmid;		/* shared memory id                           */
	long *locargs =		/* local pointer to arguments                 */
	    (long *)args;

	while (shm_ndx++ < (int)locargs[0]) {
		/* put the reader and writer threads to sleep                         */
		done_shmat = 0;

		/* generate a random size, we will ask for this amount of shared mem  */
		srand(time(NULL) % 100);
		fsize = (1 + (int)(1000.0 * rand() / (RAND_MAX + 1.0))) * 4096;

		if ((shmid = shmget(shmkey, fsize, IPC_CREAT | 0666)) == -1) {
			perror("shmat_shmdt(): shmget()");
			pthread_exit((void *)-1);
		} else {
			fprintf(stdout,
				"%s[%#lx]: shmget(): success, got segment of size %d\n",
				STR_SHMAT, pthread_self(), fsize);
		}

		if ((map_address = shmat(shmid, NULL, 0))
		    == (void *)-1) {
			fprintf(stderr, "shmat_shmat(): map address = %p\n",
				map_address);
			perror("shmat_shmdt(): shmat()");
			pthread_exit((void *)-1);
		} else {
			/* Wake up the reader and writer threads.                             */
			/* Write 'X' into map_address. We are not sure about
			   reader/writer interleaving. So the reader may expect
			   to find 'X' or 'Y'
			 */
			memset(map_address, 'X', 1);
			done_shmat = 1;
			usleep(0);
		}

		fprintf(stdout, "%s[%#lx]: Map address = %p\n",
			STR_SHMAT, pthread_self(), map_address);
		fprintf(stdout,
			"%s[%#lx]: Num iter: [%d] Total Num Iter: [%d]\n",
			STR_SHMAT, pthread_self(), shm_ndx, (int)locargs[0]);
		usleep(0);
		sched_yield();

		/* put the threads to sleep before un-shmatting                       */
		done_shmat = 0;
		if (shmdt((void *)map_address) == -1) {
			perror("shmat_shmdt(): shmdt()");
			pthread_exit((void *)-1);
		}
		if (shmctl(shmid, IPC_RMID, NULL)) {
			perror("shmat_shmdt(): shmctl()");
			pthread_exit((void *)-1);
		}
	}
	pthread_exit(NULL);
}

/******************************************************************************/
/*									      */
/* Function:	write_to_mem						      */
/*									      */
/* Description:	Thread Y function.					      */
/*		Writes 'Y' to the memory location shmat by process X.         */
/*									      */
/* Input:	arg[0]		   number of times write is performed         */
/*									      */
/* Return:	-1 on error.				                      */
/*               0 on errorless completion of the loop.                       */
/*									      */
/******************************************************************************/
void *write_to_mem(void *args)
{
	static int write_ndx = 0;	/* index to the number of writes to perform   */
	long *locargs =		/* local pointer to the arguments             */
	    (long *)args;

	while (write_ndx++ < (int)locargs[0]) {
		/* wait for the thread to shmat, and dont sleep on the processor. */
		while (!done_shmat)
			usleep(0);

		if (sigsetjmp(jmpbuf, 1) == 1) {
			fprintf(stdout,
				"page fault ocurred due a write after an shmdt from [%p]\n",
				map_address);
		}

		fprintf(stdout,
			"%s[%#lx]: write_to_mem(): memory address: [%p]\n",
			STR_WRITER, pthread_self(), map_address);
		memset(map_address, 'Y', 1);
		usleep(1);
		sched_yield();
	}
	pthread_exit(NULL);
}

/******************************************************************************/
/*									      */
/* Function:	read_from_mem						      */
/*									      */
/* Description:	Thread Z function.					      */
/*		reads from the memory location shmat by process X.            */
/*									      */
/* Input:	arg[0]		   number of times read is performed          */
/*									      */
/* Return:	-1 on error.				                      */
/*               0 on errorless completion of the loop.                       */
/*									      */
/******************************************************************************/
void *read_from_mem(void *args)
{
	static int read_ndx = 0;	/* index to the number of writes to perform   */
	long *locargs =		/* local pointer to the arguments             */
	    (long *)args;

	while (read_ndx++ < (int)locargs[0]) {
		/* wait for the shmat to happen */
		while (!done_shmat)
			usleep(0);

		fprintf(stdout,
			"%s[%#lx]: read_from_mem():  memory address: [%p]\n",
			STR_READER, pthread_self(), map_address);
		if (sigsetjmp(jmpbuf, 1) == 1) {
			fprintf(stdout,
				"page fault ocurred due a read after an shmdt from %p\n",
				map_address);
		}

		fprintf(stdout, "%s[%#lx]: read_mem(): content of memory: %s\n",
			STR_READER, pthread_self(), (char *)map_address);

		if (strncmp(map_address, "Y", 1) != 0) {
			if (strncmp(map_address, "X", 1) != 0) {
				pthread_exit((void *)-1);
			}
		}
		usleep(1);
		sched_yield();
	}
	pthread_exit(NULL);
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Descrption:	Create a large file of size up to a  Giga Bytes.  write to it */
/*		lower case alphabet 'a'. Map the file and change the contents */
/*		to 'A's (upper case alphabet), write the contents to the file,*/
/*		and unmap the file from memory. Spwan a certian number of     */
/*		LWP's that will do the above.                                 */
/*                                                                            */
/* Return:	exits with -1 on error					      */
/*		exits with a 0 on success.				      */
/*                                                                            */
/******************************************************************************/
int main(int argc,		/* number of input parameters.                        */
	 char **argv)
{				/* pointer to the command line arguments.       */
	int c;			/* command line options                       */
	int num_iter;		/* number of iteration to perform             */
	int thrd_ndx;		/* index into the array of threads.           */
	double exec_time;	/* period for which the test is executed      */
	void *status;		/* exit status for light weight process       */
	int sig_ndx;		/* index into signal handler structure.       */
	pthread_t thid[1000];	/* pids of process that will map/write/unmap  */
	long chld_args[3];	/* arguments to funcs execed by child process */
	extern char *optarg;	/* arguments passed to each option            */
	struct sigaction sigptr;	/* set up signal, for interval timer          */

	static struct signal_info {
		int signum;	/* signal number that hasto be handled                */
		char *signame;	/* name of the signal to be handled.                  */
	} sig_info[] = {
		{
		SIGHUP, "SIGHUP"}, {
		SIGINT, "SIGINT"}, {
		SIGQUIT, "SIGQUIT"}, {
		SIGABRT, "SIGABRT"}, {
		SIGBUS, "SIGBUS"}, {
		SIGSEGV, "SIGSEGV"}, {
		SIGALRM, "SIGALRM"}, {
		SIGUSR1, "SIGUSR1"}, {
		SIGUSR2, "SIGUSR2"}, {
		-1, "ENDSIG"}
	};

	/* set up the default values */
	num_iter = 1000;	/* repeate map - write - unmap operation 1000 times   */
	exec_time = 24.0;	/* minimum time period for which to run the tests     */

	while ((c = getopt(argc, argv, "h:l:x:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 'l':	/* number of times to loop in the thread function     */
			if ((num_iter = atoi(optarg)) == 0)
				num_iter = 1000;
			break;
		case 'x':	/* time in hrs to run this test.                      */
			if ((exec_time = atof(optarg)) == 0)
				exec_time = 24;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	fprintf(stdout,
		"\n\n\nTest is set to run with the following parameters:\n"
		"\tDuration of test: [%f]hrs\n"
		"\tnumber of shmat  shm detach: [%d]\n", exec_time, num_iter);

	/* set up signals */
	sigptr.sa_handler = (void (*)(int signal))sig_handler;
	sigfillset(&sigptr.sa_mask);
	sigptr.sa_flags = SA_SIGINFO;
	for (sig_ndx = 0; sig_info[sig_ndx].signum != -1; sig_ndx++) {
		sigaddset(&sigptr.sa_mask, sig_info[sig_ndx].signum);
		if (sigaction(sig_info[sig_ndx].signum, &sigptr,
			      NULL) == -1) {
			perror("man(): sigaction()");
			fprintf(stderr,
				"could not set handler for SIGALRM, errno = %d\n",
				errno);
			exit(-1);
		}
	}

	chld_args[0] = num_iter;
	alarm(exec_time * 3600.00);

	for (;;) {
		/* create 3 threads */
		if (pthread_create(&thid[0], NULL, shmat_shmdt, chld_args)) {
			perror("main(): pthread_create()");
			exit(-1);
		} else {
			fprintf(stdout,
				"created thread id[%#lx], execs fn shmat_shmdt()\n",
				thid[0]);
		}
		sched_yield();

		if (pthread_create(&thid[1], NULL, write_to_mem, chld_args)) {
			perror("main(): pthread_create()");
			exit(-1);
		} else {
			fprintf(stdout,
				"created thread id[%#lx], execs fn write_to_mem()\n",
				thid[1]);
		}
		sched_yield();

		if (pthread_create(&thid[2], NULL, read_from_mem, chld_args)) {
			perror("main(): pthread_create()");
			exit(-1);
		} else {
			fprintf(stdout,
				"created thread id[%#lx], execs fn read_from_mem()\n",
				thid[2]);
		}
		sched_yield();

		/* wait for the children to terminate */
		for (thrd_ndx = 0; thrd_ndx < 3; thrd_ndx++) {
			if (pthread_join(thid[thrd_ndx], &status)) {
				perror("main(): pthread_create()");
				exit(-1);
			}
			if (status == (void *)-1) {
				fprintf(stderr,
					"thread [%#lx] - process exited with errors %ld\n",
					thid[thrd_ndx], (long)status);
				exit(-1);
			}
		}
	}
	fprintf(stdout, "TEST PASSED\n");
	return 0;
}
