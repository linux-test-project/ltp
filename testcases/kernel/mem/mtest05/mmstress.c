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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/******************************************************************************/
/*                                                                            */
/* File:         mmstress.c                                                   */
/*                                                                            */
/* Description:  This is a test program that performs general stress with     */
/*               memory race conditions. It contains seven testcases that     */
/*               will test race conditions between simultaneous read fault,   */
/*               write fault, copy on write (COW) fault e.t.c.                */
/*               This testcase is intended to execute on the Linux operating  */
/*               system and can be easily ported to work on other operating   */
/*               systems as well.                                             */
/*                                                                            */
/* Usage:        mmstress -h -n TEST NUMBER -p NPAGES -t EXECUTION TIME -v -V */
/*                        -h                - Help                            */
/*                        -n TEST NUMBER    - Execute a particular testcase   */
/*                        -p NPAGES         - Use NPAGES pages for tests    */
/*                        -t EXECUTION TIME - Execute test for a certain time */
/*                        -v                - Verbose output                  */
/*                        -V                - Version of this program         */
/*                                                                            */
/* Author:       Manoj Iyer - manjo@mail.utexas.edu                           */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Apr-13-2001    Created: Manoj Iyer, IBM Austin.                            */
/*        These tests are adapted from AIX vmm FVT tests.                     */
/*                                                                            */
/* Oct-24-2001  Modified.                                                     */
/*        - freed buffers that were allocated.                                */
/*        - closed removed files. This will remove the disk full error        */
/*        - use pthread_exit in case of theads instead of return. This        */
/*          was really bad to use return!                                     */
/*        - created usage function.                                           */
/*        - pthread_join checks for thread exit status reported by            */
/*          pthread_exit()                                                    */
/*                                                                            */
/* Oct-25-2001  Modified.                                                     */
/*        - Fixed bug in usage()                                              */
/*        - malloc'ed pointer for pthread return value.                       */
/*        - changed scheme. If no options are specified, all the tests        */
/*          will be run once.                                                 */
/*                                                                            */
/* Nov-02-2001  Modified - Paul Larson                                        */
/*        - Added sched_yield to thread_fault to fix hang                     */
/*        - Removed thread_mmap                                               */
/*                                                                            */
/* Nov-09-2001  Modified - Manoj Iyer                                         */
/*        - Removed compile warnings.                                         */
/*        - Added missing header file. #include <stdlib.h>                    */
/*                                                                            */
/* Oct-28-2003  Modified - Manoj Iyer                                         */
/*        - missing parenthesis added.                                        */
/*        - formatting changes.                                               */
/*        - increased NUMPAGES to 9999.                                       */
/*                                                                            */
/* Jan-30-2003  Modified - Gary Williams                                      */
/*        - fixed a race condition between the two threads                    */
/*        - made it so if any of the testcases fail the test will fail        */
/*        - fixed so status of child in test 6 is used to determine result    */
/*        - fixed the use of the remove_files function in a conditional       */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <stdint.h>
#include <getopt.h>

#include "test.h"

/* GLOBAL DEFINES                                                             */
#define SIGENDSIG    -1		/* end of signal marker                             */
#define THNUM        0		/* array element pointing to number of threads      */
#define MAPADDR      1		/* array element pointing to map address            */
#define PAGESIZ      2		/* array element pointing to page size              */
#define FLTIPE       3		/* array element pointing to fault type             */
#define READ_FAULT   0		/* instructs routine to simulate read fault         */
#define WRITE_FAULT  1		/* instructs routine to simulate write fault        */
#define COW_FAULT    2		/* instructs routine to simulate copy-on-write fault */
#define NUMTHREAD    32		/* number of threads to spawn default to 32         */
#define NUMPAGES     9999	/* default (random) value of number of pages        */
#ifndef TRUE
#define TRUE         1
#endif
#ifndef FALSE
#define FALSE        0
#endif
#define FAILED       (-1)	/* return status for all funcs indicating failure   */
#define SUCCESS      0		/* return status for all routines indicating success */

#define BRKSZ        512*1024	/* program data space allocation value          */

static volatile int wait_thread;	/* used to wake up sleeping threads    */
static volatile int thread_begin;	/* used to coordinate threads          */
static int verbose_print = FALSE;	/* print more test information           */

static int pages_num = NUMPAGES;	/* number of pages to use for tests     */
static volatile int alarm_fired;

char *TCID = "mmstress";
int TST_TOTAL = 6;

static void sig_handler(int signal)
{
	if (signal != SIGALRM) {
		fprintf(stderr,
			"sig_handlder(): unexpected signal caught [%d]\n",
			signal);
		exit(TBROK);
	}

	alarm_fired = 1;
}

static void usage(char *progname)
{
	fprintf(stderr, "usage:%s -h -n test -t time -v [-V]\n", progname);
	fprintf(stderr, "\t-h displays all options\n");
	fprintf(stderr, "\t-n test number, if no test number\n"
		"\t   is specified, all the tests will be run\n");
	fprintf(stderr, "\t-p specify the number of pages to\n"
		"\t   use for allocation\n");
	fprintf(stderr, "\t-t specify the time in hours\n");
	fprintf(stderr, "\t-v verbose output\n");
	fprintf(stderr, "\t-V program version\n");
	exit(1);
}

static void set_timer(int run_time)
{
	struct itimerval timer;

	memset(&timer, 0, sizeof(struct itimerval));
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = (time_t) (run_time * 3600.0);

	if (setitimer(ITIMER_REAL, &timer, NULL)) {
		perror("set_timer(): setitimer()");
		exit(1);
	}
}

/******************************************************************************/
/*                                                                            */
/* Function:    thread_fault                                                  */
/*                                                                            */
/* Description: Executes as a thread function and accesses the memory pages   */
/*              depending on the fault_type to be generated. This function    */
/*              can cause READ fault, WRITE fault, COW fault.                 */
/*                                                                            */
/* Input:       void *args - argments passed to the exec routine by           */
/*              pthread_create()                                              */
/*                                                                            */
/******************************************************************************/
static void *thread_fault(void *args)
{
	long *local_args = args;	/* local pointer to list of arguments        */
	/* local_args[THNUM]   - the thread number   */
	/* local_args[MAPADDR] - map address         */
	/* local_args[PAGESIZ] - page size           */
	/* local_args[FLTIPE]  - fault type          */
	int pgnum_ndx = 0;	/* index to the number of pages              */
	char *start_addr	/* start address of the page                 */
	    = (void *) (local_args[MAPADDR]
			 + (int)local_args[THNUM]
			 * (pages_num / NUMTHREAD)
			 * local_args[PAGESIZ]);
	char read_from_addr = 0;	/* address to which read from page is done   */
	char write_to_addr[] = { 'a' };	/* character to be writen to the page    */

    /*************************************************************/
	/*   The way it was, args could be overwritten by subsequent uses
	 *   of it before this routine had a chance to use the data.
	 *   This flag stops the overwrite until this routine gets to
	 *   here.  At this point, it is done initializing and it is
	 *   safe for the parent thread to continue (which will change
	 *   args).
	 */
	thread_begin = FALSE;

	while (wait_thread)
		sched_yield();

	for (; pgnum_ndx < (pages_num / NUMTHREAD); pgnum_ndx++) {
		/* if the fault to be generated is READ_FAULT, read from the page     */
		/* else write a character to the page.                                */
		((int)local_args[3] == READ_FAULT) ? (read_from_addr =
						      *start_addr)
		    : (*start_addr = write_to_addr[0]);
		start_addr += local_args[PAGESIZ];
		if (verbose_print)
			tst_resm(TINFO,
				 "thread_fault(): generating fault type %ld"
				 " @page address %p", local_args[3],
				 start_addr);
		fflush(NULL);
	}
	pthread_exit(NULL);
}

/******************************************************************************/
/*                                                                            */
/* Function:    remove_tmpfiles                                               */
/*                                                                            */
/* Description: remove temporary files that were created by the tests.        */
/*                                                                            */
/******************************************************************************/
static int remove_files(char *filename, char *addr)
{
	if (addr)
		if (munmap(addr, sysconf(_SC_PAGESIZE) * pages_num) < 0) {
			perror("map_and_thread(): munmap()");
			return FAILED;
		}
	if (strcmp(filename, "NULL") && strcmp(filename, "/dev/zero")) {
		if (unlink(filename)) {
			perror("map_and_thread(): ulink()");
			return FAILED;
		}
	} else {
		if (verbose_print)
			tst_resm(TINFO, "file %s removed", filename);

	}
	return SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* Function:    map_and_thread                                                */
/*                                                                            */
/* Description: Creates mappings with the required properties, of MAP_PRIVATE */
/*              MAP_SHARED and of PROT_RED / PROT_READ|PROT_WRITE.            */
/*              Create threads and execute a routine that will generate the   */
/*              desired fault condition, viz, read, write and cow fault.      */
/*                                                                            */
/* Input:       char *tmpfile - name of temporary file that is created        */
/*              int   fault_type - type of fault that is to be generated.     */
/*                                                                            */
/******************************************************************************/
int map_and_thread(char *tmpfile,
			  void *(*exec_func) (void *),
			  int fault_type,
			  int num_thread)
{
	int fd = 0;		/* file descriptor of the file created       */
	int thrd_ndx = 0;	/* index to the number of threads created    */
	int map_type = 0;	/* specifies the type of the mapped object   */
	void *th_status;	/* status of the thread when it is finished  */
	long th_args[5];	/* argument list passed to  thread_fault()   */
	char *empty_buf = NULL;	/* empty buffer used to fill temp file       */
	long pagesize		/* contains page size at runtime             */
	    = sysconf(_SC_PAGESIZE);
	static pthread_t pthread_ids[NUMTHREAD];
	/* contains ids of the threads created       */
	void * map_addr = NULL;	/* address where the file is mapped          */

	/* Create a file with permissions 0666, and open it with RDRW perms       */
	/* if the name is not a NULL                                              */

	if (strcmp(tmpfile, "NULL")) {
		if ((fd =
		     open(tmpfile, O_RDWR | O_CREAT,
			  S_IRWXO | S_IRWXU | S_IRWXG))
		    == -1) {
			perror("map_and_thread(): open()");
			close(fd);
			fflush(NULL);
			return FAILED;
		}

		/* Write pagesize * pages_num bytes to the file */
		empty_buf = malloc(pagesize * pages_num);
		if (write(fd, empty_buf, pagesize * pages_num) !=
		    (pagesize * pages_num)) {
			perror("map_and_thread(): write()");
			free(empty_buf);
			fflush(NULL);
			remove_files(tmpfile, NULL);
			close(fd);
			return FAILED;
		}
		map_type = (fault_type == COW_FAULT) ? MAP_PRIVATE : MAP_SHARED;

		/* Map the file, if the required fault type is COW_FAULT map the file */
		/* private, else map the file shared. if READ_FAULT is required to be */
		/* generated map the file with read protection else map with read -   */
		/* write protection.                               */

		if ((map_addr = (void *) mmap(0, pagesize * pages_num,
					       ((fault_type == READ_FAULT) ?
						PROT_READ : PROT_READ |
						PROT_WRITE), map_type, fd, 0))
		    == MAP_FAILED) {
			perror("map_and_thread(): mmap()");
			free(empty_buf);
			fflush(NULL);
			remove_files(tmpfile, NULL);
			close(fd);
			return FAILED;
		} else {
			if (verbose_print)
				tst_resm(TINFO,
					 "map_and_thread(): mmap success, address = %p",
					 map_addr);
			fflush(NULL);
		}
	}

	/* As long as wait is set to TRUE, the thread that will be created will */
	/* loop in its exec routine */

	wait_thread = TRUE;

	/* Create a few threads, ideally number of threads equals number of CPU'S */
	/* so that we can assume that each thread will run on a single CPU in     */
	/* of SMP machines. Currently we will create NR_CPUS number of threads.   */

	th_args[1] = (long)map_addr;
	th_args[2] = pagesize;
	th_args[3] = fault_type;
	do {
		th_args[0] = thrd_ndx;
		th_args[4] = (long)0;

       /*************************************************************/
		/*   The way it was, args could be overwritten by subsequent uses
		 *   of it before the called routine had a chance to fully initialize.
		 *   This flag stops the overwrite until that routine gets to
		 *   begin.  At that point, it is done initializing and it is
		 *   safe for the this thread to continue (which will change
		 *   args).
		 *   A basic race condition.
		 */
		thread_begin = TRUE;
		if (pthread_create(&pthread_ids[thrd_ndx++], NULL, exec_func,
				   (void *)&th_args)) {
			perror("map_and_thread(): pthread_create()");
			thread_begin = FALSE;
			free(empty_buf);
			fflush(NULL);
			remove_files(tmpfile, map_addr);
			close(fd);
			return FAILED;
		} else {
	    /***************************************************/
			/*   Yield until new thread is done with args.
			 */
			while (thread_begin)
				sched_yield();
		}
	} while (thrd_ndx < num_thread);

	if (verbose_print)
		tst_resm(TINFO, "map_and_thread(): pthread_create() success");
	wait_thread = FALSE;

	/* suspend the execution of the calling thread till the execution of the  */
	/* other thread has been terminated.                                      */

	for (thrd_ndx = 0; thrd_ndx < NUMTHREAD; thrd_ndx++) {
		if (pthread_join(pthread_ids[thrd_ndx], &th_status)) {
			perror("map_and_thread(): pthread_join()");
			free(empty_buf);
			fflush(NULL);
			remove_files(tmpfile, map_addr);
			close(fd);
			return FAILED;
		} else {
			if ((long)th_status == 1) {
				tst_resm(TINFO,
					 "thread [%ld] - process exited with errors",
					 (long)pthread_ids[thrd_ndx]);
				free(empty_buf);
				remove_files(tmpfile, map_addr);
				close(fd);
				exit(1);
			}
		}
	}

	/* remove the temporary file that was created. - clean up                 */
	/* but dont try to remove special files.                                  */

    /***********************************************/
	/*   Was if !(remove_files()) ...
	 *   If that routine succeeds, it returns SUCCESS, which
	 *   happens to be 0.  So if the routine succeeded, the
	 *   above condition would indicate failure.  This change
	 *   fixes that.
	 */
	if (remove_files(tmpfile, map_addr) == FAILED) {
		free(empty_buf);
		return FAILED;
	}

	free(empty_buf);
	close(fd);
	return SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous read  */
/*              faults in the same address space.                             */
/*                                                                            */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause read faults by simultaneously reading*/
/*              from this memory space.                                       */
/******************************************************************************/
static int test1(void)
{
	tst_resm(TINFO, "test1: Test case tests the race condition between "
		 "simultaneous read faults in the same address space.");
	return map_and_thread("./tmp.file.1", thread_fault, READ_FAULT, NUMTHREAD);
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous write */
/*              faults in the same address space.                             */
/*                                                                            */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause write faults by simultaneously       */
/*              writing to this memory space.                                 */
/******************************************************************************/
static int test2(void)
{
	tst_resm(TINFO, "test2: Test case tests the race condition between "
		 "simultaneous write faults in the same address space.");
	return map_and_thread("./tmp.file.2", thread_fault, WRITE_FAULT, NUMTHREAD);
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous COW   */
/*              faults in the same address space.                             */
/*                                                                            */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause COW faults by simultaneously         */
/*              writing to this memory space.                                 */
/*                                                                            */
/******************************************************************************/
static int test3(void)
{
	tst_resm(TINFO, "test3: Test case tests the race condition between "
		 "simultaneous COW faults in the same address space.");
	return map_and_thread("./tmp.file.3", thread_fault, COW_FAULT, NUMTHREAD);
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous READ  */
/*              faults in the same address space. File mapped is /dev/zero    */
/*                                                                            */
/* Description: Map a file into memory, create threads and execute a thread   */
/*              function that will cause READ faults by simultaneously        */
/*              writing to this memory space.                                 */
/*                                                                            */
/******************************************************************************/
static int test4(void)
{
	tst_resm(TINFO, "test4: Test case tests the race condition between "
		 "simultaneous READ faults in the same address space. "
		 "The file mapped is /dev/zero");
	return map_and_thread("/dev/zero", thread_fault, COW_FAULT, NUMTHREAD);
}

/******************************************************************************/
/*                                                                            */
/* Test:    Test case tests the race condition between simultaneous           */
/*         fork - exit faults in the same address space.                      */
/*                                                                            */
/* Description: Initialize large data in the parent process, fork a child and */
/*              and the parent waits for the child to complete execution.     */
/*                                                                            */
/******************************************************************************/
static int test5(void)
{
	int fork_ndx = 0;
	pid_t pid = 0;
	int wait_status = 0;

	tst_resm(TINFO, "test5: Test case tests the race condition between "
		 "simultaneous fork - exit faults in the same address space.");

	/* increment the  program's  data  space  by 200*1024 (BRKSZ) bytes       */

	if (sbrk(BRKSZ) == (void *) - 1) {
		perror("test5(): sbrk()");
		fflush(NULL);
		return FAILED;
	}

	/* fork NUMTHREAD number of processes, assumption is on SMP each will get */
	/* a separate CPU if NRCPUS = NUMTHREAD. The child does nothing; exits    */
	/* immediately, parent waits for child to complete execution.             */
	do {
		if (!(pid = fork()))
			_exit(0);
		else {
			if (pid != -1)
				wait(&wait_status);
		}

	} while (fork_ndx++ < NUMTHREAD);

	if (sbrk(-BRKSZ) == (void *) - 1) {
		tst_resm(TINFO, "test5(): rollback sbrk failed");
		fflush(NULL);
		perror("test5(): sbrk()");
		fflush(NULL);
		return FAILED;
	}
	return SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous       */
/*              fork - exec - exit faults in the same address space.          */
/*                                                                            */
/* Description: Initialize large data in the parent process, fork a child and */
/*              and the parent waits for the child to complete execution. The */
/*              child program execs a dummy program.                          */
/*                                                                            */
/******************************************************************************/
static int test6(void)
{
	int res = SUCCESS;
	int fork_ndx = 0;
	pid_t pid = 0;
	int wait_status;
	char *argv_init[2] = { "arg1", NULL };

	tst_resm(TINFO, "test6: Test case tests the race condition between "
		 "simultaneous fork -exec - exit faults in the same address space.");

	/* increment the  program's  data  space  by 200*1024 (BRKSZ) bytes       */
	if (sbrk(BRKSZ) == (void *) - 1) {
		perror("test6(): sbrk()");
		fflush(NULL);
		return FAILED;
	}

	/* fork NUMTHREAD number of processes, assumption is on SMP each will get */
	/* a separate CPU if NRCPUS = NUMTHREAD. The child execs a dummy program  */
	/*  and parent waits for child to complete execution.                     */
	do {
		if (!(pid = fork())) {
			if (execvp("mmstress_dummy", argv_init) == -1) {
				if (execvp("./mmstress_dummy", argv_init) == -1) {
					perror("test6(): execvp()");
					fflush(NULL);
					exit(99);
				}
			}
		} else {
			if (pid != -1)
				wait(&wait_status);

			if (WEXITSTATUS(wait_status) != 0)
				res = FAILED;
		}

	} while (fork_ndx++ < NUMTHREAD);

	if (sbrk(-BRKSZ) == (void *) - 1) {
		tst_resm(TINFO, "test6(): rollback sbrk failed");
		fflush(NULL);
		perror("test6(): sbrk()");
		fflush(NULL);
		return FAILED;
	}

	return res;
}

static int (*(test_ptr)[]) () = {test1, test2, test3, test4, test5, test6};

static void run_test(unsigned int i)
{
	int rc;

	rc = test_ptr[i]();

	if (rc == SUCCESS)
		tst_resm(TPASS, "TEST %d Passed", i + 1);
	else
		tst_resm(TFAIL, "TEST %d Failed", i + 1);

	if (alarm_fired)
		tst_exit();
}

int main(int argc, char **argv)
{
	static char *version_info = "mmstress V1.00 04/17/2001";
	int ch;
	unsigned int i;
	int test_num = 0;
	int test_time = 0;
	int run_once = TRUE;

	static struct signal_info {
		int signum;
		char *signame;
	} sig_info[] = {
		{SIGHUP, "SIGHUP"},
		{SIGINT, "SIGINT"},
		{SIGQUIT, "SIGQUIT"},
		{SIGABRT, "SIGABRT"},
		{SIGBUS, "SIGBUS"},
		{SIGSEGV, "SIGSEGV"},
		{SIGALRM, "SIGALRM"},
		{SIGUSR1, "SIGUSR1"},
		{SIGUSR2, "SIGUSR2"},
		{SIGENDSIG, "ENDSIG"}
	};

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	if (argc < 2)
		tst_resm(TINFO, "run %s -h for all options", argv[0]);

	while ((ch = getopt(argc, argv, "hn:p:t:vV")) != -1) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			break;
		case 'n':
			test_num = atoi(optarg);
			break;
		case 'p':
			pages_num = atoi(optarg);
			break;
		case 't':
			tst_resm(TINFO,
				 "Test is scheduled to run for %d hours",
				 test_time = atoi(optarg));
			run_once = FALSE;
			break;
		case 'v':
			verbose_print = TRUE;
			break;
		case 'V':
			tst_resm(TINFO, "%s: %s", argv[0], version_info);
			break;
		case '?':
			fprintf(stderr,
				"%s: unknown option - %c ignored\n",
				argv[0], optopt);
			break;
		default:
			tst_brkm(TBROK, NULL, "%s: getopt() failed!!!\n",
				 argv[0]);
		}
	}

	set_timer(test_time);

	for (i = 0; sig_info[i].signum != -1; i++) {
		if (signal(sig_info[i].signum, sig_handler) == SIG_ERR) {
			tst_brkm(TBROK | TERRNO, NULL, "signal(%s) failed",
				 sig_info[i].signame);
		}
	}

	tst_tmpdir();

	do {
		if (!test_num) {
			for (i = 0; i < ARRAY_SIZE(test_ptr); i++)
				run_test(i);
		} else {
			if (test_num > (int)ARRAY_SIZE(test_ptr)) {
				tst_brkm(TBROK, NULL, "Invalid test number %i",
					 test_num);
			}

			run_test(test_num-1);
		}
	} while (!run_once);

	tst_rmdir();
	tst_exit();
}
