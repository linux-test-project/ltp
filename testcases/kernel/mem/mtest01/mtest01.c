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
 *  FILE		: mtest01.c
 *  DESCRIPTION : mallocs memory <chunksize> at a time until malloc fails.
 *  HISTORY:
 *	04/10/2001 Paul Larson (plars@us.ibm.com)
 *	  written
 *	11/09/2001 Manoj Iyer (manjo@austin.ibm.com)
 *	  Modified.
 *	  - Removed compile warnings.
 *	  - Added header file #include <unistd.h> definition for getopt()
 *	05/13/2003 Robbie Williamson (robbiew@us.ibm.com)
 *	  Modified.
 *	  - Rewrote the test to be able to execute on large memory machines.
 *
 */

#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"

#define FIVE_HUNDRED_MB (unsigned long long)(500*1024*1024)
#define ONE_GB	(unsigned long long)(1024*1024*1024)
#define THREE_GB (unsigned long long)(3*ONE_GB)

char *TCID = "mtest01";
int TST_TOTAL = 1;
int pid_count = 0;

void handler(int signo)
{
	pid_count++;
}

int main(int argc, char* argv[])
{
	int c;
	char *mem;
	float percent;
	unsigned int maxpercent = 0, dowrite = 0, verbose=0, j;
	unsigned long bytecount, alloc_bytes, max_pids;
	unsigned long long original_maxbytes, maxbytes = 0;
	unsigned long long pre_mem = 0, post_mem = 0;
	unsigned long long total_ram, total_free, D, C;
	int chunksize = 1024*1024; /* one meg at a time by default */
	struct sysinfo sstats;
	int i, pid_cntr;
	pid_t pid, *pid_list;
	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGRTMIN,  &act, 0);

	while ((c = getopt(argc, argv, "c:b:p:wvh")) != -1) {
		switch(c) {
		case 'c':
			chunksize = atoi(optarg);
			break;
		case 'b':
			if (maxpercent != 0)
				tst_brkm(TBROK, NULL,
				    "ERROR: -b option cannot be used with -p "
				    "option at the same time");
			maxbytes = atoll(optarg);
			break;
		case 'p':
			if (maxbytes != 0)
				tst_brkm(TBROK, NULL,
				    "ERROR: -p option cannot be used with -b "
				    "option at the same time");
			maxpercent = atoi(optarg);
			if (maxpercent <= 0)
				tst_brkm(TBROK, NULL,
				    "ERROR: -p option requires number greater "
				    "than 0");
			if (maxpercent > 99)
				tst_brkm(TBROK, NULL,
				    "ERROR: -p option cannot be greater than "
				    "99");
			break;
		case 'w':
			dowrite = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			printf("usage: %s [-c <bytes>] [-b <bytes>|-p <percent>] [-v]\n", argv[0]);
			printf("\t-c <num>\tsize of chunk in bytes to malloc on each pass\n");
			printf("\t-b <bytes>\tmaximum number of bytes to allocate before stopping\n");
			printf("\t-p <bytes>\tpercent of total memory used at which the program stops\n");
			printf("\t-w\t\twrite to the memory after allocating\n");
			printf("\t-v\t\tverbose\n");
			printf("\t-h\t\tdisplay usage\n");
			exit(1);
		}
	}

	sysinfo(&sstats);
	total_ram = sstats.totalram + sstats.totalswap;
	total_free = sstats.freeram + sstats.freeswap;
	/* Total Free Pre-Test RAM */
	pre_mem = sstats.mem_unit * total_free;
	max_pids = total_ram / (unsigned long)FIVE_HUNDRED_MB + 1;

	if ((pid_list = malloc(max_pids * sizeof(pid_t))) == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "malloc failed.");
	memset(pid_list, 0, max_pids * sizeof(pid_t));

	/* Currently used memory */
	C = sstats.mem_unit * (total_ram - total_free);
	tst_resm(TINFO, "Total memory already used on system = %llu kbytes",
	    C / 1024);

	if (maxpercent) {
		percent = (float)maxpercent / 100.00;

		/* Desired memory needed to reach maxpercent */
		D = percent * (sstats.mem_unit * total_ram);
		tst_resm(TINFO,
		    "Total memory used needed to reach maximum = %llu kbytes",
		    D / 1024);

		/* Are we already using more than maxpercent? */
		if (C > D) {
			tst_resm(TFAIL,
			    "More memory than the maximum amount you specified "
			    " is already being used");
			free(pid_list);
			tst_exit();
		}

		/* set maxbytes to the extra amount we want to allocate */
		maxbytes = D - C;
		tst_resm(TINFO, "Filling up %d%% of ram which is %llu kbytes",
		    maxpercent, maxbytes / 1024);
	}
	original_maxbytes = maxbytes;
	i = 0;
	pid_cntr = 0;
	pid = fork();
	if (pid != 0)
		pid_cntr++;
	pid_list[i] = pid;

#if defined (_s390_) /* s390's 31bit addressing requires smaller chunks */
	while (pid != 0 && maxbytes > FIVE_HUNDRED_MB) {
		i++;
		maxbytes -= FIVE_HUNDRED_MB;
		pid = fork();
		if (pid != 0) {
			pid_cntr++;
			pid_list[i] = pid;
		}
	}
	if (maxbytes > FIVE_HUNDRED_MB)
		alloc_bytes = FIVE_HUNDRED_MB;
	else
		alloc_bytes = (unsigned long) maxbytes;

#elif __WORDSIZE == 32
	while (pid != 0 && maxbytes > ONE_GB) {
		i++;
		maxbytes -= ONE_GB;
		pid = fork();
		if (pid != 0) {
			pid_cntr++;
			pid_list[i]=pid;
		}
	}
	if (maxbytes > ONE_GB)
		alloc_bytes = ONE_GB;
	else
		alloc_bytes = (unsigned long)maxbytes;

#elif __WORDSIZE == 64
	while (pid != 0 && maxbytes > THREE_GB) {
		i++;
		maxbytes -= THREE_GB;
		pid = fork();
		if (pid != 0) {
			pid_cntr++;
			pid_list[i] = pid;
		}
	}
	if (maxbytes > THREE_GB)
		alloc_bytes = THREE_GB;
	else
		alloc_bytes = maxbytes;
#endif

	if (pid == 0) {
		bytecount = chunksize;
		while (1) {
			if ((mem = malloc(chunksize)) == NULL) {
				tst_resm(TBROK|TERRNO,
				    "stopped at %lu bytes", bytecount);
				free(pid_list);
				tst_exit();
			}
			if (dowrite)
				for (j = 0; j < chunksize; j++)
					*(mem+j) = 'a';
			if (verbose)
				tst_resm(TINFO,
				    "allocated %lu bytes chunksize is %d",
				    bytecount, chunksize);
			bytecount += chunksize;
			if (alloc_bytes && bytecount >= alloc_bytes)
				break;
		}
		if (dowrite)
			tst_resm(TINFO, "... %lu bytes allocated and used.",
			    bytecount);
		else
			tst_resm(TINFO, "... %lu bytes allocated only.",
			    bytecount);
		kill(getppid(), SIGRTMIN);
		while (1)
			sleep(1);
	} else {
		i = 0;
		sysinfo(&sstats);

		if (dowrite) {
			/* Total Free Post-Test RAM */
			post_mem = (unsigned long long)sstats.mem_unit * sstats.freeram;
			post_mem = post_mem + (unsigned long long)sstats.mem_unit * sstats.freeswap;

			while ((((unsigned long long)pre_mem - post_mem) <
			    (unsigned long long)original_maxbytes) &&
			    pid_count < pid_cntr) {
				sleep(1);
				sysinfo(&sstats);
				post_mem = (unsigned long long)sstats.mem_unit * sstats.freeram;
				post_mem = post_mem + (unsigned long long)sstats.mem_unit * sstats.freeswap;
			}
		}
		while (pid_list[i] != 0) {
			kill(pid_list[i], SIGKILL);
			i++;
		}
		if (dowrite)
			tst_resm(TPASS, "%llu kbytes allocated and used.",
			    original_maxbytes / 1024);
		else
			tst_resm(TPASS, "%llu kbytes allocated only.",
			    original_maxbytes / 1024);
	}
	free(pid_list);
	tst_exit();
}
