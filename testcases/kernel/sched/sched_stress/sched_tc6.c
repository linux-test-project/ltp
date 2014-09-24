/*
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
/*
 * FUNCTIONS: Scheduler Test Suite
 */

/*---------------------------------------------------------------------+
|                               sched_tc6                              |
| ==================================================================== |
|                                                                      |
| Description:  Creates short-term disk I/O bound process              |
|                                                                      |
|		Creates a situation where a real time process is       |
|		waiting on a file lock owned by a user process.  The   |
|		scheduler should elevate the user process priority to  |
|		the same level as the real time process to avoid       |
|		deadlock situations and to decrease wait time          |
|		required of the real time process.                     |
|                                                                      |
| Algorithm:    o  Set process priority                                |
|               o  Continuously multiply matrices together until       |
|                  interrupted.                                        |
|                                                                      |
| To compile:   cc -o sched_tc6 sched_tc6.c -L. -lpsc                  |
|                                                                      |
| Usage:        sched_tc6 [-t priority_type] [-p priority]             |
|                         [-l log] [-f] [-v] [-d]                      |
|                                                                      |
| Last update:   Ver. 1.3, 4/10/94 23:05:03                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     010402  Manoj Iyer Ported to Linux			       |
|                                                                      |
+---------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include "sched.h"

/*
 * Defines:
 *
 * USAGE: usage statement
 *
 * DEFAULT_PRIORITY_TYPE: default priority
 *
 * BLOCK_SIZE: block size (in bytes) for raw I/O
 *
 * TIMES: number of times to read raw I/O device (~25MB)
 *
 */
#define DEFAULT_PRIORITY_TYPE	"variable"
#define DEFAULT_LOGFILE		"sched_tc6.log"
#define BLOCK_SIZE		512
#define TIMES			10
#define MAX_TRIES		20
#define NAPTIME			1
#define REAL_TIME		"1"
#define NO_FORK			"0"
#define USAGE "Usage:  %s  [-l log] [-t type] [-p priority] [-f] [-v] [-d]\n" \
              "        -l log      log file                             \n" \
              "        -t type     priority type 'variable' or 'fixed'  \n" \
              "        -p priority priority value                       \n" \
              "        -f          fork child                           \n" \
              "        -v          verbose                              \n" \
              "        -d          enable debugging messages            \n"

/*
 * Function prototypes:
 *
 * process_file: reads data file
 *
 * parse_args: parse command line arguments
 */
void parse_args(int, char **);
int fork_realtime(char **);
int read_file(int, char *);
int lock_file(int, short, char *);
int unlock_file(int, char *);
int lock_error(int, char *);

/*
 * Global variables:
 *
 * verbose: enable normal messages
 *
 * debug: enable debugging messages
 *
 * priority: process type (fixed priority, variable priority)
 */
int verbose = 0;
int debug = 0;
int fork_flag = 0;
int priority = DEFAULT_PRIORITY;
char *logfile = DEFAULT_LOGFILE;
char *priority_type = DEFAULT_PRIORITY_TYPE;
struct flock flock_struct;
struct flock *flock_ptr = &flock_struct;

int open_file(char *, int);

/*---------------------------------------------------------------------+
|                                 main                                 |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	char *filename = NULL;
	FILE *statfile;
	pid_t pid = 0;
	int fd;
	int rc;
	clock_t start_time;	/* start & stop times */
	clock_t stop_time;
	float elapsed_time;
#ifdef __linux__
	time_t timer_info;
#else
	struct tms timer_info;	/* time accounting info */
#endif

	if ((filename = getenv("KERNEL")) == NULL) {
		errno = ENODATA;
		sys_error("environment variable KERNEL not set", __FILE__,
			  __LINE__);
	}

	/* Process command line arguments...  */
	parse_args(argc, argv);
	if (verbose)
		printf("%s: Scheduler TestSuite program\n\n", *argv);
	if (debug) {
		printf("\tpriority type:  %s\n", priority_type);
		printf("\tpriority:       %d\n", priority);
		printf("\tlogfile:        %s\n", logfile);
	}

	/* Adjust the priority of this process if the real time flag is set */
	if (!strcmp(priority_type, "fixed")) {
#ifndef __linux__
		if (setpri(0, DEFAULT_PRIORITY) < 0)
			sys_error("setpri failed", __FILE__, __LINE__);
#else
		if (setpriority(PRIO_PROCESS, 0, 0) < 0)
			sys_error("setpri failed", __FILE__, __LINE__);
#endif
	} else {
		if (nice((priority - 50) - (nice(0) + 20)) < 0 && errno != 0)
			sys_error("nice failed", __FILE__, __LINE__);
	}

	/* Read from raw I/O device and record elapsed time...  */
	start_time = time(&timer_info);

	/* Open and lock file file...  */
	fd = open_file(filename, O_RDWR);
	if (!lock_file(fd, F_WRLCK, filename))	/* set exclusive lock */
		error("lock_file failed", __FILE__, __LINE__);

	/* If fork flag set, fork a real process */
	if (fork_flag)
		pid = fork_realtime(argv);

	/* Read file */
	if (debug) {
		printf("\tprocess id %d successfully locked %s\n",
		       getpid(), filename);
		printf("\tprocess id %d starting to read %s\n",
		       getpid(), filename);
	}

	if (!read_file(fd, filename))
		error("read_file failed", __FILE__, __LINE__);

	/* Stop the timer and calculate the elapsed time */
	stop_time = time(&timer_info);
	elapsed_time = (float)(stop_time - start_time) / 100.0;

	/* Write the elapsed time to the temporary file...  */
	if ((statfile = fopen(logfile, "w")) == NULL)
		sys_error("fopen failed", __FILE__, __LINE__);

	fprintf(statfile, "%f\n", elapsed_time);
	if (debug)
		printf("\n\telapsed time: %f\n", elapsed_time);

	if (fclose(statfile) < 0)
		sys_error("fclose failed", __FILE__, __LINE__);

	/* Unlock file at latest possible time to prevent real time child from
	 * writing throughput results before user process parent */
	unlock_file(fd, filename);
	close(fd);

	if (debug)
		printf("\tprocess id %d completed read and unlocked file\n",
		       getpid());

	/* The parent waits for child process to complete before exiting
	 * so the driver will not read the throughput results file before
	 * child writes to the file */
	if (pid != 0) {		/* if parent process ... *//* wait for child process */
		if (debug)
			printf
			    ("parent waiting on child process %d to complete\n",
			     pid);

		while ((rc = wait(NULL)) != pid)
			if (rc == -1)
				sys_error("wait failed", __FILE__, __LINE__);
/*
DARA: which one to use
1st -- hangs
2nd -- ERROR message

	    while (wait((void *) 0) != pid) ;
	    while ((rc=wait ((void *) 0)) != pid)
	       if (rc == -1)
	          sys_error ("wait failed", __FILE__, __LINE__);
*/
	}

	/* Exit with success! */
	if (verbose)
		printf("\nsuccessful!\n");
	return (0);
}

/*---------------------------------------------------------------------+
|                             open_file ()                             |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int open_file(char *file, int open_mode)
{
	int file_desc;

	if ((file_desc = open(file, open_mode)) < 0)
		sys_error("open failed", __FILE__, __LINE__);
	return (file_desc);
}

/*---------------------------------------------------------------------+
|                           fork_realtime ()                           |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int fork_realtime(char **args)
{
	int pid;
	char *results_file = args[2];
	char *priority = args[3];

	/* fork process then determine if process is parent or child */
	pid = fork();
	switch (pid) {
		/* fork failed  */
	case -1:
		sys_error("fork failed", __FILE__, __LINE__);

		/* child process */
	case 0:
		if (execl(*args, *args, REAL_TIME, results_file, priority,
			  NO_FORK, NULL) < 0)
			sys_error("execl failed", __FILE__, __LINE__);

		/* parent process */
	default:
#ifdef DEBUG
		printf("\tparent process id = %d\n", getpid());
		printf("\tchild process id = %d\n\n", pid);
#endif

		break;
	}
	return (pid);
}

/*---------------------------------------------------------------------+
|                             read_file ()                             |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int read_file(int fd, char *filename)
{
	int bytes_read;
	int loop_count;
	long total_bytes;
	off_t lseek();
	off_t file_offset = 0;
	int whence = 0;

	char buf[BLOCK_SIZE];

	/* read file for "TIMES" number of times */
	total_bytes = 0;
	if (debug)
		printf("\n");
	for (loop_count = 1; loop_count <= TIMES; loop_count++) {
		while ((bytes_read = read(fd, buf, BLOCK_SIZE)) > 0) {
			if (bytes_read == -1) {
				sys_error("read failed", __FILE__, __LINE__);
			} else
				total_bytes = total_bytes + bytes_read;
		}
		if (lseek(fd, file_offset, whence) < 0)
			sys_error("lseek failed", __FILE__, __LINE__);

		if (debug) {
			printf("\r\ttotal bytes read = %ld", total_bytes);
			fflush(stdout);
		}
		total_bytes = 0;
	}
	if (debug)
		printf("\n");
	return 1;
}

/*---------------------------------------------------------------------+
|                             lock_file ()                             |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int lock_file(int fd, short lock_type, char *file)
{
	int lock_attempt = 1;
	int lock_mode;

#ifdef DEBUG
	lock_mode = F_SETLK;	/* return if lock fails        */
#else
	lock_mode = F_SETLKW;	/* set lock and use system wait */
#endif

	/* file segment locking set data type flock - information
	 * passed to system by user --
	 *   l_whence:  starting point of relative offset of file
	 *   l_start:   defines relative offset in bytes from l_whence
	 *   l_len:     number of consecutive bytes to be locked
	 */
	flock_ptr->l_whence = 0;
	flock_ptr->l_start = 0;
	flock_ptr->l_len = 0;
	flock_ptr->l_type = lock_type;

	while (fcntl(fd, lock_mode, flock_ptr) == -1) {
		if (lock_error(fd, file)) {
			sleep(NAPTIME);
			if (++lock_attempt > MAX_TRIES) {
				printf
				    ("ERROR: max lock attempts of %d reached\n",
				     MAX_TRIES);
				return (0);
			}
		} else
			return (0);
	}
	return (1);
}

/*---------------------------------------------------------------------+
|                            unlock_file ()                            |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int unlock_file(int fd, char *file)
{
	flock_ptr->l_type = F_UNLCK;

	if (fcntl(fd, F_SETLK, flock_ptr) < 0)
		sys_error("fcntl failed", __FILE__, __LINE__);

	return 1;
}

/*---------------------------------------------------------------------+
|                                 main                                 |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int lock_error(int fd, char *file)
{
	int ret = 1;

	printf("ERROR:  lock failed: %s\n", file);
	switch (errno) {
	case EACCES:		/* access not allowed */
		fcntl(fd, F_GETLK, flock_ptr);
#ifndef __linux__
		printf("ERROR: lock exists - nid: %lX pid: %ld\n",
		       flock_ptr->l_sysid, flock_ptr->l_pid);
#else
		printf("ERROR: lock exists - nid:\n");
#endif
		break;

		/*
		 * This was a DS error code, and DS does not exist V3.1
		 */
#ifndef __linux__
	case EDIST:		/* DS file server blocking requests */
		printf("ERROR: errno == EDIST\n");
		printf("The server has blocked new inbound requests\n");
		printf("or outbound requests are currently blocked.\n");
		break;
#endif

	case EAGAIN:		/* server too busy */
		printf("ERROR:  Server too busy to accept the request\n");
		break;

	case EDEADLK:		/* only when F_SETLKW cmd is used */
		printf("ERROR:  Putting the calling process to sleep\n");
		printf("would cause a dead lock\n");
		ret = 0;
		break;

	case ENOLCK:		/* out of locks */
		printf("ERROR: No more file locks available\n");
		ret = 0;
		break;

	case ENOMEM:		/* out of memory */
		printf("ERROR: The server or client does not have enough\n");
		printf("memory to service the request.\n");
		ret = 0;
		break;

	default:		/* miscellaneous errors */
		printf("ERROR: Unknown lock error\n");
		perror("reason");
		ret = 0;
		break;
	}

	printf("errno = %d\n", errno);	/* log the errno value */
	sleep(10);

	return (ret);
}

/*---------------------------------------------------------------------+
|                             parse_args ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Parse the command line arguments & initialize global      |
|            variables.                                                |
|                                                                      |
| Updates:   (command line options)                                    |
|                                                                      |
|            [-t] type:     priority type "fixed" or "variable"        |
|            [-p] priority: priority value                             |
|            [-l] logfile:  log file name                              |
|            [-v]           verbose                                    |
|            [-d]           enable debugging messages                  |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args(int argc, char **argv)
{
	int opt;
	int lflg = 0, pflg = 0, tflg = 0;
	int errflag = 0;
	char *program_name = *argv;
	extern char *optarg;	/* Command line option */

	/*
	 * Parse command line options.
	 */
	while ((opt = getopt(argc, argv, "fl:t:p:vd")) != EOF) {
		switch (opt) {
		case 'f':	/* fork flag */
			fork_flag++;
			break;
		case 'l':	/* log file */
			lflg++;
			logfile = optarg;
			break;
		case 't':	/* process type */
			tflg++;
			priority_type = optarg;
			break;
		case 'p':	/* process priority */
			pflg++;
			priority = atoi(optarg);
			break;
		case 'v':	/* verbose */
			verbose++;
			break;
		case 'd':	/* enable debugging messages */
			verbose++;
			debug++;
			break;
		default:
			errflag++;
			break;
		}
	}

	/*
	 * Check percentage and process slots...
	 */
	if (tflg) {
		if (strcmp(priority_type, "fixed") &&
		    strcmp(priority_type, "variable")) {
			errflag++;
			fprintf(stderr, "Error: priority type must be: "
				"\'fixed\' or \'variable\'\n");
		}
	}
	if (pflg) {
		if (priority < 50 || priority > 100) {
			errflag++;
			fprintf(stderr, "Error: priority range [50..100]\n");
		}
	}
	if (errflag) {
		fprintf(stderr, USAGE, program_name);
		exit(2);
	}
}
