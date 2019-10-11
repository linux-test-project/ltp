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
/* History:     Nov - 21 - 2001 Created - Manoj Iyer, IBM Austin TX.          */
/*                               email:manjo@austin.ibm.com                   */
/*                                                                            */
/*		Nov - 26 - 2001 Modified - Manoj Iyer, IBM Austin Tx.         */
/*				- Added function rm_shared_mem.               */
/*                                                                            */
/*		Dec - 03 - 2001 Modified - Manoj Iyer, IBM Austin Tx.         */
/*				- Added code to spawn threads.		      */
/*				- Removed dead code.		              */
/*				- Checked in the initial version to CVS       */
/*								              */
/*		Feb - 27 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- removed compiler warnings.                  */
/*				- removed compiler errors.                    */
/*                                                                            */
/* File:	shm_test.c				                      */
/*									      */
/* Description:	This program is designed to stress the Memory management sub -*/
/*		system of Linux. This program will spawn multiple pairs of    */
/*		reader and writer threads. One thread will create the shared  */
/*		segment of random size and write to this memory, the other    */
/*		pair will read from this memory.		              */
/*									      */
/******************************************************************************/
#include <pthread.h>		/* required by pthread functions                      */
#include <stdio.h>		/* required by fprintf()                              */
#include <stdlib.h>		/* required by exit(), atoi()                         */
#include <string.h>		/* required by strncpy()                              */
#include <unistd.h>		/* required by getopt(), mmap()                       */
#include <sys/types.h>		/* required by open(), shmat(), shmdt()               */
#include <sys/stat.h>		/* required by open()                                 */
#include <sys/ipc.h>		/* required by shmat() shmdt(), shmctl()              */
#include <sys/shm.h>		/* required by shmat() shmdt(), shmctl()              */
#include <sys/mman.h>		/* required by mmap()                                 */
#include <fcntl.h>		/* required by open()                                 */
#include <stdint.h>		/* required by uintptr_t                              */

void noprintf(char *string, ...)
{
}

#ifdef DEBUG
#define dprt	printf
#else
#define dprt	noprintf
#endif

#define PTHREAD_EXIT(val)    do {\
			exit_val = val; \
                        dprt("pid[%d]: exiting with %d\n", getpid(),exit_val); \
			pthread_exit((void *)(uintptr_t)exit_val); \
				} while (0)

#define OPT_MISSING(prog, opt)   do{\
			       fprintf(stderr, "%s: option -%c ", prog, opt); \
                               fprintf(stderr, "requires an argument\n"); \
                               usage(prog); \
                                   } while (0)

#define MAXT	30		/* default number of threads to create.               */
#define MAXR	1000		/* default number of repatetions to execute           */

struct child_args
{
	pthread_t threadid;
	int num_reps;
	int shmkey;
	int map_size;
	int is_reader;
};


/******************************************************************************/
/*								 	      */
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
		"Usage: %s -d NUMDIR -f NUMFILES -h -t NUMTHRD\n"
		"\t -h Help!\n"
		"\t -l Number of repatetions to execute:       Default: 1000\n"
		"\t -t Number of threads to generate:          Default: 30\n",
		progname);
	exit(-1);
}

/******************************************************************************/
/*								 	      */
/* Function:	rm_shared_mem						      */
/*									      */
/* Description:	This function removes the shared segments that were created   */
/*		This function is called when shmat fails or logical end of    */
/*		the while loop is reached in shmat_rd_wr function.         */
/*									      */
/* Input:	shm_id   - id of the shared memory segment to be removed      */
/*		shm_addr - address of the shared memory segment to be removed */
/*		cmd      - remove id only or remove id and detach??           */
/*			   0 - remove id dont detach segment.                 */
/*			   1 - remove id and detach segment.                  */
/*									      */
/* Output:	NONE.                                                         */
/*									      */
/* Return:	exits with -1 on error, 0 on success                          */
/*									      */
/******************************************************************************/
static int rm_shared_mem(key_t shm_id,	/* id of shared memory segment to be removed  */
			 char *shm_addr,	/* address of shared mem seg to be removed    */
			 int cmd)
{				/* remove id only or remove id and detach seg */
	struct shmid *shmbuf = NULL;	/* info about the segment pointed by shmkey   */

	dprt("pid[%d]: rm_shared_mem(): shm_id = %d shm_addr = %#x cmd = %d\n",
	     getpid(), shm_id, shm_addr, cmd);
	if (shmctl(shm_id, IPC_RMID, (struct shmid_ds *)shmbuf) == -1) {
		dprt("pid[%d]: rm_shared_mem(): shmctl unable to remove shm_id[%d]\n", getpid(), shm_id);
		perror("rm_shared_mem(): shmctl()");
		return -1;
	}

	if (cmd) {
		if (shmdt((void *)shm_addr) == -1) {
			dprt("pid[%d]:rm_shared_mem(): shmdt unable to detach addr = %#x\n", getpid(), shm_addr);
			perror("rm_shared_mem(): shmdt()");
			return -1;
		}
	}
	return 0;
}

/******************************************************************************/
/*								 	      */
/* Function:	shmat_rd_wr						      */
/*									      */
/* Description:	This function repeatedly attaches and detaches the memory     */
/*		The size of the file is a multiple of page size.              */
/*		The function acts as either reader or writer thread depending */
/*		on arg[3]. The reader and writer thread use the same key so   */
/*		they get access to the same shared memory segment.            */
/*									      */
/* Input:	The argument pointer contains the following.                  */
/*		arg[0] - number of repatetions of the above operation         */
/*		arg[1] - shared memory key.				      */
/*		arg[2] - size of the memory that is to be attached.           */
/*		arg[3] - reader or writer.                                    */
/*									      */
/* Return:	exits with -1 on error, 0 on success                          */
/*									      */
/******************************************************************************/
static void *shmat_rd_wr(void *vargs)
{				/* arguments to the thread function             */
	int shmndx = 0;		/* index to the number of attach and detach   */
	int index = 0;		/* index to the number of blocks touched      */
	key_t shm_id = 0;	/* shared memory id                           */
	struct child_args *args = vargs;
	volatile int exit_val = 0;	/* exit value of the pthread                  */
	char *read_from_mem;	/* ptr to touch each (4096) block in memory   */
	char *write_to_mem;	/* ptr to touch each (4096) block in memory   */
	char *shmat_addr;	/* address of the attached memory             */
	char buff;		/* temporary buffer                           */

	while (shmndx++ < args->num_reps) {
		dprt("pid[%d]: shmat_rd_wr(): locargs[1] = %#x\n",
		     getpid(), args->shmkey);

		/* get shared memory id */
		if ((shm_id =
		     shmget(args->shmkey, args->map_size, IPC_CREAT | 0666))
		    == -1) {
			dprt("pid[%d]: shmat_rd_wr(): shmget failed\n",
			     getpid());
			perror("do_shmat_shmadt(): shmget()");
			PTHREAD_EXIT(-1);
		}

		fprintf(stdout, "pid[%d]: shmat_rd_wr(): shmget():"
			"success got segment id %d\n", getpid(), shm_id);

		/* get shared memory segment */
		if ((shmat_addr = shmat(shm_id, NULL, 0)) == (void *)-1) {
			rm_shared_mem(shm_id, shmat_addr, 0);
			fprintf(stderr,
				"pid[%d]: do_shmat_shmadt(): shmat_addr = %#lx\n",
				getpid(), (long)shmat_addr);
			perror("do_shmat_shmadt(): shmat()");
			PTHREAD_EXIT(-1);
		}
		dprt("pid[%d]: do_shmat_shmadt(): content of memory shmat_addr = %s\n", getpid(), shmat_addr);

		fprintf(stdout,
			"pid[%d]: do_shmat_shmadt(): got shmat address = %#lx\n",
			getpid(), (long)shmat_addr);

		if (args->is_reader) {
			/* write character 'Y' to that memory area */
			index = 0;
			write_to_mem = shmat_addr;
			while (index < args->map_size) {
				dprt("pid[%d]: do_shmat_shmatd(): write_to_mem = %#x\n", getpid(), write_to_mem);
				*write_to_mem = 'Y';
				index++;
				write_to_mem++;
				sched_yield();
			}
		} else {
			/* read from the memory area */
			index = 0;
			read_from_mem = shmat_addr;
			while (index < args->map_size) {
				buff = *read_from_mem;
				index++;
				read_from_mem++;
				sched_yield();
			}
		}

		sched_yield();

		/* remove the shared memory */
		if (rm_shared_mem(shm_id, shmat_addr, 1) == -1) {
			fprintf(stderr,
				"pid[%d]: do_shmat_shmatd(): rm_shared_mem(): faild to rm id\n",
				getpid());
			PTHREAD_EXIT(-1);
		}
	}

	PTHREAD_EXIT(0);
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
	int num_reps = MAXR;	/* number of repatitions the test is run      */
	int i;
	void *th_status;	/* exit status of LWP's                       */
	int map_size;		/* size of the file mapped.                   */
	int shmkey = 1969;	/* key used to generate shmid by shmget()     */
	struct child_args chld_args[30];	/* arguments to the thread function */
	char *map_address = NULL;
	/* address in memory of the mapped file       */
	extern int optopt;	/* options to the program                     */

	while ((c = getopt(argc, argv, "hl:t:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 'l':	/* how many repetitions of the test to exec   */
			if ((num_reps = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_reps < 0) {
				fprintf(stdout,
					"WARNING: bad argument. Using default\n");
				num_reps = MAXR;
			}
			break;
		case 't':
			if ((num_thrd = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_thrd < 0 || num_thrd > MAXT) {
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

	for (i = 0; i < num_thrd; i += 2) {
		srand(time(NULL) % 100);
		map_size = (1 + (int)(1000.0 * rand() / (RAND_MAX + 1.0))) * 4096;

		dprt("main(): thrd_ndx = %d map_address = %#x map_size = %d\n",
		     i, map_address, map_size);

		chld_args[i].num_reps = num_reps;
		chld_args[i].map_size = map_size;
		chld_args[i].shmkey = shmkey++;
		chld_args[i].is_reader = 0;
		if (pthread_create
		    (&chld_args[i].threadid, NULL, shmat_rd_wr, &chld_args[i])) {
			perror("shmat_rd_wr(): pthread_create()");
			exit(-1);
		}

		chld_args[i + 1] = chld_args[i];
		chld_args[i + 1].is_reader = 1;
		if (pthread_create
		    (&chld_args[i + 1].threadid, NULL, shmat_rd_wr, &chld_args[i + 1])) {
			perror("shmat_rd_wr(): pthread_create()");
			exit(-1);
		}
	}

	sync();

	for (i = 0; i < num_thrd; i++) {
		if (pthread_join(chld_args[i].threadid, &th_status) != 0) {
			perror("shmat_rd_wr(): pthread_join()");
			exit(-1);
		} else {
			dprt("WE ARE HERE %d\n", __LINE__);
			if (th_status == (void *)-1) {
				fprintf(stderr,
					"thread [%ld] - process exited with errors\n",
					(long)chld_args[i].threadid);
				exit(-1);
			}
		}
	}
	exit(0);
}
