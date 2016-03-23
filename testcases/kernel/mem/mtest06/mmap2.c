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
/*									      */
/* History:	July - 02 - 2001 Created by Manoj Iyer, IBM Austin TX.	      */
/*			         email:manjo@austin.ibm.com		      */
/*									      */
/*		July - 07 - 2001 Modified - changed MAP_PRIVATE to MAP_SHARED */
/*			         read defect 187 for details.	              */
/*									      */
/*		July - 09 - 2001 Modified - added option to MAP_PRIVATE or    */
/*				 MAP_SHARED, -p, default is to MAP_SHARED.    */
/*									      */
/*		July - 09 - 2001 Modified - added option '-a' MAP_ANONYMOUS.  */
/*                               Default is to map a file.		      */
/*									      */
/*		Aug  - 01 - 2001 Modified - added option 'a' to getop list.   */
/*									      */
/*		Oct  - 25 - 2001 Modified - changed scheme. Test will be run  */
/*				 once unless -x option is used.               */
/*									      */
/*		Apr  - 16 - 2003 Modified - replaced tempnam() use with       */
/*				 mkstemp(). -Robbie Williamson                */
/*				 email:robbiew@us.ibm.com                     */
/*									      */
/*		May  - 12 - 2003 Modified - remove the huge files when        */
/*				 we are done with the test - Paul Larson      */
/*				 email:plars@linuxtestproject.org             */
/* File:	mmap2.c							      */
/*									      */
/* Description: Test the LINUX memory manager. The program is aimed at        */
/*              stressing the memory manager by repeaded map/write/unmap of a */
/*		large (by default 128MB) file.			              */
/*									      */
/*		Create a file of the specified size in mb, map the file,      */
/*		change the contents of the file and unmap it. This is repeated*/
/*		several times for the specified number of hours.	      */
/*									      */
/******************************************************************************/

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
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include "test.h"

#define MB (1024 * 1024)
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static int mkfile(int size)
{
	int fd;
	int index = 0;
	char buff[4096];
	char template[PATH_MAX];

	memset(buff, 'a', 4096);
	snprintf(template, PATH_MAX, "ashfileXXXXXX");
	fd = mkstemp(template);
	if (fd == -1) {
		perror("mkfile(): mkstemp()");
		return -1;
	} else {
		unlink(template);
		fprintf(stdout, "creating tmp file and writing 'a' to it ");
	}

	while (index < (size * MB)) {
		index += 4096;
		if (write(fd, buff, 4096) == -1) {
			perror("mkfile(): write()");
			return -1;
		}
	}
	fprintf(stdout, "created file of size %d\n"
		"content of the file is 'a'\n", index);

	if (fsync(fd) == -1) {
		perror("mkfile(): fsync()");
		return -1;
	}
	return fd;
}

static void sig_handler(int signal)
{
	if (signal != SIGALRM) {
		fprintf(stderr, "sig_handlder(): unexpected signal caught"
			"[%d]\n", signal);
		exit(-1);
	} else
		fprintf(stdout, "Test ended, success\n");
	exit(0);
}

static void usage(char *progname)
{
	fprintf(stderr,
		"Usage: %s -h -s -x\n"
		"\t -a set map_flags to MAP_ANONYMOUS\n"
		"\t -h help, usage message.\n"
		"\t -p set map_flag to MAP_PRIVATE.\tdefault:"
		"MAP_SHARED\n"
		"\t -s size of the file/memory to be mmaped.\tdefault:"
		"128MB\n"
		"\t -x time for which test is to be run.\tdefault:"
		"24 Hrs\n", progname);
	exit(-1);
}

unsigned long get_available_memory_mb(void)
{
	unsigned long ps, pn;

	ps = sysconf(_SC_PAGESIZE);
	pn = sysconf(_SC_AVPHYS_PAGES);
	return (ps / 1024) * pn / 1024;
}

int main(int argc, char **argv)
{
	int fd;
	unsigned long fsize = 128;
	float exec_time = 24;
	int c;
	int sig_ndx;
	int map_flags = MAP_SHARED;
	int map_anon = FALSE;
	int run_once = TRUE;
	char *memptr;
	unsigned long avail_memory_mb;
	struct sigaction sigptr;

	static struct signal_info {
		int signum;
		char *signame;
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

	while ((c = getopt(argc, argv, "ahps:x:")) != -1) {
		switch (c) {
		case 'a':
			map_anon = TRUE;
			break;
		case 'h':
			usage(argv[0]);
			exit(-1);
			break;
		case 'p':
			map_flags = MAP_PRIVATE;
			break;
		case 's':
			fsize = atoi(optarg);
			if (fsize == 0)
				fprintf(stderr, "Using default "
					"fsize %lu MB\n", fsize = 128);
			break;
		case 'x':
			exec_time = atof(optarg);
			if (exec_time == 0)
				fprintf(stderr, "Using default exec "
					"time %f hrs", exec_time = (float)24);
			run_once = FALSE;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	fprintf(stdout, "MM Stress test, map/write/unmap large file\n"
		"\tTest scheduled to run for:       %f\n"
		"\tSize of temp file in MB:         %lu\n", exec_time, fsize);

	avail_memory_mb = get_available_memory_mb();
	fprintf(stdout, "Available memory: %ldMB\n", avail_memory_mb);
	if (fsize > avail_memory_mb) {
		fprintf(stdout, "Not enough memory to run this case\n");
		exit(0);
	}

	alarm(exec_time * 3600.00);

	sigptr.sa_handler = sig_handler;
	sigfillset(&sigptr.sa_mask);
	sigptr.sa_flags = 0;
	for (sig_ndx = 0; sig_info[sig_ndx].signum != -1; sig_ndx++) {
		sigaddset(&sigptr.sa_mask, sig_info[sig_ndx].signum);
		if (sigaction(sig_info[sig_ndx].signum, &sigptr,
			      NULL) == -1) {
			perror("man(): sigaction()");
			fprintf(stderr, "could not set handler for SIGALRM,"
				"errno = %d\n", errno);
			exit(-1);
		}
	}

	do {
		if (!map_anon) {
			fd = mkfile(fsize);
			if (fd == -1) {
				fprintf(stderr, "main(): mkfile(): Failed "
					"to create temp file.\n");
				exit(-1);
			}
		} else {
			fd = -1;
			map_flags = map_flags | MAP_ANONYMOUS;
		}
		memptr = mmap(0, (fsize * MB), PROT_READ | PROT_WRITE,
			      map_flags, fd, 0);
		if (memptr == MAP_FAILED) {
			perror("main(): mmap()");
			exit(-1);
		} else
			fprintf(stdout, "file mapped at %p\n"
				"changing file content to 'A'\n", memptr);

		memset(memptr, 'A', ((fsize * MB) / sizeof(char)));

		if (msync(memptr, ((fsize * MB) / sizeof(char)),
			  MS_SYNC | MS_INVALIDATE) == -1) {
			perror("main(): msync()");
			exit(-1);
		}

		if (munmap(memptr, (fsize * MB) / sizeof(char)) == -1) {
			perror("main(): munmap()");
			exit(-1);
		} else
			fprintf(stdout, "unmapped file at %p\n", memptr);

		close(fd);
		sync();
	} while (TRUE && !run_once);
	exit(0);
}
