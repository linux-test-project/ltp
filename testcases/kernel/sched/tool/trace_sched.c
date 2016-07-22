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
/*			                                                      */
/* History:	Feb - 21 - 2002 Created - Manoj Iyer, IBM Austin TX.          */
/*				          email: manjo@austin.ibm.com.        */
/*			                                                      */
/* 		Feb - 25 - 2002 Modified - Manoj Iyer, IBM Austin TX.         */
/*		                - Added structure thread_sched_t.             */
/*				- Added logic to specify scheduling policy.   */
/*			                                                      */
/*		Feb - 25 - 2002 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- Added header file string.h.	              */
/*				- Removed dead variable ppid from thread_func.*/
/*				- Fixed date from 2001 to 2002 in History.    */
/*			                                                      */
/* File:	trace_sched.c						      */
/*			                                                      */
/* Description:	This utility spawns N tasks, each task sets its priority by   */
/*		making a system call to the scheduler. The thread function    */
/*		reads the priority that tbe schedular sets for this task and  */
/*		also reads from /proc the processor this task last executed on*/
/*		the information that is gathered by the thread function may   */
/*		be in real-time. Its only an approximation.                   */
/*			                                                      */
/******************************************************************************/

#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <string.h>

void noprintf(char *string, ...)
{
}

#ifdef DEBUG			/* compile with this flag for debug, use dprt in code */
#define dprt    printf
#else
#define dprt    noprintf
#endif

#ifndef PID_MAX
#define PID_MAX 0x8000
#endif

#define MAXT 100

#ifdef PTHREAD_THREADS_MAX
#define PIDS PTHREAD_THREADS_MAX	/* maximum thread allowed.                     */
#elif defined(PID_MAX_DEFAULT)
#define PIDS PID_MAX_DEFAULT	/* maximum pids allowed.                       */
#else
#define PIDS PID_MAX		/* alternative way maximum pids may be defined */
#endif

#define UP   1			/* assume UP if no SMP value is specified.     */

#define OPT_MISSING(prog, opt)   do{\
                               fprintf(stderr, "%s: option -%c ", prog, opt); \
                               fprintf(stderr, "requires an argument\n"); \
                               usage(prog); \
                                   } while (0)

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }

typedef struct {		/* contains priority and CPU info of the task.        */
	int exp_prio;		/* priority that we wish to set.                      */
	int act_prio;		/* priority set by the scheduler.                     */
	int proc_num;		/* last processor on which this task executed.        */
	int procs_id;		/* pid of this task.                                  */
	int s_policy;		/* scheduling policy for the task.                    */
} thread_sched_t;

int verbose = 0;		/* set verbose printing, makes output look ugly!      */

/******************************************************************************/
/*                                                                            */
/* Function:    usage                                                         */
/*                                                                            */
/* Description: Print the usage message.                                      */
/*                                                                            */
/* Return:      exits with -1                                                 */
/*                                                                            */
/******************************************************************************/
void usage(char *progname)
{				/* name of this program                                 */
	fprintf(stderr,
		"Usage: %s -c NCPU -h -p [fifo:rr:other] -t THREADS -v\n"
		"\t -c Number of CUPS in the machine. User MUST provide\n"
		"\t -h Help!\n"
		"\t -p Scheduling policy, choice: fifo, rr, other. Default: fifo\n"
		"\t -t Number of threads to create.                Default: %d\n"
		"\t -v Verbose out put, print ugly!.               Default: OFF\n",
		progname, MAXT);
	exit(-1);
}

/******************************************************************************/
/*                                                                            */
/* Function:    get_proc_num                                                  */
/*                                                                            */
/* Description: Function reads the proc filesystem file /proc/<PID>/stat      */
/*		gets the CPU number this process last executed on and returns */
/*		Some hard assumptions were made regarding buffer sizes.       */
/*                                                                            */
/* Return:      exits with -1 - on error                                      */
/*              CPU number - on success				              */
/*                                                                            */
/******************************************************************************/
static int get_proc_num(void)
{
	int fd = -1;		/* file descriptor of the /proc/<pid>/stat file.      */
	int fsize = -1;		/* size of the /proc/<pid>/stat file.                 */
	char filename[256];	/* buffer to hold the string /proc/<pid>/stat.        */
	char fbuff[512];	/* contains the contents of the stat file.            */

	/* get the name of the stat file for this process */
	sprintf(filename, "/proc/%d/stat", getpid());

	/* open the stat file and read the contents to a buffer */
	if ((fd = open(filename, O_RDONLY)) == -1) {
		perror("get_proc_num(): open()");
		return -1;
	}

	usleep(6);
	sched_yield();

	if ((fsize = read(fd, fbuff, 512)) == -1) {
		perror("main(): read()");
		return -1;
	}

	close(fd);
	/* return the processor number last executed on. */
	return atoi(&fbuff[fsize - 2]);
}

/******************************************************************************/
/*                                                                            */
/* Function:    thread_func                                                   */
/*                                                                            */
/* Description: This function is executed in the context of the new task that */
/*		pthread_createi() will spawn. The (thread) task will get the  */
/*		minimum and maximum static priority for this system, set the  */
/*		priority of the current task to a random priority value if    */
/*		the policy set if SCHED_FIFO or SCHED_RR. The priority if this*/
/*		task that was assigned by the scheduler is got from making the*/
/*		system call to sched_getscheduler(). The CPU number on which  */
/*		the task was last seen is also recorded. All the above data is*/
/*		returned to the calling routine in a structure thread_sched_t.*/
/*                                                                            */
/* Input:       thread_sched_t 						      */
/*		    s_policy - scheduling policy for the task.                */
/*                                                                            */
/* Return:      thread_sched_t - on success.				      */
/*		    exp_prio - random priority value to set.                  */
/*		    act_prio - priority set by the scheduler.                 */
/*		    proc_num - CPU number on which this task last executed.   */
/*		    procs_id -  pid of this task.                             */
/*                                                                            */
/*		-1 	       - on error.                                    */
/*                                                                            */
/******************************************************************************/
void *thread_func(void *args)
{				/* arguments to the thread function           */
	static int max_priority;	/* max possible priority for a process.       */
	static int min_priority;	/* min possible priority for a process.       */
	static int set_priority;	/* set the priority of the proc by this value. */
	static int get_priority;	/* get the priority that is set for this proc. */
	static int procnum;	/* processor number last executed on.         */
	static int sched_policy;	/* scheduling policy as set by user/default   */
	struct sched_param ssp;	/* set schedule priority.                     */
	struct sched_param gsp;	/* gsp schedule priority.                     */
	struct timeb tptr;	/* tptr.millitm will be used to seed srand.   */
	thread_sched_t *locargptr =	/* local ptr to the arguments.                */
	    (thread_sched_t *) args;

	/* Get the system max and min static priority for a process. */
	if (((max_priority = sched_get_priority_max(SCHED_FIFO)) == -1) ||
	    ((min_priority = sched_get_priority_min(SCHED_FIFO)) == -1)) {
		fprintf(stderr, "failed to get static priority range\n");
		dprt("pid[%d]: exiting with -1\n", getpid());
		pthread_exit((void *)-1);
	}

	if ((sched_policy = locargptr->s_policy) == SCHED_OTHER)
		ssp.sched_priority = 0;
	else {
		/* Set a random value between max_priority and min_priority */
		ftime(&tptr);
		srand((tptr.millitm) % 1000);
		set_priority = (min_priority + (int)((float)max_priority
						     * rand() / (RAND_MAX +
								 1.0)));
		ssp.sched_priority = set_priority;
	}

	/* give other threads a chance */
	usleep(8);

	/* set a random priority value and check if this value was honoured. */
	if ((sched_setscheduler(getpid(), sched_policy, &ssp)) == -1) {
		perror("main(): sched_setscheduler()");
		dprt("pid[%d]: exiting with -1\n", getpid());
		pthread_exit((void *)-1);
	}

	/* processor number this process last executed on */
	if ((procnum = get_proc_num()) == -1) {
		fprintf(stderr, "main(): get_proc_num() failed\n");
		dprt("pid[%d]: exiting with -1\n", getpid());
		pthread_exit((void *)-1);
	}

	if ((get_priority = sched_getparam(getpid(), &gsp)) == -1) {
		perror("main(): sched_setscheduler()");
		dprt("pid[%d]: exiting with -1\n", getpid());
		pthread_exit((void *)-1);
	}

	/* processor number this process last executed on */
	if ((procnum = get_proc_num()) == -1) {
		fprintf(stderr, "main(): get_proc_num() failed\n");
		dprt("pid[%d]: exiting with -1\n", getpid());
		pthread_exit((void *)-1);
	}

	if (verbose) {
		fprintf(stdout,
			"PID of this task         = %d\n"
			"Max priority             = %d\n"
			"Min priority             = %d\n"
			"Expected priority        = %d\n"
			"Actual assigned priority = %d\n"
			"Processor last execed on = %d\n\n", getpid(),
			max_priority, min_priority, set_priority,
			gsp.sched_priority, procnum);
	}

	locargptr->exp_prio = set_priority;
	locargptr->act_prio = gsp.sched_priority;
	locargptr->proc_num = procnum;
	locargptr->procs_id = getpid();

	dprt("pid[%d]: exiting with %ld\n", getpid(), locargptr);
	pthread_exit((void *)locargptr);
}

/******************************************************************************/
/*                                                                            */
/* Function:    main						              */
/*                                                                            */
/* Description: Entry point of the program, parse options, check for their    */
/*		validity, spawn N tasks, wait for them to return, in the end  */
/*		print all the data that the thiread function collected.       */
/*                                                                            */
/* Return:      exits with -1 - on error.                                     */
/*		exits with  0 - on success.				      */
/*                                                                            */
/******************************************************************************/
int main(int argc,		/* number of input parameters.                        */
	 char **argv)
{				/* pointer to the command line arguments.       */
	int c;			/* command line options.                      */
	int proc_ndx;		/* number of time to repete the loop.         */
	int pid_ndx;		/* number of time to repete the loop.         */
	int num_cpus = UP;	/* assume machine is an UP machine.           */
	int num_thrd = MAXT;	/* number of threads to create.               */
	int thrd_ndx;		/* index into the array of threads.           */
	int exp_prio[PIDS];	/* desired priority, random value.            */
	int act_prio[PIDS];	/* priority actually set.                     */
	int gen_pid[PIDS];	/* pid of the processes on this processor.    */
	int proc_id[PIDS];	/* id of the processor last execed on.        */
	int spcy = SCHED_FIFO;	/* scheduling policy for the tasks.           */
	pthread_t thid[PIDS];	/* pids of process or threads spawned         */
	thread_sched_t *chld_args;	/* arguments to funcs execed by child process. */
	thread_sched_t *status;	/* exit status for light weight process.      */
	extern char *optarg;	/* arguments passed to each option.           */
	thread_sched_t **args_table;	/* pointer table of arguments address         */
	thread_sched_t **status_table;	/*pointer table of status address          */

	if (getuid() != 0) {
		fprintf(stderr,
			"ERROR: Only root user can run this program.\n");
		usage(argv[0]);
	}

	if (argc < 2) {
		fprintf(stderr,
			"ERROR: Enter a value for the number of CPUS\n");
		usage(argv[0]);
	}

	while ((c = getopt(argc, argv, "c:hp:t:v")) != -1) {
		switch (c) {
		case 'c':	/* number of processors. no default. */
			if ((num_cpus = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_cpus < 0) {
				fprintf(stdout,
					"WARNING: Bad argument -p %d. Using default\n",
					num_cpus);
				num_cpus = UP;
			}
			/* MAXT threads per cpu. */
			num_thrd = num_thrd * num_cpus;
			break;
		case 'h':	/* usage message */
			usage(argv[0]);
			break;
		case 'p':	/* schedular policy. default SCHED_FIFO */
			if (strncmp(optarg, "fifo", 4) == 0)
				spcy = SCHED_FIFO;
			else if (strncmp(optarg, "rr", 2) == 0)
				spcy = SCHED_RR;
			else if (strncmp(optarg, "other", 5) == 0)
				spcy = SCHED_OTHER;
			else {
				fprintf(stderr,
					"ERROR: Unrecognized scheduler policy,"
					"using default\n");
				usage(argv[0]);
			}
			break;
		case 't':	/* input how many threads to create */
			if ((num_thrd = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_thrd < 0) {
				fprintf(stderr,
					"WARNING: Bad argument -t %d. Using default\n",
					num_thrd);
				num_thrd = MAXT;
			} else if (num_thrd > PIDS) {
				fprintf(stderr,
					"WARNING: -t %d exceeds maximum number of allowed pids"
					" %d\n Setting number of threads to %d\n",
					num_thrd, PIDS, PIDS - 1000);
				num_thrd = (PIDS - 1000);
			}
			break;
		case 'v':	/* verbose out put, make output look ugly! */
			verbose = 1;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	/* create num_thrd number of threads. */
	args_table = malloc(num_thrd * sizeof(thread_sched_t *));
	if (!args_table) {
		perror("main(): malloc failed");
		exit(-1);
	}
	for (thrd_ndx = 0; thrd_ndx < num_thrd; thrd_ndx++) {
		args_table[thrd_ndx] = malloc(sizeof(thread_sched_t));
		if (!args_table[thrd_ndx]) {
			perror("main(): malloc failed");
			exit(-1);
		}
		chld_args = args_table[thrd_ndx];
		chld_args->s_policy = spcy;
		if (pthread_create(&thid[thrd_ndx], NULL, thread_func,
				   chld_args)) {
			fprintf(stderr, "ERROR: creating task number: %d\n",
				thrd_ndx);
			perror("main(): pthread_create()");
			exit(-1);
		}
		if (verbose)
			fprintf(stdout, "Created thread[%d]\n", thrd_ndx);
		usleep(9);
		sched_yield();
	}

	/* wait for the children to terminate */
	status_table = malloc(num_thrd * sizeof(thread_sched_t *));
	if (!status_table) {
		perror("main(): malloc failed");
		exit(-1);
	}
	for (thrd_ndx = 0; thrd_ndx < num_thrd; thrd_ndx++) {
		status_table[thrd_ndx] = malloc(sizeof(thread_sched_t));
		if (!status_table[thrd_ndx]) {
			perror("main(): malloc failed");
			exit(-1);
		}
		status = status_table[thrd_ndx];
		if (pthread_join(thid[thrd_ndx], (void **)&status)) {
			perror("main(): pthread_join()");
			exit(-1);
		} else {
			if (status == (thread_sched_t *) - 1) {
				fprintf(stderr,
					"thread [%d] - process exited with exit code -1\n",
					thrd_ndx);
				exit(-1);
			} else {
				exp_prio[thrd_ndx] = status->exp_prio;
				act_prio[thrd_ndx] = status->act_prio;
				proc_id[thrd_ndx] = status->proc_num;
				gen_pid[thrd_ndx] = status->procs_id;
			}
		}
		SAFE_FREE(args_table[thrd_ndx]);
		SAFE_FREE(status_table[thrd_ndx]);
		usleep(10);
	}

	if (verbose) {
		fprintf(stdout,
			"Number of tasks spawned: %d\n"
			"Number of CPUs:          %d\n"
			"Scheduling policy:       %d\n", num_thrd, num_cpus,
			spcy);
	}

	SAFE_FREE(args_table);
	SAFE_FREE(status_table);

	for (proc_ndx = 0; proc_ndx < num_cpus; proc_ndx++) {
		fprintf(stdout, "For processor number = %d\n", proc_ndx);
		fprintf(stdout, "%s\n", "===========================");
		for (pid_ndx = 0; pid_ndx < num_thrd; pid_ndx++) {
			if (proc_id[pid_ndx] == proc_ndx)
				fprintf(stdout,
					"pid of task = %d priority requested = %d"
					" priority assigned by scheduler = %d\n",
					gen_pid[pid_ndx], exp_prio[pid_ndx],
					act_prio[pid_ndx]);
		}
	}
	exit(0);
}
