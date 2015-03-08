/******************************************************************************/
/*                                                                            */
/* Copyright (s) Ying Han <yinghan@google.com>, 2009                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
 ftruncate-mmap: pages are lost after writing to mmaped file,

 We triggered the failure during some internal experiment with
 ftruncate/mmap/write/read sequence. And we found that some pages are
 "lost" after writing to the mmaped file. which in the following test
 cases (count >= 0).

  First we deployed the test cases into group of machines and see about
  >20% failure rate on average. Then, I did couple of experiment to try
  to reproduce it on a single machine. what i found is that:
  1. add a fsync after write the file, i can not reproduce this issue.
  2. add memory pressure(mmap/mlock) while run the test in infinite
  loop, the failure is reproduced quickly. ( background flushing ? )

  The "bad pages" count differs each time from one digit to 4,5 digit
  for 128M ftruncated file. and what i also found that the bad page
  number are contiguous for each segment which total bad pages container
  several segments. ext "1-4, 9-20, 48-50" (  batch flushing ? )

  (The failure is reproduced based on 2.6.29-rc8, also happened on
   2.6.18 kernel. . Here is the simple test case to reproduce it with
   memory pressure. )
*/

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "test.h"

/* Extern Global Variables */
extern int tst_count;

/* Global Variables */
char *TCID = "mmap-corruption01";	/* test program identifier.          */
int TST_TOTAL = 1;		/* total number of tests in this file.   */

long kMemSize = 128 << 20;
int kPageSize = 4096;

char *usage = "-h hours -m minutes -s secs\n";

int anyfail(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed\n");
}

int main(int argc, char **argv)
{
	char *progname;
	int count = 0;
	int i, c;
	char *fname = "test.mmap-corruption";
	char *mem;
	unsigned long alarmtime = 0;
	struct sigaction sa;
	void finish(int sig);

	progname = *argv;
	while ((c = getopt(argc, argv, ":h:m:s:")) != -1) {
		switch (c) {
		case 'h':
			alarmtime += atoi(optarg) * 60 * 60;
			break;
		case 'm':
			alarmtime += atoi(optarg) * 60;
			break;
		case 's':
			alarmtime += atoi(optarg);
			break;
		default:
			(void)fprintf(stderr, "usage: %s %s\n", progname,
				      usage);
			anyfail();
		}
	}

	/*
	 *  Plan for death by signal.  User may have specified
	 *  a time limit, in which case set an alarm and catch SIGALRM.
	 *  Also catch and cleanup with SIGINT, SIGQUIT, and SIGTERM.
	 */
	sa.sa_handler = finish;
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask)) {
		perror("sigempty error");
		exit(1);
	}

	if (sigaction(SIGINT, &sa, 0) == -1) {
		perror("sigaction error SIGINT");
		exit(1);
	}
	if (alarmtime) {
		if (sigaction(SIGALRM, &sa, 0) == -1) {
			perror("sigaction error");
			exit(1);
		}
		(void)alarm(alarmtime);
		printf("mmap-corruption will run for=> %ld, seconds\n",
		       alarmtime);
	} else {		//Run for 5 secs only
		if (sigaction(SIGALRM, &sa, 0) == -1) {
			perror("sigaction error");
			exit(1);
		}
		(void)alarm(5);
		printf("mmap-corruption will run for=> 5, seconds\n");
	}
	/* If we get a SIGQUIT or SIGTERM, clean up and exit immediately. */
	sa.sa_handler = finish;
	if (sigaction(SIGQUIT, &sa, 0) == -1) {
		perror("sigaction error SIGQUIT");
		exit(1);
	}
	if (sigaction(SIGTERM, &sa, 0) == -1) {
		perror("sigaction error SIGTERM");
		exit(1);
	}

	tst_tmpdir();
	while (1) {
		unlink(fname);
		int fd = open(fname, O_CREAT | O_EXCL | O_RDWR, 0600);
		ftruncate(fd, kMemSize);

		mem =
		    mmap(0, kMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			 0);
		// Fill the memory with 1s.
		memset(mem, 1, kMemSize);

		for (i = 0; i < kMemSize; i++) {
			int byte_good = mem[i] != 0;
			if (!byte_good && ((i % kPageSize) == 0)) {
				//printf("%d ", i / kPageSize);
				count++;
			}
		}
		munmap(mem, kMemSize);
		close(fd);
		unlink(fname);
		if (count > 0) {
			printf("Running %d bad page\n", count);
			return 1;
		}
		count = 0;
	}
	return 0;
}

void finish(int sig)
{
	printf("mmap-corruption PASSED\n");
	exit(0);
}
