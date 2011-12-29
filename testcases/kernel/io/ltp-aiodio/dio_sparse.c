/*
 *   Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *                 2004 Open Source Development Lab
 *   
 *   Copyright (c) 2004 Marty Ridgeway <mridge@us.ibm.com>
 *   
 *   Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
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

static void setup(void);
static void cleanup(void);
static void usage(void);
static int debug = 0;

char *TCID="dio_sparse";
int TST_TOTAL=1;

#include "common_sparse.h"

/*
 * Write zeroes using O_DIRECT into sparse file.
 */
int dio_sparse(char *filename, int align, int writesize, int filesize)
{
	int fd;
	void *bufptr;
	int i, w;

	fd = open(filename, O_DIRECT|O_WRONLY|O_CREAT|O_EXCL, 0600);

	if (fd < 0) {
		tst_resm(TBROK|TERRNO, "open()");
		return 1;
	}

	SAFE_FTRUNCATE(cleanup, fd, filesize);

	if (posix_memalign(&bufptr, align, writesize)) {
		close(fd);
		tst_resm(TBROK|TERRNO, "posix_memalign()");
		return 1;
	}

	memset(bufptr, 0, writesize);
	for (i = 0; i < filesize;)  {
		if ((w = write(fd, bufptr, writesize)) != writesize) {
			tst_resm(TBROK|TERRNO, "write() returned %d", w);
			close(fd);
			return 1;
		}
		
		i += w;
	}

	close(fd);
	unlink(filename);

	return 0;
}

void usage(void)
{
	fprintf(stderr, "usage: dio_sparse [-d] [-n children] [-s filesize]"
		" [-w writesize]\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char *filename = "dio_sparse";
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;
	long alignment = 512;
	int writesize = 65536;
	int filesize = 100*1024*1024;
	int c;
	int children_errors = 0;
	int ret;

	while ((c = getopt(argc, argv, "dw:n:a:s:")) != -1) {
		char *endp;
		switch (c) {
		case 'd':
			debug++;
		break;
		case 'a':
			alignment = strtol(optarg, &endp, 0);
			alignment = scale_by_kmg(alignment, *endp);
		break;
		case 'w':
			writesize = strtol(optarg, &endp, 0);
			writesize = scale_by_kmg(writesize, *endp);
		break;
		case 's':
			filesize = strtol(optarg, &endp, 0);
			filesize = scale_by_kmg(filesize, *endp);
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
	tst_resm(TINFO, "Dirtying free blocks");
	dirty_freeblocks(filesize);

	tst_resm(TINFO, "Starting I/O tests");
	for (i = 0; i < num_children; i++) {
		switch (pid[i] = fork()) {
		case 0:
			signal(SIGTERM, SIG_DFL);
			read_sparse(filename, filesize);
		break;
		case -1:
			while (i-- > 0)
				kill(pid[i], SIGTERM);

			tst_brkm(TBROK|TERRNO, cleanup, "fork()");
		default:
			continue;
		}
	}
	
	ret = dio_sparse(filename, alignment, writesize, filesize);

	tst_resm(TINFO, "Killing childrens(s)");
	
	for (i = 0; i < num_children; i++)
		kill(pid[i], SIGTERM);

	for (i = 0; i < num_children; i++) {
		int status;
		pid_t p;

		p = waitpid(pid[i], &status, 0);
		if (p < 0) {
			tst_resm(TBROK|TERRNO, "waitpid()");
		} else {
			if (WIFEXITED(status) && WEXITSTATUS(status) == 10)
				children_errors++;
		}
	}

	if (children_errors)
		tst_resm(TFAIL, "%i children(s) exited abnormally",
		         children_errors);
		
	if (!children_errors && !ret)
		tst_resm(TPASS, "Test passed");
	
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}
