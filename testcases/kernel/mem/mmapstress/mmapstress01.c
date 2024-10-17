// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2003
 * 01/02/2003	Port to LTP avenkat@us.ibm.com
 * 06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * 10/03/2022	Refactor to LTP framework	edliaw@google.com
 */
/*\
 * This test stresses mmaps, without dealing with fragments or anything!
 * It forks a specified number of children,
 * all of whom mmap the same file, make a given number of accesses
 * to random pages in the map (reading & writing and comparing data).
 * Then the child exits and the parent forks another to take its place.
 * Each time a child is forked, it stats the file and maps the full
 * length of the file.
 *
 * This program continues to run until it either receives a SIGINT,
 * or times out (if a timeout value is specified).  When either of
 * these things happens, it cleans up its kids, then checks the
 * file to make sure it has the correct data.
 */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <float.h>
#include "tst_test.h"

#if _FILE_OFFSET_BITS == 64
# define FSIZE_MIN LONG_MIN
# define FSIZE_MAX LONG_MAX
#else
# define FSIZE_MIN INT_MIN
# define FSIZE_MAX INT_MAX
#endif
#define MAXLOOPS	500	/* max pages for map children to write */
#define TEST_FILE	"mmapstress01.out"

#ifdef roundup
#undef roundup
#endif
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))

static unsigned int initrand(void);
static void sighandler(int);

static char *debug;
static char *do_sync;
static char *do_offset;
static char *opt_filesize;
static char *opt_nprocs;
static char *opt_pattern;
static char *opt_sparseoffset;
static char *randloops;

static int fd;
static volatile int finished;
static int nprocs;
static long long filesize = 4096;
static long long sparseoffset;
static size_t pagesize;
static int pattern;

static void setup(void)
{
	struct sigaction sa;

	sa.sa_handler = sighandler;
	sa.sa_flags = 0;
	SAFE_SIGEMPTYSET(&sa.sa_mask);
	SAFE_SIGACTION(SIGINT, &sa, 0);
	SAFE_SIGACTION(SIGQUIT, &sa, 0);
	SAFE_SIGACTION(SIGTERM, &sa, 0);
	SAFE_SIGACTION(SIGALRM, &sa, 0);

	pagesize = sysconf(_SC_PAGE_SIZE);

	if (tst_parse_filesize(opt_filesize, &filesize, 0, FSIZE_MAX))
		tst_brk(TBROK, "invalid initial filesize '%s'", opt_filesize);

	if (tst_parse_filesize(opt_sparseoffset, &sparseoffset, FSIZE_MIN, FSIZE_MAX))
		tst_brk(TBROK, "invalid sparse offset '%s'", opt_sparseoffset);
	if (sparseoffset % pagesize != 0)
		tst_brk(TBROK, "sparseoffset must be pagesize multiple");

	if (tst_parse_int(opt_nprocs, &nprocs, 0, 255))
		tst_brk(TBROK, "invalid number of mapping children '%s'",
			opt_nprocs);
	if (!opt_nprocs)
		nprocs = MAX(MIN(tst_ncpus() - 1L, 20L), 1L);

	if (tst_parse_int(opt_pattern, &pattern, 0, 255))
		tst_brk(TBROK, "invalid pattern '%s'", opt_pattern);
	if (!opt_pattern)
		pattern = initrand() & 0xff;

	tst_res(TINFO, "creating file <%s> with %lld bytes, pattern %d",
		TEST_FILE, filesize, pattern);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

/*
 * Child process that reads/writes map.  The child stats the file
 * to determine the size, maps the size of the file, then reads/writes
 * its own locations on random pages of the map (its locations being
 * determined based on nprocs & procno).  After a specific number of
 * iterations, it exits.
 */
static void child_mapper(char *file, unsigned int procno, unsigned int nprocs)
{
	struct stat statbuf;
	off_t filesize;
	off_t offset;
	size_t validsize;
	size_t mapsize;
	char *maddr = NULL, *paddr;
	unsigned int randpage;
	unsigned int seed;
	unsigned int loopcnt;
	unsigned int nloops;
	unsigned int mappages;
	unsigned int i;

	seed = initrand();

	SAFE_STAT(file, &statbuf);
	filesize = statbuf.st_size;

	fd = SAFE_OPEN(file, O_RDWR);

	if (statbuf.st_size - sparseoffset > UINT_MAX)
		tst_brk(TBROK, "size_t overflow when setting up map");
	mapsize = (size_t) (statbuf.st_size - sparseoffset);
	mappages = roundup(mapsize, pagesize) / pagesize;
	offset = sparseoffset;
	if (do_offset) {
		int pageoffset = lrand48() % mappages;
		int byteoffset = pageoffset * pagesize;

		offset += byteoffset;
		mapsize -= byteoffset;
		mappages -= pageoffset;
	}
	nloops = (randloops) ? (lrand48() % MAXLOOPS) : MAXLOOPS;

	if (debug)
		tst_res(TINFO, "child %u (pid %d): seed %d, fsize %lld, mapsize %ld, off %lld, loop %d",
			procno, getpid(), seed, (long long)filesize,
			(long)mapsize, (long long)offset / pagesize, nloops);

	maddr = SAFE_MMAP(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			  offset);
	SAFE_CLOSE(fd);

	for (loopcnt = 0; loopcnt < nloops; loopcnt++) {
		randpage = lrand48() % mappages;
		paddr = maddr + (randpage * pagesize);	/* page address */

		if (randpage < mappages - 1 || !(mapsize % pagesize))
			validsize = pagesize;
		else
			validsize = mapsize % pagesize;

		for (i = procno; i < validsize; i += nprocs) {
			if (*((unsigned char *)(paddr + i))
			    != ((procno + pattern) & 0xff))
				tst_brk(TFAIL, "child %u: invalid data <x%x>\n"
					" at pg %d off %d, exp <x%x>", procno,
					*((unsigned char *)(paddr + i)),
					randpage, i, (procno + pattern) & 0xff);

			*(paddr + i) = (procno + pattern) & 0xff;
		}
	}

	if (do_sync) {
		randpage = lrand48() % mappages;
		paddr = maddr + (randpage * pagesize);	/* page address */
		SAFE_MSYNC(paddr, (mappages - randpage) * pagesize, MS_SYNC);
	}

	SAFE_MUNMAP(maddr, mapsize);
	exit(0);
}

/* Make sure file has all the correct data. */
static void fileokay(char *file, unsigned char *expbuf)
{
	int cnt;
	size_t mapsize;
	struct stat statbuf;
	unsigned char readbuf[pagesize];
	unsigned int i, j;
	unsigned int mappages;

	fd = SAFE_OPEN(file, O_RDONLY);

	SAFE_FSTAT(fd, &statbuf);
	SAFE_LSEEK(fd, sparseoffset, SEEK_SET);

	if (statbuf.st_size - sparseoffset > UINT_MAX)
		tst_brk(TBROK, "size_t overflow when setting up map");
	mapsize = (size_t) (statbuf.st_size - sparseoffset);

	mappages = roundup(mapsize, pagesize) / pagesize;

	for (i = 0; i < mappages; i++) {
		cnt = SAFE_READ(0, fd, readbuf, pagesize);
		if ((unsigned int)cnt != pagesize) {
			/* Okay if at last page in file... */
			if ((i * pagesize) + cnt != mapsize)
				tst_brk(TFAIL, "missing data: read %lu of %ld bytes",
					(i * pagesize) + cnt, (long)mapsize);
		}
		/* Compare read bytes of data. */
		for (j = 0; j < (unsigned int)cnt; j++) {
			if (expbuf[j] != readbuf[j])
				tst_brk(TFAIL,
					"read bad data: exp %c got %c, pg %d off %d, (fsize %lld)",
					expbuf[j], readbuf[j], i, j,
					(long long)statbuf.st_size);
		}
	}
	SAFE_CLOSE(fd);
}

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	finished++;
}

static unsigned int initrand(void)
{
	unsigned int seed;

	/*
	 * Use srand/rand to diffuse the information from the
	 * time and pid.  If you start several processes, then
	 * the time and pid information don't provide much
	 * variation.
	 */
	srand((unsigned int)getpid());
	seed = rand();
	srand((unsigned int)time(NULL));
	seed = (seed ^ rand()) % 100000;
	srand48((long)seed);
	return seed;
}

static void run(void)
{
	int c;
	int i;
	int wait_stat;
	off_t bytes_left;
	pid_t pid;
	pid_t *pidarray;
	size_t write_cnt;
	unsigned char data;
	unsigned char *buf;

	alarm(tst_remaining_runtime());

	finished = 0;
	fd = SAFE_OPEN(TEST_FILE, O_CREAT | O_TRUNC | O_RDWR, 0664);
	buf = SAFE_MALLOC(pagesize);
	pidarray = SAFE_MALLOC(nprocs * sizeof(pid_t));

	for (i = 0; i < nprocs; i++)
		*(pidarray + i) = 0;

	for (i = 0, data = 0; i < (int)pagesize; i++) {
		*(buf + i) = (data + pattern) & 0xff;
		if (++data == nprocs)
			data = 0;
	}
	SAFE_LSEEK(fd, (off_t)sparseoffset, SEEK_SET);
	for (bytes_left = filesize; bytes_left; bytes_left -= c) {
		write_cnt = MIN((long long)pagesize, (long long)bytes_left);
		c = SAFE_WRITE(1, fd, buf, write_cnt);
	}
	SAFE_CLOSE(fd);

	for (i = 0; i < nprocs; i++) {
		pid = SAFE_FORK();

		if (pid == 0) {
			child_mapper(TEST_FILE, (unsigned int)i, (unsigned int)nprocs);
			exit(0);
		} else {
			pidarray[i] = pid;
		}
	}

	while (!finished) {
		pid = wait(&wait_stat);
		if (pid != -1) {
			if (!WIFEXITED(wait_stat)
			    || WEXITSTATUS(wait_stat) != 0)
				tst_brk(TBROK, "child exit with err <x%x>",
					wait_stat);
			for (i = 0; i < nprocs; i++)
				if (pid == pidarray[i])
					break;
			if (i == nprocs)
				tst_brk(TBROK, "unknown child pid %d, <x%x>",
					pid, wait_stat);

			pid = SAFE_FORK();
			if (pid == 0) {
				child_mapper(TEST_FILE, (unsigned int)i, (unsigned int)nprocs);
				exit(0);
			} else {
				pidarray[i] = pid;
			}
		} else {
			if (errno != EINTR || !finished)
				tst_brk(TBROK | TERRNO,
					"unexpected wait error");
		}
	}
	alarm(0);

	fileokay(TEST_FILE, buf);
	tst_res(TPASS, "file has expected data");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.options = (struct tst_option[]) {
		{"d", &debug, "Enable debug output"},
		{"f:", &opt_filesize, "Initial filesize (default 4096)"},
		{"m", &do_sync, "Do random msync/fsyncs as well"},
		{"o", &do_offset, "Randomize the offset of file to map"},
		{"p:", &opt_nprocs,
		 "Number of mapping children to create (default 1 < ncpus < 20)"},
		{"P:", &opt_pattern,
		 "Use a fixed pattern (default random)"},
		{"r", &randloops,
		 "Randomize number of pages map children check (random % 500), "
		 "otherwise each child checks 500 pages"},
		{"S:", &opt_sparseoffset,
		 "When non-zero, causes the sparse area to be left before the data, "
		 "so that the actual initial filesize is sparseoffset + filesize "
		 "(default 0)"},
		{},
	},
	.cleanup = cleanup,
	.runtime = 12,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
