/*
 *   Copyright (C) Bull S.A. 1996
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
/*---------------------------------------------------------------------+
|                           shmem_test_07.c                            |
| ==================================================================== |
|                                                                      |
| Description:  Verify shared memory functions with                    |
|                                                                      |
| Algorithm:                                                           |
|                    ## shared memory segments ##                      |
|               *  from 1 up to number_of_writer                       |
|               {                                                      |
|               o  Obtain three shared memory segments per writer      |
|                  one for storing the read count (number of thread    |
|                  reading "scratch" shm) ,                            |
|                  another for storing the checksums of readers ,      |
|                  and the last for the "scratch" shared memory segment|
|                  for storing a series of values .                    |
|               }                                                      |
|                    ## Threads ##                                     |
|               *  from 1 up to number_of_writer                       |
|               {                                                      |
|                  Initializes mutexes ,                               |
|                  Insure the writer gets first access to the segment: |
|                  Threads are synchronized with Condition Varaiables  |
|                                                                      |
|                  thread_hold[number_of_writer]=1;                    |
|                                                                      |
|               }                                                      |
|               *  from 1 up to number_of_writer                       |
|               {                                                      |
|                  Create/Start all num_writers threads (writer)       |
|                  from 1 up to number_of_reader                       |
|                       {                                              |
|                       Create/Start all num_readers threads (reader)  |
|                       }                                              |
|               }                                                      |
|               *  from 1 up to number_of_writer                       |
|               {                                                      |
|                  Wait for the termination of writer thread           |
|                  from 1 up to number_of_reader                       |
|                       {                                              |
|                       Wait for the termination of reader thread      |
|                       }                                              |
|               }                                                      |
|               *  from 1 up to number_of_writer                       |
|               {                                                      |
|                  Get writer checksum                                 |
|                  from 1 up to number_of_reader                       |
|                       {                                              |
|                       Verify that each checksum calculated by readers|
|                       match with the writer checksum                 |
|                       }                                              |
|               }                                                      |
|----------------------------------------------------------------------|
|                               writer ()                              |
|----------------------------------------------------------------------|
|               o  Writer:                                             |
|                     - Fill the "scratch" shared memory segment up    |
|                       with data                                      |
|                     - Compute the segment checksum                   |
|                     - release lock (reader threads may begin) i.e:   |
|                                                                      |
|                       thread_hold[num_w]=0;                          |
|                                                                      |
|----------------------------------------------------------------------|
|                               reader ()                              |
|----------------------------------------------------------------------|
|               o  Reader:                                             |
|                     - Check to see if we need to wait i.e:           |
|                                                                      |
|                       while (thread_hold[num_w]) wait;               |
|                                                                      |
|                     - Evaluate checksum                              |
|                     - Store the resulting checksum                   |
|----------------------------------------------------------------------|
|                                                                      |
|                                                                      |
| System calls: The following system calls are tested:                 |
|               shmget ()                                              |
|               shmat ()                                               |
|               shmctl ()                                              |
|                                                                      |
|               The following system calls are used:                   |
|               malloc ()                                              |
|               pthread_mutex_init()                                   |
|               pthread_cond_init()                                    |
|               pthread_attr_init()                                    |
|               pthread_attr_setdetachstate()                          |
|               pthread_create()                                       |
|               pthread_join()                                         |
|               pthread_mutex_lock()                                   |
|               pthread_cond_broadcast()                               |
|               pthread_mutex_unlock()                                 |
|               pthread_cond_wait()                                    |
|               pthread_mutex_destroy()                                |
|                                                                      |
|                                                                      |
| Usage:                                                               |
|       shmem_test_07 [-c num_readers] [-s shmem_size] [-g num_writers]|
|                                                                      |
| To compile:                                                          |
|       cc_r -g -lpthreads -lc_r -o shmem_test_07 shmem_test_07.c      |
|                                                                      |
| Last update:                                                         |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     011697  JM    Initial version for AIX 4.2G                |
|                                                                      |
+---------------------------------------------------------------------*/
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include <sys/stat.h>
/* Defines
 *
 * DEFAULT_SHMEM_SIZE: default shared memory size, unless specified with
 * -s command line option
 *
 * SHMEM_MODE: shared memory access permissions (permit process to read
 * and write access)
 *
 * USAGE: usage statement
 */
#define MAX_THREAD_NUMBER	500
#define MAX_WRITER_NUMBER	100
#define MAX_READER_NUMBER	400

#define DEFAULT_NUM_READERS	2
#define DEFAULT_NUM_WRITERS	2

#define SHMEM_MODE              (SHM_R | SHM_W)

#define DEFAULT_SHMEM_SIZE	200000
#define MB              	(1024*1024)
#define MAX_SHMEM_NUMBER        11

#define USAGE	"\nUsage: %s [-c num_readers] [-g num_writers] [-s shmem_size]\n\n" \
		"\t-c num_readers    number of thread (readers) to create\n" \
		"\t-g num_writers    number of thread (writers) to create\n" \
		"\t-s buffer_size    size of shared memory segment (bytes)\n" \
		"\t                  (must be less than 256MB!)\n\n"

/*
 * Function prototypes
 *
 * parse_args (): Parse command line arguments
 * reader (): Thread program
 * writer (): "scratch" each and every shared memory segment
 * sys_error (): System error message function
 * error (): Error message function
 * release (): Release the shared memory segments
 */
static void parse_args (int, char **);
static void *reader (void *);
static void *writer (void *);
static void sys_error (const char *, int);
static void error (const char *, int);
static void release ();

/*
 * Global Variables:
 *
 * checksum: Array of checksums computed by reader threads
 * read_count: number of reader threads reading data

 * num_readers: number of reader threads to create
 * num_writers: number of writer threads to create
 * buffer_size: size of "scratch" shared memory segment
 * parent_pid: Process id of the parent
 */
pthread_t * writer_th;
pthread_t * reader_th;

pthread_mutex_t mutex_r[MAX_WRITER_NUMBER];
pthread_mutex_t cond_mutex[MAX_WRITER_NUMBER];
int 		thread_hold[MAX_WRITER_NUMBER];
pthread_cond_t  cond_var[MAX_WRITER_NUMBER];

int	 *read_count[MAX_WRITER_NUMBER];    /* Shared memory segment address */
unsigned long *checksum[MAX_WRITER_NUMBER]; /* Shared memory segment address */
unsigned char *shmptr[MAX_WRITER_NUMBER];   /* Shared memory segment address */
unsigned long cksum[MAX_WRITER_NUMBER];     /* Shared memory segment checksum */

int 	 shmem_size = DEFAULT_SHMEM_SIZE;
pid_t	 parent_pid;			    /* process id of parent */

int      num_readers = DEFAULT_NUM_READERS;
int 	 buffer_size = DEFAULT_SHMEM_SIZE;
int 	 num_writers = DEFAULT_NUM_WRITERS;

int 	 shmid[MAX_THREAD_NUMBER + MAX_WRITER_NUMBER];

/*---------------------------------------------------------------------+
|                               main                                   |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
| Returns:   (0)  Successful completion                                |
|            (-1) Error occurred                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	pthread_attr_t	newattr;

	int	i; 		/* Misc loop index */
	int	j; 		/* Misc loop index */
	int	k; 		/* Misc loop index */
	size_t Size;		/* Size (in bytes) of shared memory segment*/

	unsigned long *ulptr;	/* Misc pointer */
				/* Index into shared memory segment */

	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args (argc, argv);

	printf ("%s: IPC Shared Memory TestSuite program\n", *argv);
        /*
         * Show options in effect.
         */
        printf ("\tNumber of writers    = %d\n", num_writers);
        printf ("\tNumber of readers    = %d\n", num_readers);
        printf ("\tBytes per writer	= %d\n", buffer_size);

/*---------------------------------------------------------------------+
|			shared memory segments                         |
+---------------------------------------------------------------------*/

	for (i=0; i<num_writers; i++) {
        /*
         * Obtain a unique shared memory identifier with shmget ().
	 * Attach the shared memory segment to the process with shmat ().
         */

	j=i*3;
	Size=sizeof (int);
        /*
         * Create a shared memory segment for storing the read count
         * (number of reader threads reading shared data)
         * After creating the shared memory segment, initialize it.
         */

        if ((shmid[j] = shmget (IPC_PRIVATE, Size, SHMEM_MODE)) < 0)
                sys_error ("read_count shmget failed", __LINE__);

        if ((long)(read_count[i] = (int *) shmat (shmid[j], 0, 0)) == -1)
		sys_error ("shmat failed", __LINE__);

        *(read_count[i]) = 0;

        /*
         * Create a shared memory segment for storing the checksums of readers.
         * After creating the shared memory segment, initialize it.
         */

	j++;
	Size=sizeof (unsigned long) * num_readers;

        if ((shmid[j] = shmget (IPC_PRIVATE, Size, SHMEM_MODE)) < 0)
                sys_error ("checksum shmget failed", __LINE__);

        if ((long)(checksum[i] = (unsigned long *) shmat (shmid[j], 0, 0)) == -1)
                sys_error ("shmat failed", __LINE__);

	ulptr=checksum[i];

        for (k=0; k < num_readers; k++)
	{
	*ulptr = 0;
	ulptr++;
	}

        /*
         * Create the "scratch" shared memory segment for storing
         * a series of values .
         */

	Size=buffer_size;
	j++;

        if ((shmid[j] = shmget (IPC_PRIVATE, Size, SHMEM_MODE)) < 0)
                sys_error ("shmptr shmget failed", __LINE__);

        if ((long)(shmptr[i] = shmat (shmid[j], 0, 0)) == -1)
                sys_error ("shmat failed", __LINE__);

        }
/*---------------------------------------------------------------------+
|			Threads                                        |
+---------------------------------------------------------------------*/

        /*
         * Create threads array...
         */
       writer_th = (pthread_t *) malloc ((size_t) (num_writers * sizeof (pthread_t)));
       reader_th = (pthread_t *) malloc ((size_t) (num_writers * num_readers * sizeof (pthread_t)));
	/*
	 * Initializes mutexes and sets their attributes
	 */
        for (i=0; i<num_writers; i++) {

	if (pthread_mutex_init(&mutex_r[i] , NULL) != 0)
		sys_error ("Can't initialize mutex_r", __LINE__);

	if (pthread_mutex_init (&cond_mutex[i], NULL))
		sys_error ("Can't initialize cond_mutex", __LINE__);
	if (pthread_cond_init (&cond_var[i], NULL))
		sys_error ("cond_init(&cond_var) failed", __LINE__);
        /*
         * lock the access to the shared memory data segment --
         * get lock now to insure the writer gets first access to the segment.
         *
         */

	thread_hold[i]=1;

	}

	/*
	 * Creates a thread attributes object and initializes it
	 * with default values.
	*/
        if (pthread_attr_init(&newattr))
                sys_error ("attr_init(&newattr) failed", __LINE__);
	/*
	 * Sets the value of the detachstate attribute of a thread attributes
	 * object :
	 * PTHREAD_CREATE_UNDETACHED	Specifies that the thread will be
	 * created in undetached state.
	*/
#ifdef _LINUX_
	// the DEFAULT state for linux pthread_create is to be "undetatched" or joinable
	/* if (pthread_attr_setdetachstate (&newattr, PTHREAD_CREATE_JOINABLE))
                sys_error ("attr_setdetachstate(&newattr) failed", __LINE__);*/
#else
        if (pthread_attr_setdetachstate (&newattr, PTHREAD_CREATE_UNDETACHED))
                sys_error ("attr_setdetachstate(&newattr) failed", __LINE__);
#endif

        /*
         * Create all num_writers threads .  Each writer thread will fill
	 * the "scratch" shared memory segment (shmptr) up with data and
         * will store the result in cksum array accessible by the main.
         */

        for (i = 0; i < num_writers; i++)
        {
                if (pthread_create (&writer_th[i], &newattr, writer, (void *) (long)i))
                        sys_error ("writer: pthread_create failed", __LINE__);

        /*
         * Create all num_readers threads .  Each reader thread will compute
         * the checksum of the shared memory segment (shmptr) and will store
         * the result in the other shared memory segment (checksum)accessible
         * by the writer.
         */

	k=i*num_readers;
        for (j = k; j < (k + num_readers) ; j++)
        {
                if (pthread_create (&reader_th[j], &newattr, reader, (void *) (long)j))
                        sys_error ("reader: pthread_create failed", __LINE__);
	}
	}

        for (i = 0; i < num_writers; i++)
        {
               if (pthread_join( writer_th[i], NULL)) {
                        printf("writer_th: pthread_join return: %d\n",i);
                        sys_error("pthread_join bad status", __LINE__);
                }

        /*
         * Wait for the reader threads to compute the checksums and complete.
	 */
	k=i*num_readers;
	for (j = k; j < (k + num_readers) ; j++)
        {
               if (pthread_join( reader_th[j], NULL)) {
                        printf("reader_th: pthread_join return: %d\n",j);
                        sys_error("pthread_join bad status", __LINE__);
                }
        }
        }

	/*
	 * After the threads complete, check their exit status to insure
	 * that they ran to completion and then verify the corresponding
	 * checksum.
	 */
        for (i = 0; i < num_writers; i++)
        {
	ulptr=checksum[i];
	for (j=0; j<num_readers; j++) {

		if (cksum[i] != *ulptr)
			error ("checksums do not match", __LINE__);

		}
	}
	printf ("\n\tMain: readers calculated segment successfully\n");

	release();
	printf ("\nsuccessful!\n");

	return (0);
}

/*---------------------------------------------------------------------+
|                               writer ()                              |
| ==================================================================== |
|                                                                      |
| Function:  Fill the "scratch" shared memory segment up with data and |
|            compute the segment checksum.                             |
|            Release "write" lock after completing so that the readers |
|	     are able to start.                                        |
|	                                                               |
| Updates:   cksum[]  - array containing checksums computed by writers.|
|	     data shared memory segment filled up.                     |
|                                                                      |
+---------------------------------------------------------------------*/
void *writer (void *parm)
{
	int num_w = (int) (long)parm;
	unsigned long cksum_w = 0;	/* Shared memory regions checksum */
        unsigned char data = 0; /* Value written into shared memory segment */
        unsigned char *ptr;     /* Misc pointer */

        /*
         * Fill the "scratch" shared memory segment up with data and
         * compute the segments checksum.  Release "write" lock after
         * completing so that the reader threads may begin to read the
         * data.
         */
        data = num_w;

        for (ptr=shmptr[num_w]; ptr < (shmptr[num_w]+buffer_size); ptr++) {
                *ptr = (data++) % (UCHAR_MAX + 1);
                cksum_w += *ptr;
        }
        if (pthread_mutex_lock (&cond_mutex[num_w]))
                sys_error ("mutex_lock(&cond_mutex) failed", __LINE__);
        thread_hold[num_w]=0;
        if (pthread_cond_broadcast (&cond_var[num_w]))
                sys_error ("cond_signal(&cond_var) failed", __LINE__);
        if (pthread_mutex_unlock (&cond_mutex[num_w]))
                sys_error ("mutex_unlock(&cond_mutex) failed", __LINE__);

	cksum[num_w] = cksum_w;
        printf ("\t\twriter (%03d): shared memory checksum %08lx\n", num_w, cksum_w);

	return NULL;
}
/*---------------------------------------------------------------------+
|                               reader ()                              |
| ==================================================================== |
|                                                                      |
| Function:  Waits for read access to the shared memory segment,       |
|            computes the shared memory segments checksum and releases |
|            the read lock.  Then stores the checksum.                 |
|                                                                      |
| Updates:   checksum - shared memory segment containing checksums     |
|                       computed by reader threads                     |
|                                                                      |
+---------------------------------------------------------------------*/
void *reader (void *parm)
{
	int num_p = (int) (long)parm;
	unsigned long cksum_r = 0;	/* Shared memory regions checksum */
	int	i;			/* Misc index */
	int	num_r;			/* Misc index */
	int	num_w;			/* Misc index */
	unsigned char * ptr;	  /* Misc pointer */
	unsigned long *ulptr_r;   /* Misc pointer */

	/*
	 * Wait for a READ_COUNT lock on the shared memory segment, then
	 * compute the checksum and release the READ_COUNT lock.
	 */

	num_r=num_p % num_readers;
	num_w=num_p - num_r;
	num_w=num_w / num_readers;
	ptr=shmptr[num_w];
	ulptr_r=checksum[num_w];

	if (pthread_mutex_lock (&cond_mutex[num_w]))
		sys_error ("Can't take cond lock", __LINE__);
        /*
         * Check to see if we need to wait: if yes, wait for the condition
         * variable (blocking wait).
         */
	while (thread_hold[num_w])
	{
		if (pthread_cond_wait (&cond_var[num_w], &cond_mutex[num_w]))
			sys_error ("cond_wait failed", __LINE__);
	}
	if (pthread_mutex_unlock (&cond_mutex[num_w]))
		sys_error ("Can't release cond lock", __LINE__);

        if (pthread_mutex_lock(&mutex_r[num_w]))
                sys_error ("Can't take read lock", __LINE__);

	(*(read_count [num_w]))++;

        if (pthread_mutex_unlock(&mutex_r[num_w]))
                sys_error ("Can't release read lock", __LINE__);

	for (i=0; i<buffer_size; i++)
		cksum_r += *ptr++;

        if (pthread_mutex_lock(&mutex_r[num_w]))
                sys_error ("Can't take read lock", __LINE__);
	(*(read_count[num_w]))--;
        if (pthread_mutex_unlock(&mutex_r[num_w]))
                sys_error ("Can't release 1 read lock", __LINE__);

	/*
	 * Store the resulting checksum and print out a message
	 */

	*ulptr_r = cksum_r;
	printf ("\t\treader (%03d) of writer (%03d): checksum %08lx\n", num_r, num_w, cksum_r);
	return NULL;
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
|            [-s] size: shared memory segment size                     |
|                                                                      |
|            [-c] num_readers: number of reader threads                |
|                                                                      |
|            [-g] num_writers: number of writer threads                |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args (int argc, char **argv)
{
	int	i;
	int	errflag = 0;
	char	*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	while ((i = getopt(argc, argv, "c:s:g:?")) != EOF) {
		switch (i) {
			case 'c':
				num_readers = atoi (optarg);
				break;
			case 's':
				buffer_size = atoi (optarg);
				break;
                        case 'g':               /* number of group */
                                num_writers = atoi (optarg);
                                break;
			case '?':
				errflag++;
				break;
		}
	}
	if (num_writers >= MAX_WRITER_NUMBER) {
		errflag++;
		fprintf (stderr, "ERROR: num_writers must be less than %d\n",
			MAX_WRITER_NUMBER);
	}
	if (num_readers >= MAX_READER_NUMBER) {
		errflag++;
		fprintf (stderr, "ERROR: num_readers must be less than %d\n",
			MAX_READER_NUMBER);
	}
	i=num_readers*num_writers;
	if (i >= MAX_THREAD_NUMBER) {
		errflag++;
		fprintf (stderr, "ERROR: maximun threads number must be less than %d\n", MAX_THREAD_NUMBER);
	}

	if (errflag) {
		fprintf (stderr, USAGE, program_name);
		exit (2);
	}
}

/*---------------------------------------------------------------------+
|                             release ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Release shared memory regions.                            |
|                                                                      |
+---------------------------------------------------------------------*/
void release ()
{
        int i;
        int j;
        for (i=0; i<num_writers; i++) {
        if (pthread_mutex_destroy(&cond_mutex[i]) != 0)
                sys_error ("Can't destroy cond_mutex", __LINE__);
        if (pthread_mutex_destroy(&mutex_r[i]) != 0)
                sys_error ("Can't destroy mutex_r", __LINE__);
        }

        for (i=0; i<num_writers; i++) {
        /*
         * Release shared memory regions
         */
	j=i*3;
        if (shmctl (shmid[j], IPC_RMID, 0) < 0)
                sys_error ("read_count shmctl failed", __LINE__);
	j++;
        if (shmctl (shmid[j], IPC_RMID, 0) < 0)
                sys_error ("checksum shmctl failed", __LINE__);
	j++;
        if (shmctl (shmid[j], IPC_RMID, 0) < 0)
                sys_error ("shmptr shmctl failed", __LINE__);

        }
}

/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
void sys_error (const char *msg, int line)
{
	char syserr_msg [256];

	sprintf (syserr_msg, "%s: %s\n", msg, strerror (errno));
	error (syserr_msg, line);
}

/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and exits...                           |
|                                                                      |
+---------------------------------------------------------------------*/
void error (const char *msg, int line)
{
	fprintf (stderr, "ERROR [line: %d] %s\n", line, msg);
	if (line >= 260)
	release ();
	exit (-1);
}