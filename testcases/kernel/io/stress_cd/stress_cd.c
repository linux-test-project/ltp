/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *    06/20/2001 Robbie Williamson (robbiew@us.ibm.com)
 *    11/08/2001 Manoj Iyer (manjo@austin.ibm.com)
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
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Usage:	stress_cd [-n n] [-f file] [-m xx] [-d]
 *		where:
 *		  -n n     Number of threads to create
 *		  -f file  File or device to read from
 *		  -m xx    Number of MB to read from file
 *		  -b xx    Number of bytes to read from file
 *		  -d       Enable debugging messages
 */

#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_NUM_THREADS	10
#define DEFAULT_NUM_BYTES	(1024*1024*100)	/* 100Mb */
#define DEFAULT_FILE		"/dev/cdrom"

static void sys_error(const char *, int);
static void parse_args(int, char **);
static void *thread(int *);
static int read_data(int, unsigned long *);

static int num_threads = DEFAULT_NUM_THREADS;
static int num_bytes = DEFAULT_NUM_BYTES;
static char *file = DEFAULT_FILE;
static unsigned long checksum;
static int debug;

int main(int argc, char **argv)
{
	pthread_attr_t attr;
	int rc = 0, i;

	/* Parse command line arguments and print out program header */
	parse_args(argc, argv);

	/* Read data from CDROM & compute checksum */
	read_data(0, &checksum);
	if (debug)
		printf("Thread [main] checksum: %-#12lx\n", checksum);

	if (pthread_attr_init(&attr))
		sys_error("pthread_attr_init failed", __LINE__);
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE))
		sys_error("pthread_attr_setdetachstate failed", __LINE__);

	printf("\tThread [main] Creating %d threads\n", num_threads);

	pthread_t array[num_threads];
	int arg[num_threads];

	for (i = 0; i < num_threads; i++) {
		if (debug)
			printf("\tThread [main]: creating thread %d\n", i + 1);
		arg[i] = i + 1;
		if (pthread_create((pthread_t *)&array[i], &attr,
				   (void *)thread, (void *)&arg[i])) {
			if (errno == EAGAIN) {
				fprintf(stderr,
					"\tThread [main]: unable to create "
					"thread %d\n", i);
			} else {
				sys_error("pthread_create failed", __LINE__);
			}
		}
		if (debug)
			printf("\tThread [main]: created thread %d\n", i + 1);
	}
	if (pthread_attr_destroy(&attr))
		sys_error("pthread_attr_destroy failed", __LINE__);

	for (i = 0; i < num_threads; i++) {
		void *exit_value;
		printf("\tThread [main]: waiting for thread: %d\n", i + 1);
		if (pthread_join(array[i], &exit_value))
			sys_error("pthread_join failed", __LINE__);

		if (debug)
			printf("\tThread [%d]: return %ld\n", i + 1,
			       (long)exit_value);
		rc += (long)exit_value;
	}

	if (rc != 0) {
		printf("test failed!\n");
		exit(-1);
	}

	printf("\tThread [main] All threads completed successfully...\n");
	exit(0);
}

static void *thread(int *parm)
{
	int num = *parm;
	unsigned long cksum = 0;

	if (debug)
		printf("\tThread [%d]: begin\n", num);

	read_data(num, &cksum);
	if (checksum != cksum) {
		fprintf(stderr, "\tThread [%d]: checksum mismatch!\n", num);
		pthread_exit((void *)-1);
	}

	if (debug)
		printf("\tThread [%d]: done\n", num);

	pthread_exit(NULL);
}

static int read_data(int num, unsigned long *cksum)
{
	int fd;
	const int bufSize = 1024;
	char *buffer;
	int bytes_read = 0;
	int n;
	char *p;

	if (debug)
		printf("\tThread [%d]: read_data()\n", num);

	if ((fd = open(file, O_RDONLY, NULL)) < 0)
		sys_error("open failed /dev/cdrom", __LINE__);

	buffer = malloc(sizeof(char) * bufSize);
	assert(buffer);

	lseek(fd, 1024 * 36, SEEK_SET);
	while (bytes_read < num_bytes) {
		n = read(fd, buffer, bufSize);
		if (n < 0)
			sys_error("read failed", __LINE__);
		else if (n == 0)
			sys_error("End of file", __LINE__);
		bytes_read += n;

		for (p = buffer; p < buffer + n; p++)
			*cksum += *p;

		if (debug)
			printf("\tThread [%d] bytes read: %5d checksum: "
			       "%-#12lx\n", num, bytes_read, *cksum);
	}
	free(buffer);

	if (debug)
		printf("\tThread [%d] bytes read: %5d checksum: %-#12lx\n",
		       num, bytes_read, *cksum);

	if (close(fd) < 0)
		sys_error("close failed", __LINE__);

	if (debug)
		printf("\tThread [%d]: done\n", num);

	return (0);
}

static void parse_args(int argc, char **argv)
{
	int i;
	int errflag = 0;
	char *program_name = *argv;
	extern char *optarg;	/* Command line option */

	while ((i = getopt(argc, argv, "df:n:b:m:?")) != EOF) {
		switch (i) {
		case 'd':	/* debug option */
			debug++;
			break;
		case 'f':	/* file to read from */
			file = optarg;
			break;
		case 'm':	/* num MB to read */
			num_bytes = atoi(optarg) * 1024 * 1024;
			break;
		case 'b':	/* num bytes to read */
			num_bytes = atoi(optarg);
			break;
		case 'n':	/* number of threads */
			num_threads = atoi(optarg);
			break;
		case '?':	/* help */
			errflag++;
			break;
		}
	}
	if (num_bytes < 0) {
		errflag++;
		fprintf(stderr, "ERROR: num_bytes must be greater than 0");
	}
	if (num_threads < 0) {
		errflag++;
		fprintf(stderr, "ERROR: num_threads must be greater than 0");
	}

	if (errflag) {
		fprintf(stderr, "\nUsage: %s"
			" [-n xx] [-m|b xx] [-d]\n\n"
			"\t-n xx    Number of threads to create (up to %d)\n"
			"\t-f file  File to read from\n"
			"\t-m xx    Number of MB to read\n"
			"\t-b xx    Number of bytes to read\n"
			"\t-d       Debug option\n", program_name,
			DEFAULT_NUM_THREADS);
		exit(2);
	}
}

static void sys_error(const char *msg, int line)
{
	fprintf(stderr, "ERROR [%d: %s: %s]\n", line, msg, strerror(errno));
	exit(-1);
}
