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
 *  FILE        : stress_cd.c
 *  DESCRIPTION : creates multiple read threads on the cdrom device.
 *  HISTORY:
 *    06/20/2001 Robbie Williamson (robbiew@us.ibm.com)
 *      -Ported
 *    11/08/2001 Manoj Iyer (manjo@austin.ibm.com)
 *      - Modified.
 *	- removed compiler warnings.
 *	- Added #include <sys/types.h>, #include <unistd.h> and
 *	  #include <string.h>
 *	- print unsigned long correctly in printf() use "lx" instead of "x"
 *	- added missing parameter in usage message.
 *
+--------------------------------------------------------------------+
|                                                                    |
| Usage:        cdtest [-n n] [-f file] [-m xx] [-d]                 |
|                                                                    |
|               where:                                               |
|                 -n n     Number of threads to create               |
|                 -f file  File or device to read from               |
|                 -m xx    Number of MB to read from file            |
|                 -b xx    Number of bytes to read from file         |
|                 -d       Enable debugging messages                 |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

/* Defines
 *
 * DEFAULT_NUM_THREADS: Default number of threads to create,
 * user can specifiy with [-n] command line option.
 *
 * USAGE: usage statement
 */
#define DEFAULT_NUM_THREADS 	10
#define DEFAULT_NUM_BYTES   	1024*1024*100 /* 100Mb */
#define DEFAULT_FILE        	"/dev/cdrom"

/*
 * Function prototypes
 *
 * sys_error (): System error message function
 * error (): Error message function
 * parse_args (): Parses command line arguments
 */
static void sys_error (const char *, int);
static void error (const char *, int);
static void parse_args (int, char **);
void *thread (int *);
int read_data (int, unsigned long);

/*
 * Global Variables
 */
int num_threads = DEFAULT_NUM_THREADS;
int num_bytes = DEFAULT_NUM_BYTES;
char *file = DEFAULT_FILE;
unsigned long checksum = 0;
int debug = 0;

/*-------------------------------------------------------------------+
|                               main ()                              |
| ================================================================== |
|                                                                    |
| Function:  Main program  (see prolog for more details)             |
|                                                                    |
+-------------------------------------------------------------------*/
int main (int argc, char **argv)
{
	pthread_attr_t  attr;
	pthread_t *array;
	int *arg;
	int rc = 0, i;

	/* Parse command line arguments and print out program header */
	parse_args (argc, argv);

	/* Read data from CDROM & compute checksum */
	read_data (0, checksum);
	if (debug)
	   printf("Thread [main] checksum: %-#12lx \n", checksum);

	if (pthread_attr_init (&attr))
	   sys_error ("pthread_attr_init failed", __LINE__);
	if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE))
	   sys_error ("pthread_attr_setdetachstate failed", __LINE__);

	/* Create num_thread threads... */
	printf("\tThread [main] Creating %d threads\n", num_threads);

	array = (pthread_t *) malloc (sizeof (pthread_t) * num_threads);
	arg = (int *) malloc (sizeof (int) * num_threads);
	assert (array);
	assert (arg);
	for (i=0; i < num_threads; i++)
        {
	    if (debug)
	       printf("\tThread [main]: creating thread %d\n", i + 1);
	    arg[i] = i + 1;
	    if (pthread_create ((pthread_t *) &array [i], &attr, (void *)thread, (void *) &arg[i]))
	    {
	       if (errno == EAGAIN)
		  fprintf(stderr, "\tThread [main]: unable to create thread %d\n", i);
	       else
		  sys_error ("pthread_create failed", __LINE__);
	    }
	    if (debug)
	       printf("\tThread [main]: created thread %d\n", i+1);
	}
	if (pthread_attr_destroy (&attr))
	   sys_error ("pthread_attr_destroy failed", __LINE__);

	for (i=0; i<num_threads; i++)
        {
	    int exit_value;
	    printf("\tThread [main]: waiting for thread: %d\n", i+1);
	    /*if (pthread_join ((pthread_t*) array [i], (void **) &exit_value))*/
	    if (pthread_join ((pthread_t) array [i], (void **) &exit_value))
	       sys_error ("pthread_join failed", __LINE__);

	    if (debug)
	       printf("\tThread [%d]: return %d\n", i + 1, exit_value);
	    rc += exit_value;
	}
	free(array);
	free(arg);

	/* One or more of the threads did not complete sucessfully! */
	if (rc != 0)
        {
	   printf("test failed!\n");
	   exit (-1);
	}

	/* Program completed successfully... */
	printf("\tThread [main] All threads completed successfully...\n");
	exit (0);
}

/*-------------------------------------------------------------------+
|                               thread ()                            |
| ================================================================== |
|                                                                    |
| Function:  ...                                                     |
|                                                                    |
+-------------------------------------------------------------------*/
void *thread (int *parm)
{
	int num = *parm;
	unsigned long cksum = 0;

	if (debug)
           printf("\tThread [%d]: begin\n", num);

	read_data (num, cksum);
	if (checksum != cksum)
        {
	   fprintf(stderr, "\tThread [%d]: checksum mismatch!\n", num);
	   pthread_exit ((void *) -1);
	}

	if (debug)
           printf("\tThread [%d]: done\n", num);

	pthread_exit ((void *) 0);
	return (NULL);
}

/*-------------------------------------------------------------------+
|                           read_data ()                             |
| ================================================================== |
|                                                                    |
| Function:  Reads data from the CDROM                               |
|                                                                    |
+-------------------------------------------------------------------*/
int read_data (int num, unsigned long cksum)
{
	int fd;
	const int bufSize = 1024;
	char *buffer;
	int bytes_read = 0;
	int n;
	char *p;

	if (debug)
	   printf("\tThread [%d]: read_data()\n", num);

	if ((fd = open (file, O_RDONLY, NULL)) < 0)
	   sys_error ("open failed /dev/cdrom", __LINE__);

        buffer = (char *) malloc (sizeof(char) * bufSize);
	assert (buffer);

	lseek(fd,1024*36,SEEK_SET);
	while (bytes_read < num_bytes)
        {
	   if ((n = read (fd, buffer, bufSize)) < 0)
	      sys_error ("read failed", __LINE__);
	   bytes_read += n;

	   for (p=buffer; p < buffer+n; p++)
	      cksum += *p;

	   if (debug)
              printf("\tThread [%d] bytes read: %5d checksum: %-#12lx\n",
		     num, bytes_read, cksum);
	}
	free(buffer);

	if (debug)
	   printf("\tThread [%d] bytes read: %5d checksum: %-#12lx\n",
		  num, bytes_read, cksum);

	if (close (fd) < 0)
	   sys_error ("close failed", __LINE__);

	if (debug)
	   printf("\tThread [%d]: done\n", num);

	return (0);
}

/*-------------------------------------------------------------------+
|                             parse_args ()                          |
| ================================================================== |
|                                                                    |
| Function:  Parse the command line arguments & initialize global    |
|            variables.                                              |
|                                                                    |
| Updates:   (command line options)                                  |
|                                                                    |
|            [-n] num   number of threads to create                  |
|                                                                    |
|            [-d]       enable debugging messages                    |
|                                                                    |
+-------------------------------------------------------------------*/
static void parse_args (int argc, char **argv)
{
	int		i;
	int		errflag = 0;
	char		*program_name = *argv;
	extern char 	*optarg;	/* Command line option */

	while ((i = getopt(argc, argv, "df:n:b:m:?")) != EOF)
        {
	   switch (i)
 	   {
		case 'd':	/* debug option */
				debug++;
				break;
		case 'f':	/* file to read from */
				file = optarg;
				break;
		case 'm':	/* num MB to read */
				num_bytes = atoi (optarg) * 1024 * 1024 ;
				break;
		case 'b':	/* num bytes to read */
				num_bytes = atoi (optarg);
				break;
		case 'n':	/* number of threads */
				num_threads = atoi (optarg);
				break;
		case '?':	/* help */
				errflag++;
				break;
	   }
	}
	if (num_bytes < 0)
        {
	   errflag++;
	   fprintf(stderr, "ERROR: num_bytes must be greater than 0");
	}

	if (errflag)
        {
	   fprintf(stderr, "\nUsage: %s"
		   " [-n xx] [-m|b xx] [-d]\n\n"
		   "\t-n xx    Number of threads to create (up to %d)\n"
		   "\t-f file  File to read from\n"
		   "\t-m xx    Number of MB to read\n"
		   "\t-b xx    Number of bytes to read\n"
		   "\t-d       Debug option\n", program_name,
			       DEFAULT_NUM_THREADS);
	   exit (2);
	}
}

/*-------------------------------------------------------------------+
|                             sys_error ()                           |
| ================================================================== |
|                                                                    |
| Function:  Creates system error message and calls error ()         |
|                                                                    |
+-------------------------------------------------------------------*/
static void sys_error (const char *msg, int line)
{
	char syserr_msg [256];

	sprintf (syserr_msg, "%s: %s\n", msg, strerror(errno));
	error (syserr_msg, line);
}

/*-------------------------------------------------------------------+
|                               error ()                             |
| ================================================================== |
|                                                                    |
| Function:  Prints out message and exits...                         |
|                                                                    |
+-------------------------------------------------------------------*/
static void error (const char *msg, int line)
{
	fprintf(stderr, "ERROR [line: %s] \n", msg);
	exit (-1);
}