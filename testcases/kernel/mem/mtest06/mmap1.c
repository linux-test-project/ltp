/******************************************************************************/
/*									      */
/* Copyright (c) International Business Machines  Corp., 2001		      */
/* Copyright (c) 2001 Manoj Iyer <manjo@austin.ibm.com>                       */
/* Copyright (c) 2003 Robbie Williamson <robbiew@us.ibm.com>                  */
/* Copyright (c) 2004 Paul Larson <plars@linuxtestproject.org>                */
/* Copyright (c) 2007 <rsalveti@linux.vnet.ibm.com>                           */
/* Copyright (c) 2007 Suzuki K P <suzuki@in.ibm.com>                          */
/* Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>                          */
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
/* Description:	Test the LINUX memory manager. The program is aimed at        */
/*		stressing the memory manager by simultanious map/unmap/read   */
/*		by light weight processes, the test is scheduled to run for   */
/*		a mininum of 24 hours.					      */
/*									      */
/*		Create two light weight processes X and Y.                    */
/*		X - maps, writes  and unmap a file in a loop.	              */
/*		Y - read from this mapped region in a loop.		      */
/*	        read must be a success between map and unmap of the region.   */
/*									      */
/******************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "test.h"
#include "safe_macros.h"

#define DISTANT_MMAP_SIZE (64*1024*1024)
#define OPT_MISSING(prog, opt) do { \
	fprintf(stderr, "%s: option -%c ", prog, opt); \
        fprintf(stderr, "requires an argument\n"); \
	usage(prog); \
} while (0)

static int verbose_print = 0;
static char *volatile map_address;
static jmp_buf jmpbuf;
static volatile char read_lock = 0;
static void *distant_area;

char *TCID = "mmap1";
int TST_TOTAL = 1;

static void sig_handler(int signal, siginfo_t * info, void *ut)
{
	switch (signal) {
	case SIGALRM:
		tst_resm(TPASS, "Test ended, success");
		_exit(TPASS);
	case SIGSEGV:
		longjmp(jmpbuf, 1);
		break;
	default:
		fprintf(stderr, "Unexpected signal - %d --- exiting\n", signal);
		_exit(TBROK);
	}
}

/*
 * Signal handler that is active, when file is mapped, eg. we do not expect
 * SIGSEGV to be delivered.
 */
static void sig_handler_mapped(int signal, siginfo_t * info, void *ut)
{
	switch (signal) {
	case SIGALRM:
		tst_resm(TPASS, "Test ended, success");
		_exit(TPASS);
	case SIGSEGV:
		tst_resm(TINFO, "[%lu] Unexpected page fault at %p",
			 pthread_self(), info->si_addr);
		_exit(TFAIL);
		break;
	default:
		fprintf(stderr, "Unexpected signal - %d --- exiting\n", signal);
		_exit(TBROK);
	}
}

int mkfile(int size)
{
	char template[] = "/tmp/ashfileXXXXXX";
	int fd, i;

	if ((fd = mkstemp(template)) == -1)
		tst_brkm(TBROK | TERRNO, NULL, "mkstemp() failed");

	unlink(template);

	for (i = 0; i < size; i++)
		if (write(fd, "a", 1) == -1)
			tst_brkm(TBROK | TERRNO, NULL, "write() failed");

	if (write(fd, "\0", 1) == -1)
		tst_brkm(TBROK | TERRNO, NULL, "write() failed");

	if (fsync(fd) == -1)
		tst_brkm(TBROK | TERRNO, NULL, "fsync() failed");

	return fd;
}

void *map_write_unmap(void *ptr)
{
	struct sigaction sa;
	long *args = ptr;
	long i;
	int j;

	tst_resm(TINFO, "[%lu] - map, change contents, unmap files %ld times",
		 pthread_self(), args[2]);

	if (verbose_print)
		tst_resm(TINFO, "map_write_unmap() arguments are: "
			 "fd - arg[0]: %ld; "
			 "size of file - arg[1]: %ld; "
			 "num of map/write/unmap - arg[2]: %ld",
			 args[0], args[1], args[2]);

	for (i = 0; i < args[2]; i++) {
		map_address = mmap(distant_area, (size_t) args[1],
			PROT_WRITE | PROT_READ, MAP_SHARED, (int)args[0], 0);

		if (map_address == (void *)-1) {
			perror("map_write_unmap(): mmap()");
			pthread_exit((void *)1);
		}

		while (read_lock)
			sched_yield();

		sigfillset(&sa.sa_mask);
		sigdelset(&sa.sa_mask, SIGSEGV);
		sa.sa_flags = SA_SIGINFO | SA_NODEFER;
		sa.sa_sigaction = sig_handler_mapped;

		if (sigaction(SIGSEGV, &sa, NULL)) {
			perror("map_write_unmap(): sigaction()");
			pthread_exit((void *)1);
		}

		if (verbose_print)
			tst_resm(TINFO, "map address = %p", map_address);

		for (j = 0; j < args[1]; j++) {
			map_address[j] = 'a';
			if (random() % 2)
				sched_yield();
		}

		if (verbose_print)
			tst_resm(TINFO,
				 "[%ld] times done: of total [%ld] iterations, "
				 "map_write_unmap():memset() content of memory = %s",
				 i, args[2], (char *)map_address);

		sigfillset(&sa.sa_mask);
		sigdelset(&sa.sa_mask, SIGSEGV);
		sa.sa_flags = SA_SIGINFO | SA_NODEFER;
		sa.sa_sigaction = sig_handler;

		if (sigaction(SIGSEGV, &sa, NULL)) {
			perror("map_write_unmap(): sigaction()");
			pthread_exit((void *)1);
		}

		if (munmap(map_address, (size_t) args[1]) == -1) {
			perror("map_write_unmap(): mmap()");
			pthread_exit((void *)1);
		}
	}

	pthread_exit(NULL);
}

void *read_mem(void *ptr)
{
	long i;
	long *args = ptr;
	int j;

	tst_resm(TINFO, "[%lu] - read contents of memory %p %ld times",
		 pthread_self(), map_address, args[2]);

	if (verbose_print)
		tst_resm(TINFO, "read_mem() arguments are: "
			 "number of reads to be performed - arg[2]: %ld; "
			 "read from address %p", args[2], map_address);

	for (i = 0; i < args[2]; i++) {

		if (verbose_print)
			tst_resm(TINFO, "read_mem() in while loop %ld times "
				 "to go %ld times", i, args[2]);

		if (setjmp(jmpbuf) == 1) {
			read_lock = 0;
			if (verbose_print)
				tst_resm(TINFO, "page fault occurred due to "
					 "a read after an unmap");
		} else {
			if (verbose_print) {
				read_lock = 1;
				tst_resm(TINFO,
					 "read_mem(): content of memory: %s",
					 (char *)map_address);
				read_lock = 0;
			}
			for (j = 0; j < args[1]; j++) {
				read_lock = 1;
				if (map_address[j] != 'a')
					pthread_exit((void *)-1);
				read_lock = 0;
				if (random() % 2)
					sched_yield();
			}
		}
	}

	pthread_exit(NULL);
}

static void usage(char *progname)
{
	fprintf(stderr, "Usage: %s -d -l -s -v -x\n"
		"\t -h help, usage message.\n"
		"\t -l number of mmap/write/unmap     default: 1000\n"
		"\t -s size of the file to be mmapped default: 1024 bytes\n"
		"\t -v print more info.               default: quiet\n"
		"\t -x test execution time            default: 24 Hrs\n",
		progname);

	exit(-1);
}

struct signal_info {
	int signum;
	char *signame;
};

static struct signal_info sig_info[] = {
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

int main(int argc, char **argv)
{
	int c, i;
	int file_size;
	int num_iter;
	double exec_time;
	int fd;
	void *status;
	pthread_t thid[2];
	long chld_args[3];
	extern char *optarg;
	struct sigaction sigptr;
	int ret;

	/* set up the default values */
	file_size = 1024;
	num_iter = 1000;
	exec_time = 24;

	while ((c = getopt(argc, argv, "hvl:s:x:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 'l':
			if ((num_iter = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (num_iter < 0)
				printf
				    ("WARNING: bad argument. Using default %d\n",
				     (num_iter = 1000));
			break;
		case 's':
			if ((file_size = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else if (file_size < 0)
				printf
				    ("WARNING: bad argument. Using default %d\n",
				     (file_size = 1024));
			break;
		case 'v':
			verbose_print = 1;
			break;
		case 'x':
			exec_time = atof(optarg);
			if (exec_time == 0)
				OPT_MISSING(argv[0], optopt);
			else if (exec_time < 0)
				printf
				    ("WARNING: bad argument. Using default %.0f\n",
				     (exec_time = 24));
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	/* We don't want other mmap calls to map into same area as is
	 * used for test (mmap_address). The test expects read to return
	 * test pattern or read must fail with SIGSEGV. Find an area
	 * that we can use, which is unlikely to be chosen for other
	 * mmap calls. */
	distant_area = mmap(0, DISTANT_MMAP_SIZE, PROT_WRITE | PROT_READ,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (distant_area == (void *)-1)
		tst_brkm(TBROK | TERRNO, NULL, "distant_area: mmap()");
	SAFE_MUNMAP(NULL, distant_area, (size_t)DISTANT_MMAP_SIZE);
	distant_area += DISTANT_MMAP_SIZE / 2;

	if (verbose_print)
		tst_resm(TINFO, "Input parameters are: File size:  %d; "
			 "Scheduled to run:  %lf hours; "
			 "Number of mmap/write/read:  %d",
			 file_size, exec_time, num_iter);

	alarm(exec_time * 3600);

	/* Do not mask SIGSEGV, as we are interested in handling it. */
	sigptr.sa_sigaction = sig_handler;
	sigfillset(&sigptr.sa_mask);
	sigdelset(&sigptr.sa_mask, SIGSEGV);
	sigptr.sa_flags = SA_SIGINFO | SA_NODEFER;

	for (i = 0; sig_info[i].signum != -1; i++) {
		if (sigaction(sig_info[i].signum, &sigptr, NULL) == -1) {
			perror("man(): sigaction()");
			fprintf(stderr,
				"could not set handler for %s, errno = %d\n",
				sig_info[i].signame, errno);
			exit(-1);
		}
	}

	for (;;) {
		if ((fd = mkfile(file_size)) == -1)
			tst_brkm(TBROK, NULL,
				 "main(): mkfile(): Failed to create temp file");

		if (verbose_print)
			tst_resm(TINFO, "Tmp file created");

		chld_args[0] = fd;
		chld_args[1] = file_size;
		chld_args[2] = num_iter;

		if ((ret =
		     pthread_create(&thid[0], NULL, map_write_unmap,
				    chld_args)))
			tst_brkm(TBROK, NULL, "main(): pthread_create(): %s",
				 strerror(ret));

		tst_resm(TINFO, "created writing thread[%lu]", thid[0]);

		if ((ret = pthread_create(&thid[1], NULL, read_mem, chld_args)))
			tst_brkm(TBROK, NULL, "main(): pthread_create(): %s",
				 strerror(ret));

		tst_resm(TINFO, "created reading thread[%lu]", thid[1]);

		for (i = 0; i < 2; i++) {
			if ((ret = pthread_join(thid[i], &status)))
				tst_brkm(TBROK, NULL,
					 "main(): pthread_join(): %s",
					 strerror(ret));

			if (status)
				tst_brkm(TFAIL, NULL,
					 "thread [%lu] - process exited "
					 "with %ld", thid[i], (long)status);
		}

		close(fd);
	}

	exit(0);
}
