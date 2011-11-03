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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
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
#include "usctest.h"

#define OPT_MISSING(prog, opt) do { \
	fprintf(stderr, "%s: option -%c ", prog, opt); \
        fprintf(stderr, "requires an argument\n"); \
	usage(prog); \
} while (0)

int 	   verbose_print = 0;
caddr_t    *map_address;
sigjmp_buf jmpbuf;

char *TCID = "mmap1";
int TST_TOTAL = 1;

static void sig_handler(int signal, siginfo_t *info, void *ut)
{
	switch (signal) {
	case SIGALRM:
		tst_resm(TPASS, "Test ended, success");
		_exit(0);

	case SIGSEGV:
		if (info->si_code == SEGV_MAPERR &&
		    info->si_addr == map_address) {
			tst_resm(TINFO, "page fault occurred at %p", map_address);
			longjmp(jmpbuf, 1);
		}
        default:
		fprintf(stderr, "caught unexpected signal - %d --- exiting\n",
		        signal);
		_exit(-1);
	}
}

static void set_timer(double run_time)
{
	struct itimerval timer;

	memset(&timer, 0, sizeof(struct itimerval));
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = (time_t)(run_time * 3600);

	if (setitimer(ITIMER_REAL, &timer, NULL))
		tst_brkm(TBROK|TERRNO, NULL, "setitimer() failed");
}

int mkfile(int size)
{
	int  fd;
	char template[PATH_MAX];

	snprintf(template, PATH_MAX, "ashfileXXXXXX");

	if ((fd = mkstemp(template)) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "mkstemp() failed");
		
	unlink(template);
	
	if (lseek(fd, (size - 1), SEEK_SET) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "lseek() failed fd = %i size = %i",
		         fd, size);

	if (write(fd, "\0", 1) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "write() failed");
		
	if (fsync(fd) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "fsync() failed");

	return fd;
}

void *map_write_unmap(void *ptr)
{
	long *args = ptr;
	long i;

	tst_resm(TINFO, "pid[%d]: map, change contents, unmap files %ld times",
	         getpid(), args[2]);

	if (verbose_print)
		tst_resm(TINFO, "map_write_unmap() arguments are: "
		                "fd - arg[0]: %ld; "
		                "size of file - arg[1]: %ld; "
		                "num of map/write/unmap - arg[2]: %ld",
		                args[0], args[1], args[2]);

	for (i = 0; i < args[2]; i++) {
		map_address = mmap(0, (size_t)args[1], PROT_WRITE|PROT_READ,
		                        MAP_SHARED, (int)args[0], 0);
		
		if (map_address == (caddr_t *) -1) {
			perror("map_write_unmap(): mmap()");
			pthread_exit((void *)1);
		}

		if (verbose_print)
			tst_resm(TINFO, "map address = %p", map_address);

		memset(map_address, 'a', args[1]);

		if (verbose_print)
			tst_resm(TINFO, "[%ld] times done: of total [%ld] iterations, "
			         "map_write_unmap():memset() content of memory = %s",
                                 i, args[2], (char*)map_address);
		usleep(1);

		if (munmap(map_address, (size_t)args[1]) == -1) {
			perror("map_write_unmap(): mmap()");
			pthread_exit((void *)1);
		}
	}

	pthread_exit((void *)0);
}

void *read_mem(void *ptr)
{
	long i = 0;
	long *args = ptr;

	tst_resm(TINFO, "pid[%d] - read contents of memory %p %ld times",
	         getpid(), map_address, args[2]);

	if (verbose_print)
		tst_resm(TINFO, "read_mem() arguments are: "
		         "number of reads to be performed - arg[2]: %ld; "
		         "read from address %p",
                         args[2], map_address);

	for (i = 0; i < args[2]; i++) {
		
		if (verbose_print)
			tst_resm(TINFO, "read_mem() in while loop %ld times "
			         "to go %ld times", i, args[2]);

		if (setjmp(jmpbuf) == 1) {
			if (verbose_print)
				tst_resm(TINFO, "page fault occurred due to "
				         "a read after an unmap from %p",
					 map_address);
		} else {
			if (verbose_print)
				tst_resm(TINFO, "read_mem(): content of memory: %s",
				         (char *)map_address);

			if (strncmp((char *)map_address, "a", 1) != 0)
				pthread_exit((void *)-1);

			usleep(1);
		}
	}

	pthread_exit((void *)0);
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
	int  signum;
	char *signame;
};

static struct signal_info sig_info[] = {
	{SIGHUP,  "SIGHUP" },
	{SIGINT,  "SIGINT" },
	{SIGQUIT, "SIGQUIT"},
	{SIGABRT, "SIGABRT"},
	{SIGBUS,  "SIGBUS" },
	{SIGSEGV, "SIGSEGV"},
	{SIGALRM, "SIGALRM"},
	{SIGUSR1, "SIGUSR1"},
	{SIGUSR2, "SIGUSR2"},
	{-1,      "ENDSIG" }
};

int main(int  argc, char **argv)
{
	int c, i;
	int file_size;
	int num_iter;
	double exec_time;
	int fd;
	int status[2];
	pthread_t thid[2];
	long chld_args[3];
	extern char *optarg;
	struct sigaction sigptr;
	int ret;

	/* set up the default values */
	file_size = 1024;
	num_iter = 1000;
	exec_time = 24;

	while ((c =  getopt(argc, argv, "hvl:s:x:")) != -1) {
		switch(c) {
		case 'h':
			usage(argv[0]);
		break;
		case 'l':
			if ((num_iter = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
                	else
				if (num_iter < 0)
					printf("WARNING: bad argument. Using default %d\n", (num_iter = 1000));
		break;
		case 's':
			if ((file_size = atoi(optarg)) == 0)
				OPT_MISSING(argv[0], optopt);
			else
				if (file_size < 0)
					printf("WARNING: bad argument. Using default %d\n", (file_size = 1024));
		break;
		case 'v':
			verbose_print = 1;
		break;
		case 'x':
			exec_time = atof(optarg);
			if (exec_time == 0)
				OPT_MISSING(argv[0], optopt);
			else
				if (exec_time < 0)
					printf("WARNING: bad argument. Using default %.0f\n", (exec_time = 24));
		break;
		default:
			usage(argv[0]);
		break;
		}
	}

	if (verbose_print)
		tst_resm(TINFO, "Input parameters are: File size:  %d; "
		         "Scheduled to run:  %lf hours; "
		         "Number of mmap/write/read:  %d",
		         file_size, exec_time, num_iter);

	set_timer(exec_time);

	/* Do not mask SIGSEGV, as we are interested in handling it. */
	sigptr.sa_sigaction = sig_handler;
	sigfillset(&sigptr.sa_mask);
	sigdelset(&sigptr.sa_mask, SIGSEGV);
	sigptr.sa_flags = SA_SIGINFO | SA_NODEFER;
	
	for (i = 0; sig_info[i].signum != -1; i++) {
		if (sigaction(sig_info[i].signum, &sigptr, NULL) == -1) {
			perror( "man(): sigaction()" );
			fprintf(stderr, "could not set handler for %s, errno = %d\n",
			sig_info[i].signame, errno);
			exit(-1);
		}
	}

	for (;;) {
	        /* create temporary file */
		if ((fd = mkfile(file_size)) == -1)
			tst_brkm(TBROK, NULL, "main(): mkfile(): Failed to create temp file");
		
		if (verbose_print)
			tst_resm(TINFO, "Tmp file created");

		chld_args[0] = fd;
		chld_args[1] = file_size;
		chld_args[2] = num_iter;

		if ((ret = pthread_create(&thid[0], NULL, map_write_unmap, chld_args)))
			tst_brkm(TBROK, NULL, "main(): pthread_create(): %s", strerror(ret));
		
		tst_resm(TINFO, "created thread[%ld]", thid[0]);

		sched_yield();
		
		if ((ret = pthread_create(&thid[1], NULL, read_mem, chld_args)))
			tst_brkm(TBROK, NULL, "main(): pthread_create(): %s", strerror(ret));
		
		tst_resm(TINFO, "created thread[%ld]", thid[1]);

		sched_yield();

		for (i = 0; i < 2; i++) {
			if ((ret = pthread_join(thid[i], (void*)&status[i])))
				tst_brkm(TBROK, NULL, "main(): pthread_join(): %s",
				         strerror(ret));
			
			if (status[i])
				tst_brkm(TFAIL, NULL, "thread [%ld] - process exited "
				         "with %d\n", thid[i], status[i]);
		}

		close(fd);
	}

	exit(0);
}
