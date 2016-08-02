/******************************************************************************
 *									      *
 * Copyright (c) International Business Machines  Corp., 2001		      *
 *  2001 Created by Manoj Iyer, IBM Austin TX <manjo@austin.ibm.com>          *
 *									      *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.					      *
 *									      *
 * This program is distributed in the hope that it will be useful,	      *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of	      *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	              *
 * the GNU General Public License for more details.			      *
 *									      *
 * You should have received a copy of the GNU General Public License	      *
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *									      *
 ******************************************************************************/

/*
 * Tests the LINUX memory manager. The program is aimed at stressing the memory
 * manager by repeaded map/write/unmap of file/memory of random size (maximum
 * 1GB) this is done by multiple threads.
 *
 * Create a file of random size upto 1000 times 4096, map it, change the
 * contents of the file and unmap it. This is repeated several times for the
 * specified number of hours by a certain number of threads.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "test.h"

static int mkfile(int *size)
{
	int fd;
	int index = 0;
	char buff[4096];
	char template[PATH_MAX];

	memset(buff, 'a', 4096);
	snprintf(template, PATH_MAX, "ashfileXXXXXX");
	if ((fd = mkstemp(template)) == -1) {
		perror("mkfile(): mkstemp()");
		return -1;
	}
	unlink(template);

	srand(time(NULL) % 100);
	*size = (1 + (int)(1000.0 * rand() / (RAND_MAX + 1.0))) * 4096;

	while (index < *size) {
		index += 4096;
		if (write(fd, buff, 4096) == -1) {
			perror("mkfile(): write()");
			return -1;
		}
	}

	if (fsync(fd) == -1) {
		perror("mkfile(): fsync()");
		return -1;
	}

	return fd;
}

static void sig_handler(int signal)
{
	if (signal != SIGALRM) {
		fprintf(stderr,
			"sig_handlder(): unexpected signal caught [%d]\n",
			signal);
		exit(-1);
	} else
		fprintf(stdout, "Test ended, success\n");
	exit(0);
}

static void usage(char *progname)
{
	fprintf(stderr,
		"Usage: %s -h -l -n -p -x\n"
		"\t -h help, usage message.\n"
		"\t -l number of map - write - unmap.    default: 1000\n"
		"\t -n number of LWP's to create.        default: 20\n"
		"\t -p set mapping to MAP_PRIVATE.       default: MAP_SHARED\n"
		"\t -x time for which test is to be run. default: 24 Hrs\n",
		progname);
	exit(-1);
}

void *map_write_unmap(void *args)
{
	int fsize;
	int fd;
	int mwu_ndx = 0;
	caddr_t *map_address;
	int map_type;
	long *mwuargs = args;

	while (mwu_ndx++ < mwuargs[0]) {
		if ((fd = mkfile(&fsize)) == -1) {
			fprintf(stderr,
				"main(): mkfile(): Failed to create temp file.\n");
			pthread_exit((void *)-1);
		}

		if (mwuargs[1])
			map_type = MAP_PRIVATE;
		else
			map_type = MAP_SHARED;

		map_address = mmap(NULL, fsize, PROT_WRITE | PROT_READ, map_type, fd, 0);
		if (map_address == MAP_FAILED) {
			perror("map_write_unmap(): mmap()");
			pthread_exit((void *)-1);
		}

		memset(map_address, 'A', fsize);

		fprintf(stdout,
			"Map address = %p\nNum iter: [%d]\nTotal Num Iter: [%ld]",
			map_address, mwu_ndx, mwuargs[0]);
		usleep(1);
		if (munmap(map_address, fsize) == -1) {
			perror("map_write_unmap(): mmap()");
			pthread_exit((void *)-1);
		}
		close(fd);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int c;
	int num_iter = 1000;
	int num_thrd = 40;
	int i;
	float exec_time = 24;
	void *status;
	int sig_ndx;
	pthread_t thid[1000];
	long pargs[3];
	struct sigaction sigptr;
	int map_private = 0;

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
		{-1, "ENDSIG"}
	};

	while ((c = getopt(argc, argv, "h:l:n:px:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 'l':
			if ((num_iter = atoi(optarg)) == 0)
				num_iter = 1000;
			break;
		case 'n':
			if ((num_thrd = atoi(optarg)) == 0)
				num_thrd = 20;
			break;
		case 'p':
			map_private = 1;
			break;
		case 'x':
			if ((exec_time = atof(optarg)) == 0)
				exec_time = 24;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	sigptr.sa_handler = sig_handler;
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
	pargs[0] = num_iter;
	pargs[1] = map_private;
	alarm(exec_time * 3600.00);

	fprintf(stdout,
		"\n\n\nTest is set to run with the following parameters:\n"
		"\tDuration of test: [%f]hrs\n"
		"\tNumber of threads created: [%d]\n"
		"\tnumber of map-write-unmaps: [%d]\n"
		"\tmap_private?(T=1 F=0): [%d]\n\n\n\n", exec_time, num_thrd,
		num_iter, map_private);

	for (;;) {
		for (i = 0; i < num_thrd; i++) {
			if (pthread_create(&thid[i], NULL, map_write_unmap, pargs)) {
				perror("main(): pthread_create()");
				exit(-1);
			}
			sched_yield();
		}

		for (i = 0; i < num_thrd; i++) {
			if (pthread_join(thid[i], &status)) {
				perror("main(): pthread_create()");
				exit(-1);
			} else {
				if (status) {
					fprintf(stderr,
						"thread [%d] - process exited with errors\n",
						i);
					exit(-1);
				}
			}
		}
	}
	exit(0);
}
