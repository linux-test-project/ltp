
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
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
 *
 * Module: .c
 */

/*
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <limits.h>
#include <getopt.h>

#include "test.h"
#include "safe_macros.h"

#define NUM_CHILDREN 1000

int debug;

const char *filename1=NULL;

static void setup(void);
static void cleanup(void);

char *TCID="dio_sparse";
int TST_TOTAL=1;

#include "common_sparse.h"

int read_sparse(char *filename, int filesize)
{
	int fd;
	int i;
	int j;
	int r;
	char buf[4096];

	while ((fd = open(filename, O_RDONLY)) < 0) {
		sleep(1);	/* wait for file to be created */
	}

	for (i = 0 ; i < 100000000; i++) {
		off_t offset = 0;
		char *badbuf;

		if (debug > 1 && (i % 10) == 0) {
			fprintf(stderr, "child %d, read loop count %d\n",
				getpid(), i);
		}
		lseek(fd, SEEK_SET, 0);
		for (j = 0; j < filesize+1; j += sizeof(buf)) {
			r = read(fd, buf, sizeof(buf));
			if (r > 0) {
				if ((badbuf = check_zero(buf, r))) {
					fprintf(stderr, "non-zero read at offset %d\n",
						offset + badbuf - buf);
					kill(getppid(), SIGTERM);
					exit(10);
				}
			}
			offset += r;
		}
	}
  return 0;
}

volatile int got_signal;

void sig_term_func(int i, siginfo_t *si, void *p)
{
	if (debug)
		fprintf(stderr, "sig(%d, %p, %p)\n", i, si, p);
	got_signal++;
}

/*
 * do DIO writes to a sparse file
 */
void dio_sparse(char *filename, int align, int writesize, int filesize)
{
	int fd;
	void *bufptr;
	int i;
	int w;
	static struct sigaction s;

	s.sa_sigaction = sig_term_func;
	s.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &s, 0);

	WITH_SIGNALS_BLOCKED(
		fd = open(filename, O_DIRECT|O_WRONLY|O_CREAT|O_EXCL, 0600);
		if (fd >= 0)
			filename1 = filename;
	);

	if (fd < 0) {
		perror("cannot create file");
		return;
	}

	SAFE_FTRUNCATE(cleanup, fd, filesize);

	if (posix_memalign(&bufptr, align, writesize)) {
		perror("cannot malloc aligned memory");
		close(fd);
		WITH_SIGNALS_BLOCKED(
			filename1=NULL;
			unlink(filename);
		);
		return;
	}

	memset(bufptr, 0, writesize);
	for (i = 0; i < filesize;)  {
		if (debug > 1 && ((i % writesize) % 100)) {
			fprintf(stderr, "DIO write # %d\n", i);
		}
		if ((w = write(fd, bufptr, writesize)) != writesize) {
			fprintf(stderr, "write %d returned %d\n", i, w);
			break;
		}
		i += w;
		if (got_signal) {
			if (debug)
				fprintf(stderr, "signal -- stop at %d\n", i);
			break;
		}
	}
	if (debug)
		fprintf(stderr, "DIO write done unlinking file\n");
	close(fd);
	WITH_SIGNALS_BLOCKED(
		filename1=NULL;
		unlink(filename);
	);
}


int usage(void)
{
	fprintf(stderr, "usage: dio_sparse [-n children] [-s filesize]"
		" [-w writesize] [-r readsize] \n");
	exit(1);
}

/*
 * Scale value by kilo, mega, or giga.
 */
long long scale_by_kmg(long long value, char scale)
{
	switch (scale) {
	case 'g':
	case 'G':
		value *= 1024;
	case 'm':
	case 'M':
		value *= 1024;
	case 'k':
	case 'K':
		value *= 1024;
		break;
	case '\0':
		break;
	default:
		usage();
		break;
	}
	return value;
}

/*
 *	usage: dio_sparse [-r readsize] [-w writesize] [-n chilren] [-a align]
 */
int main(int argc, char **argv)
{
	char *filename = "dio_sparse";
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;
	long alignment = 512;
	int readsize = 65536;
	int writesize = 65536;
	int filesize = 100*1024*1024;
	int c;
	int children_errors = 0;

	while ((c = getopt(argc, argv, "dr:w:n:a:s:")) != -1) {
		char *endp;
		switch (c) {
		case 'd':
			debug++;
			break;
		case 'a':
			alignment = strtol(optarg, &endp, 0);
			alignment = (int)scale_by_kmg((long long)alignment,
                                                        *endp);
			break;
		case 'r':
			readsize = strtol(optarg, &endp, 0);
			readsize = (int)scale_by_kmg((long long)readsize, *endp);
			break;
		case 'w':
			writesize = strtol(optarg, &endp, 0);
			writesize = (int)scale_by_kmg((long long)writesize, *endp);
			break;
		case 's':
			filesize = strtol(optarg, &endp, 0);
			filesize = (int)scale_by_kmg((long long)filesize, *endp);
			break;
		case 'n':
			num_children = atoi(optarg);
			if (num_children > NUM_CHILDREN) {
				fprintf(stderr,
					"number of children limited to %d\n",
					NUM_CHILDREN);
				num_children = NUM_CHILDREN;
			}
			break;
		case '?':
			usage();
			break;
		}
	}

	setup();
	dirty_freeblocks(filesize);

	for (i = 0; i < num_children; i++) {
		if ((pid[i] = fork()) == 0) {
			/* child */
			return read_sparse(filename, filesize);
		} else if (pid[i] < 0) {
			/* error */
			perror("fork error");
			break;
		} else {
			/* Parent */
			continue;
		}
	}

	/*
	 * Parent write to a hole in a file using direct i/o
	 */
	dio_sparse(filename, alignment, writesize, filesize);

	if (debug)
		fprintf(stderr, "dio_sparse done writing, kill children\n");
	for (i = 0; i < num_children; i++) {
		kill(pid[i], SIGTERM);
	}

	for (i = 0; i < num_children; i++) {
		int status;
		pid_t p;

		p = waitpid(pid[i], &status, 0);
		if (p < 0) {
			perror("waitpid");
		} else {
			if (WIFEXITED(status) && WEXITSTATUS(status) == 10) {
				children_errors++;
				if (debug) {
					fprintf(stderr, "child %d bad exit\n", p);
				}
			}
		}
	}
	if (debug)
		fprintf(stderr, "dio_sparse %d children had errors\n",
			children_errors);
	if (children_errors)
		exit(10);
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	signal(SIGTERM, SIG_DFL);
	tst_tmpdir();
}

static void cleanup(void)
{
	if (filename1)
		unlink(filename1);
	tst_rmdir();
}
