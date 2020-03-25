/*
 * Copyright (C) 1991, NeXT Computer, Inc.  All Rights Reserverd.
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 *	File:	fsx.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	File system exerciser.
 *
 *	Rewrite and enhancements 1998-2001 Conrad Minshall -- conrad@mac.com
 *
 *	Various features from Joe Sokol, Pat Dirks, and Clark Warner.
 *
 *	Small changes to work under Linux -- davej@suse.de
 *
 *	Sundry porting patches from Guy Harris 12/2001
 * $FreeBSD: src/tools/regression/fsx/fsx.c,v 1.1 2001/12/20 04:15:57 jkh Exp $
 *
 *	Add multi-file testing feature -- Zach Brown <zab@clusterfs.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#if defined(_UWIN) || defined(__linux__)
#include <sys/param.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

/*
 *	A log entry is an operation and a bunch of arguments.
 */

struct log_entry {
	int operation;
	struct timeval tv;
	int args[3];
};

#define	LOGSIZE	1000

struct log_entry oplog[LOGSIZE];	/* the log */
int logptr = 0;			/* current position in log */
int logcount = 0;		/* total ops */

/*
 *	Define operations
 */

#define	OP_READ		1
#define OP_WRITE	2
#define OP_TRUNCATE	3
#define OP_CLOSEOPEN	4
#define OP_MAPREAD	5
#define OP_MAPWRITE	6
#define OP_SKIPPED	7

int page_size;
int page_mask;

char *original_buf;		/* a pointer to the original data */
char *good_buf;			/* a pointer to the correct data */
char *temp_buf;			/* a pointer to the current data */
char *fname;			/* name of our test file */
char logfile[1024];		/* name of our log file */
char goodfile[1024];		/* name of our test file */

off_t file_size = 0;
off_t biggest = 0;
char state[256];
unsigned long testcalls = 0;	/* calls to function "test" */

unsigned long simulatedopcount = 0;	/* -b flag */
int closeprob = 0;		/* -c flag */
int debug = 0;			/* -d flag */
unsigned long debugstart = 0;	/* -D flag */
unsigned long maxfilelen = 256 * 1024;	/* -l flag */
int sizechecks = 1;		/* -n flag disables them */
int maxoplen = 64 * 1024;	/* -o flag */
int quiet = 0;			/* -q flag */
unsigned long progressinterval = 0;	/* -p flag */
int readbdy = 1;		/* -r flag */
int style = 0;			/* -s flag */
int truncbdy = 1;		/* -t flag */
int writebdy = 1;		/* -w flag */
long monitorstart = -1;		/* -m flag */
long monitorend = -1;		/* -m flag */
int lite = 0;			/* -L flag */
long numops = -1;		/* -N flag */
int randomoplen = 1;		/* -O flag disables it */
int seed = 1;			/* -S flag */
int mapped_writes = 1;		/* -W flag disables */
int mapped_reads = 1;		/* -R flag disables it */
int fsxgoodfd = 0;
FILE *fsxlogf = NULL;
int badoff = -1;

void vwarnc(int code,const char *fmt, va_list ap)
{
	fprintf(stderr, "fsx: ");
	if (fmt != NULL) {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(code));
}

void warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnc(errno, fmt, ap);
	va_end(ap);
}

void
    __attribute__ ((format(printf, 1, 2)))
    prt(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);

	if (fsxlogf) {
		va_start(args, fmt);
		vfprintf(fsxlogf, fmt, args);
		va_end(args);
	}
}

void prterr(char *prefix)
{
	prt("%s%s%s\n", prefix, prefix ? ": " : "", strerror(errno));
}

void log4(int operation, int arg0, int arg1, int arg2, struct timeval *tv)
{
	struct log_entry *le;

	le = &oplog[logptr];
	le->tv = *tv;
	le->operation = operation;
	le->args[0] = arg0;
	le->args[1] = arg1;
	le->args[2] = arg2;
	logptr++;
	logcount++;
	if (logptr >= LOGSIZE)
		logptr = 0;
}

void logdump(void)
{
	int i, count, down;
	struct log_entry *lp;

	prt("LOG DUMP (%d total operations):\n", logcount);
	if (logcount < LOGSIZE) {
		i = 0;
		count = logcount;
	} else {
		i = logptr;
		count = LOGSIZE;
	}
	for (; count > 0; count--) {
		int opnum;

		opnum = i + 1 + (logcount / LOGSIZE) * LOGSIZE;
		lp = &oplog[i];
		prt("%d: %lu.%06lu ", opnum, lp->tv.tv_sec, lp->tv.tv_usec);

		switch (lp->operation) {
		case OP_MAPREAD:
			prt("MAPREAD  0x%x thru 0x%x (0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (badoff >= lp->args[0] && badoff <
			    lp->args[0] + lp->args[1])
				prt("\t***RRRR***");
			break;
		case OP_MAPWRITE:
			prt("MAPWRITE 0x%x thru 0x%x (0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (badoff >= lp->args[0] && badoff <
			    lp->args[0] + lp->args[1])
				prt("\t******WWWW");
			break;
		case OP_READ:
			prt("READ     0x%x thru 0x%x (0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (badoff >= lp->args[0] &&
			    badoff < lp->args[0] + lp->args[1])
				prt("\t***RRRR***");
			break;
		case OP_WRITE:
			prt("WRITE    0x%x thru 0x%x (0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (lp->args[0] > lp->args[2])
				prt(" HOLE");
			else if (lp->args[0] + lp->args[1] > lp->args[2])
				prt(" EXTEND");
			if ((badoff >= lp->args[0] || badoff >= lp->args[2]) &&
			    badoff < lp->args[0] + lp->args[1])
				prt("\t***WWWW");
			break;
		case OP_TRUNCATE:
			down = lp->args[0] < lp->args[1];
			prt("TRUNCATE %s\tfrom 0x%x to 0x%x",
			    down ? "DOWN" : "UP", lp->args[1], lp->args[0]);
			if (badoff >= lp->args[!down] &&
			    badoff < lp->args[! !down])
				prt("\t******WWWW");
			break;
		case OP_CLOSEOPEN:
			prt("CLOSE/OPEN");
			break;
		case OP_SKIPPED:
			prt("SKIPPED (no operation)");
			break;
		default:
			prt("BOGUS LOG ENTRY (operation code = %d)!",
			    lp->operation);
		}
		prt("\n");
		i++;
		if (i == LOGSIZE)
			i = 0;
	}
}

void save_buffer(char *buffer, off_t bufferlength, int fd)
{
	off_t ret;
	ssize_t byteswritten;

	if (fd <= 0 || bufferlength == 0)
		return;

	if (bufferlength > INT_MAX) {
		prt("fsx flaw: overflow in save_buffer\n");
		exit(67);
	}
	if (lite) {
		off_t size_by_seek = lseek(fd, (off_t) 0, SEEK_END);
		if (size_by_seek == (off_t) - 1)
			prterr("save_buffer: lseek eof");
		else if (bufferlength > size_by_seek) {
			warn("save_buffer: .fsxgood file too short... will"
			     "save 0x%llx bytes instead of 0x%llx\n",
			     (unsigned long long)size_by_seek,
			     (unsigned long long)bufferlength);
			bufferlength = size_by_seek;
		}
	}

	ret = lseek(fd, (off_t) 0, SEEK_SET);
	if (ret == (off_t) - 1)
		prterr("save_buffer: lseek 0");

	byteswritten = write(fd, buffer, (size_t) bufferlength);
	if (byteswritten != bufferlength) {
		if (byteswritten == -1)
			prterr("save_buffer write");
		else
			warn("save_buffer: short write, 0x%x bytes instead"
			     "of 0x%llx\n",
			     (unsigned)byteswritten,
			     (unsigned long long)bufferlength);
	}
}

void report_failure(int status)
{
	logdump();

	if (fsxgoodfd) {
		if (good_buf) {
			save_buffer(good_buf, file_size, fsxgoodfd);
			prt("Correct content saved for comparison\n");
			prt("(maybe hexdump \"%s\" vs \"%s\")\n",
			    fname, goodfile);
		}
		close(fsxgoodfd);
	}
	exit(status);
}

#define short_at(cp) ((unsigned short)((*((unsigned char *)(cp)) << 8) | \
				        *(((unsigned char *)(cp)) + 1)))

void check_buffers(unsigned offset, unsigned size)
{
	unsigned char c, t;
	unsigned i = 0;
	unsigned n = 0;
	unsigned op = 0;
	unsigned bad = 0;

	if (memcmp(good_buf + offset, temp_buf, size) != 0) {
		prt("READ BAD DATA: offset = 0x%x, size = 0x%x\n",
		    offset, size);
		prt("OFFSET\tGOOD\tBAD\tRANGE\n");
		while (size > 0) {
			c = good_buf[offset];
			t = temp_buf[i];
			if (c != t) {
				if (n == 0) {
					bad = short_at(&temp_buf[i]);
					prt("%#07x\t%#06x\t%#06x", offset,
					    short_at(&good_buf[offset]), bad);
					op = temp_buf[offset & 1 ? i + 1 : i];
				}
				n++;
				badoff = offset;
			}
			offset++;
			i++;
			size--;
		}
		if (n) {
			prt("\t%#7x\n", n);
			if (bad)
				prt("operation# (mod 256) for the bad data"
				    "may be %u\n", ((unsigned)op & 0xff));
			else
				prt("operation# (mod 256) for the bad data"
				    "unknown, check HOLE and EXTEND ops\n");
		} else
			prt("????????????????\n");
		report_failure(110);
	}
}

struct test_file {
	char *path;
	int fd;
} *test_files = NULL;

int num_test_files = 0;
enum fd_iteration_policy {
	FD_SINGLE,
	FD_ROTATE,
	FD_RANDOM,
};
int fd_policy = FD_RANDOM;
int fd_last = 0;

struct test_file *get_tf(void)
{
	unsigned index = 0;

	switch (fd_policy) {
	case FD_ROTATE:
		index = fd_last++;
		break;
	case FD_RANDOM:
		index = random();
		break;
	case FD_SINGLE:
		index = 0;
		break;
	default:
		prt("unknown policy");
		exit(1);
		break;
	}
	return &test_files[index % num_test_files];
}

void assign_fd_policy(char *policy)
{
	if (!strcmp(policy, "random"))
		fd_policy = FD_RANDOM;
	else if (!strcmp(policy, "rotate"))
		fd_policy = FD_ROTATE;
	else {
		prt("unknown -I policy: '%s'\n", policy);
		exit(1);
	}
}

int get_fd(void)
{
	struct test_file *tf = get_tf();
	return tf->fd;
}

void open_test_files(char **argv, int argc)
{
	struct test_file *tf;
	int i;

	num_test_files = argc;
	if (num_test_files == 1)
		fd_policy = FD_SINGLE;

	test_files = calloc(num_test_files, sizeof(*test_files));
	if (test_files == NULL) {
		prterr("reallocating space for test files");
		exit(1);
	}

	for (i = 0, tf = test_files; i < num_test_files; i++, tf++) {

		tf->path = argv[i];
		tf->fd = open(tf->path, O_RDWR | (lite ? 0 : O_CREAT | O_TRUNC),
			      0666);
		if (tf->fd < 0) {
			prterr(tf->path);
			exit(91);
		}
	}

	if (quiet || fd_policy == FD_SINGLE)
		return;

	for (i = 0, tf = test_files; i < num_test_files; i++, tf++)
		prt("fd %d: %s\n", i, tf->path);
}

void close_test_files(void)
{
	int i;
	struct test_file *tf;

	for (i = 0, tf = test_files; i < num_test_files; i++, tf++) {
		if (close(tf->fd)) {
			prterr("close");
			report_failure(99);
		}
	}
}

void check_size(void)
{
	struct stat statbuf;
	off_t size_by_seek;
	int fd = get_fd();

	if (fstat(fd, &statbuf)) {
		prterr("check_size: fstat");
		statbuf.st_size = -1;
	}
	size_by_seek = lseek(fd, (off_t) 0, SEEK_END);
	if (file_size != statbuf.st_size || file_size != size_by_seek) {
		prt("Size error: expected 0x%llx stat 0x%llx seek 0x%llx\n",
		    (unsigned long long)file_size,
		    (unsigned long long)statbuf.st_size,
		    (unsigned long long)size_by_seek);
		report_failure(120);
	}
}

void check_trunc_hack(void)
{
	struct stat statbuf;
	int fd = get_fd();

	ftruncate(fd, (off_t) 0);
	ftruncate(fd, (off_t) 100000);
	if (fstat(fd, &statbuf)) {
		prterr("trunc_hack: fstat");
		statbuf.st_size = -1;
	}
	if (statbuf.st_size != (off_t) 100000) {
		prt("no extend on truncate! not posix!\n");
		exit(130);
	}
	ftruncate(fd, 0);
}

static char *tf_buf = NULL;
static int max_tf_len = 0;

void alloc_tf_buf(void)
{
	char dummy = '\0';
	int highest = num_test_files - 1;
	int len;

	len = snprintf(&dummy, 0, "%u ", highest);
	if (len < 1) {
		prterr("finding max tf_buf");
		exit(1);
	}
	len++;
	tf_buf = malloc(len);
	if (tf_buf == NULL) {
		prterr("allocating tf_buf");
		exit(1);
	}
	max_tf_len = snprintf(tf_buf, len, "%u ", highest);
	if (max_tf_len < 1) {
		prterr("fiding max_tv_len\n");
		exit(1);
	}
	if (max_tf_len != len - 1) {
		warn("snprintf() gave %d instead of %d?\n",
		     max_tf_len, len - 1);
		exit(1);
	}
}

char *fill_tf_buf(struct test_file *tf)
{
	if (tf_buf == NULL)
		alloc_tf_buf();

	sprintf(tf_buf, "%lu ", (unsigned long)(tf - test_files));
	return tf_buf;
}

void
output_line(struct test_file *tf, int op, unsigned offset,
	    unsigned size, struct timeval *tv)
{
	char *tf_num = "";

	char *ops[] = {
		[OP_READ] = "read",
		[OP_WRITE] = "write",
		[OP_TRUNCATE] = "trunc from",
		[OP_MAPREAD] = "mapread",
		[OP_MAPWRITE] = "mapwrite",
	};

	/* W. */
	if (!(!quiet && ((progressinterval &&
			  testcalls % progressinterval == 0) ||
			 (debug &&
			  (monitorstart == -1 ||
			   (offset + size > monitorstart &&
			    (monitorend == -1 || offset <= monitorend)))))))
		return;

	if (fd_policy != FD_SINGLE)
		tf_num = fill_tf_buf(tf);

	prt("%06lu %lu.%06lu %.*s%-10s %#08x %s %#08x\t(0x%x bytes)\n",
	    testcalls, tv->tv_sec, tv->tv_usec, max_tf_len,
	    tf_num, ops[op],
	    offset, op == OP_TRUNCATE ? " to " : "thru",
	    offset + size - 1, size);
}

void doread(unsigned offset, unsigned size)
{
	struct timeval t;
	off_t ret;
	unsigned iret;
	struct test_file *tf = get_tf();
	int fd = tf->fd;

	offset -= offset % readbdy;
	gettimeofday(&t, NULL);
	if (size == 0) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping zero size read\n");
		log4(OP_SKIPPED, OP_READ, offset, size, &t);
		return;
	}
	if (size + offset > file_size) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping seek/read past end of file\n");
		log4(OP_SKIPPED, OP_READ, offset, size, &t);
		return;
	}

	log4(OP_READ, offset, size, 0, &t);

	if (testcalls <= simulatedopcount)
		return;

	output_line(tf, OP_READ, offset, size, &t);

	ret = lseek(fd, (off_t) offset, SEEK_SET);
	if (ret == (off_t) - 1) {
		prterr("doread: lseek");
		report_failure(140);
	}
	iret = read(fd, temp_buf, size);
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu read done\n", t.tv_sec, t.tv_usec);
	}
	if (iret != size) {
		if (iret == -1)
			prterr("doread: read");
		else
			prt("short read: 0x%x bytes instead of 0x%x\n",
			    iret, size);
		report_failure(141);
	}
	check_buffers(offset, size);
}

void domapread(unsigned offset, unsigned size)
{
	struct timeval t;
	unsigned pg_offset;
	unsigned map_size;
	char *p;
	struct test_file *tf = get_tf();
	int fd = tf->fd;

	offset -= offset % readbdy;
	gettimeofday(&t, NULL);
	if (size == 0) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping zero size read\n");
		log4(OP_SKIPPED, OP_MAPREAD, offset, size, &t);
		return;
	}
	if (size + offset > file_size) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping seek/read past end of file\n");
		log4(OP_SKIPPED, OP_MAPREAD, offset, size, &t);
		return;
	}

	log4(OP_MAPREAD, offset, size, 0, &t);

	if (testcalls <= simulatedopcount)
		return;

	output_line(tf, OP_MAPREAD, offset, size, &t);

	pg_offset = offset & page_mask;
	map_size = pg_offset + size;

	if ((p = mmap(0, map_size, PROT_READ, MAP_FILE | MAP_SHARED, fd,
		      (off_t) (offset - pg_offset))) == MAP_FAILED) {
		prterr("domapread: mmap");
		report_failure(190);
	}
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu mmap done\n", t.tv_sec, t.tv_usec);
	}
	memcpy(temp_buf, p + pg_offset, size);
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu memcpy done\n", t.tv_sec, t.tv_usec);
	}
	if (munmap(p, map_size) != 0) {
		prterr("domapread: munmap");
		report_failure(191);
	}
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu munmap done\n", t.tv_sec, t.tv_usec);
	}

	check_buffers(offset, size);
}

void gendata(char *original_buf, char *good_buf, unsigned offset, unsigned size)
{
	while (size--) {
		good_buf[offset] = testcalls % 256;
		if (offset % 2)
			good_buf[offset] += original_buf[offset];
		offset++;
	}
}

void dowrite(unsigned offset, unsigned size)
{
	struct timeval t;
	off_t ret;
	unsigned iret;
	struct test_file *tf = get_tf();
	int fd = tf->fd;

	offset -= offset % writebdy;
	gettimeofday(&t, NULL);
	if (size == 0) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping zero size write\n");
		log4(OP_SKIPPED, OP_WRITE, offset, size, &t);
		return;
	}

	log4(OP_WRITE, offset, size, file_size, &t);

	gendata(original_buf, good_buf, offset, size);
	if (file_size < offset + size) {
		if (file_size < offset)
			memset(good_buf + file_size, '\0', offset - file_size);
		file_size = offset + size;
		if (lite) {
			warn("Lite file size bug in fsx!");
			report_failure(149);
		}
	}

	if (testcalls <= simulatedopcount)
		return;

	output_line(tf, OP_WRITE, offset, size, &t);

	ret = lseek(fd, (off_t) offset, SEEK_SET);
	if (ret == (off_t) - 1) {
		prterr("dowrite: lseek");
		report_failure(150);
	}
	iret = write(fd, good_buf + offset, size);
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu write done\n", t.tv_sec, t.tv_usec);
	}
	if (iret != size) {
		if (iret == -1)
			prterr("dowrite: write");
		else
			prt("short write: 0x%x bytes instead of 0x%x\n",
			    iret, size);
		report_failure(151);
	}
}

void domapwrite(unsigned offset, unsigned size)
{
	struct timeval t;
	unsigned pg_offset;
	unsigned map_size;
	off_t cur_filesize;
	char *p;
	struct test_file *tf = get_tf();
	int fd = tf->fd;

	offset -= offset % writebdy;
	gettimeofday(&t, NULL);
	if (size == 0) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping zero size write\n");
		log4(OP_SKIPPED, OP_MAPWRITE, offset, size, &t);
		return;
	}
	cur_filesize = file_size;

	log4(OP_MAPWRITE, offset, size, 0, &t);

	gendata(original_buf, good_buf, offset, size);
	if (file_size < offset + size) {
		if (file_size < offset)
			memset(good_buf + file_size, '\0', offset - file_size);
		file_size = offset + size;
		if (lite) {
			warn("Lite file size bug in fsx!");
			report_failure(200);
		}
	}

	if (testcalls <= simulatedopcount)
		return;

	output_line(tf, OP_MAPWRITE, offset, size, &t);

	if (file_size > cur_filesize) {
		if (ftruncate(fd, file_size) == -1) {
			prterr("domapwrite: ftruncate");
			exit(201);
		}
		if (!quiet && (debug > 1 &&
			       (monitorstart == -1 ||
				(offset + size > monitorstart &&
				 (monitorend == -1
				  || offset <= monitorend))))) {
			gettimeofday(&t, NULL);
			prt("       %lu.%06lu truncate done\n", t.tv_sec,
			    t.tv_usec);
		}
	}
	pg_offset = offset & page_mask;
	map_size = pg_offset + size;

	if ((p =
	     mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
		  fd, (off_t) (offset - pg_offset))) == MAP_FAILED) {
		prterr("domapwrite: mmap");
		report_failure(202);
	}
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu mmap done\n", t.tv_sec, t.tv_usec);
	}
	memcpy(p + pg_offset, good_buf + offset, size);
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu memcpy done\n", t.tv_sec, t.tv_usec);
	}
	if (msync(p, map_size, 0) != 0) {
		prterr("domapwrite: msync");
		report_failure(203);
	}
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu msync done\n", t.tv_sec, t.tv_usec);
	}
	if (munmap(p, map_size) != 0) {
		prterr("domapwrite: munmap");
		report_failure(204);
	}
	if (!quiet && (debug > 1 &&
		       (monitorstart == -1 ||
			(offset + size > monitorstart &&
			 (monitorend == -1 || offset <= monitorend))))) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu munmap done\n", t.tv_sec, t.tv_usec);
	}
}

void dotruncate(unsigned size)
{
	struct timeval t;
	int oldsize = file_size;
	struct test_file *tf = get_tf();
	int fd = tf->fd;

	size -= size % truncbdy;
	gettimeofday(&t, NULL);
	if (size > biggest) {
		biggest = size;
		if (!quiet && testcalls > simulatedopcount)
			prt("truncating to largest ever: 0x%x\n", size);
	}

	log4(OP_TRUNCATE, size, (unsigned)file_size, 0, &t);

	if (size > file_size)
		memset(good_buf + file_size, '\0', size - file_size);
	file_size = size;

	if (testcalls <= simulatedopcount)
		return;

	output_line(tf, OP_TRUNCATE, oldsize, size, &t);

	if (ftruncate(fd, (off_t) size) == -1) {
		prt("ftruncate1: %x\n", size);
		prterr("dotruncate: ftruncate");
		report_failure(160);
	}
	if (!quiet && debug > 1) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu trunc done\n", t.tv_sec, t.tv_usec);
	}
}

void writefileimage(void)
{
	ssize_t iret;
	int fd = get_fd();

	if (lseek(fd, (off_t) 0, SEEK_SET) == (off_t) - 1) {
		prterr("writefileimage: lseek");
		report_failure(171);
	}
	iret = write(fd, good_buf, file_size);
	if ((off_t) iret != file_size) {
		if (iret == -1)
			prterr("writefileimage: write");
		else
			prt("short write: 0x%lx bytes instead of 0x%llx\n",
			    (unsigned long)iret, (unsigned long long)file_size);
		report_failure(172);
	}
	if (lite ? 0 : ftruncate(fd, file_size) == -1) {
		prt("ftruncate2: %llx\n", (unsigned long long)file_size);
		prterr("writefileimage: ftruncate");
		report_failure(173);
	}
}

void docloseopen(void)
{
	struct timeval t;
	struct test_file *tf = get_tf();

	if (testcalls <= simulatedopcount)
		return;

	gettimeofday(&t, NULL);
	log4(OP_CLOSEOPEN, file_size, (unsigned)file_size, 0, &t);

	if (debug)
		prt("%06lu %lu.%06lu close/open\n", testcalls, t.tv_sec,
		    t.tv_usec);
	if (close(tf->fd)) {
		prterr("docloseopen: close");
		report_failure(180);
	}
	if (!quiet && debug > 1) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu close done\n", t.tv_sec, t.tv_usec);
	}
	tf->fd = open(tf->path, O_RDWR, 0);
	if (tf->fd < 0) {
		prterr("docloseopen: open");
		report_failure(181);
	}
	if (!quiet && debug > 1) {
		gettimeofday(&t, NULL);
		prt("       %lu.%06lu open done\n", t.tv_sec, t.tv_usec);
	}
}

void test(void)
{
	unsigned long offset;
	unsigned long size = maxoplen;
	unsigned long rv = random();
	unsigned long op = rv % (3 + !lite + mapped_writes);

	/* turn off the map read if necessary */

	if (op == 2 && !mapped_reads)
		op = 0;

	if (simulatedopcount > 0 && testcalls == simulatedopcount)
		writefileimage();

	testcalls++;

	if (debugstart > 0 && testcalls >= debugstart)
		debug = 1;

	if (!quiet && testcalls < simulatedopcount && testcalls % 100000 == 0)
		prt("%lu...\n", testcalls);

	/*
	 * READ:        op = 0
	 * WRITE:       op = 1
	 * MAPREAD:     op = 2
	 * TRUNCATE:    op = 3
	 * MAPWRITE:    op = 3 or 4
	 */
	if (lite ? 0 : op == 3 && (style & 1) == 0)	/* vanilla truncate? */
		dotruncate(random() % maxfilelen);
	else {
		if (randomoplen)
			size = random() % (maxoplen + 1);
		if (lite ? 0 : op == 3)
			dotruncate(size);
		else {
			offset = random();
			if (op == 1 || op == (lite ? 3 : 4)) {
				offset %= maxfilelen;
				if (offset + size > maxfilelen)
					size = maxfilelen - offset;
				if (op != 1)
					domapwrite(offset, size);
				else
					dowrite(offset, size);
			} else {
				if (file_size)
					offset %= file_size;
				else
					offset = 0;
				if (offset + size > file_size)
					size = file_size - offset;
				if (op != 0)
					domapread(offset, size);
				else
					doread(offset, size);
			}
		}
	}
	if (sizechecks && testcalls > simulatedopcount)
		check_size();
	if (closeprob && (rv >> 3) < (1 << 28) / closeprob)
		docloseopen();
}

void cleanup(int sig)
{
	if (sig)
		prt("signal %d\n", sig);
	prt("testcalls = %lu\n", testcalls);
	exit(sig);
}

void usage(void)
{
	fprintf(stdout, "usage: %s",
		"fsx [-dnqLOW] [-b opnum] [-c Prob] [-l flen] [-m "
		"start:end] [-o oplen] [-p progressinterval] [-r readbdy] [-s style] [-t "
		"truncbdy] [-w writebdy] [-D startingop] [-N numops] [-P dirpath] [-S seed] "
		"[ -I random|rotate ] fname [additional paths to fname..]\n"
		"	-b opnum: beginning operation number (default 1)\n"
		"	-c P: 1 in P chance of file close+open at each op (default infinity)\n"
		"	-d: debug output for all operations [-d -d = more debugging]\n"
		"	-l flen: the upper bound on file size (default 262144)\n"
		"	-m start:end: monitor (print debug) specified byte range (default 0:infinity)\n"
		"	-n: no verifications of file size\n"
		"	-o oplen: the upper bound on operation size (default 65536)\n"
		"	-p progressinterval: debug output at specified operation interval\n"
		"	-q: quieter operation\n"
		"	-r readbdy: 4096 would make reads page aligned (default 1)\n"
		"	-s style: 1 gives smaller truncates (default 0)\n"
		"	-t truncbdy: 4096 would make truncates page aligned (default 1)\n"
		"	-w writebdy: 4096 would make writes page aligned (default 1)\n"
		"	-D startingop: debug output starting at specified operation\n"
		"	-L: fsxLite - no file creations & no file size changes\n"
		"	-N numops: total # operations to do (default infinity)\n"
		"	-O: use oplen (see -o flag) for every op (default random)\n"
		"	-P: save .fsxlog and .fsxgood files in dirpath (default ./)\n"
		"	-S seed: for random # generator (default 1) 0 gets timestamp\n"
		"	-W: mapped write operations DISabled\n"
		"	-R: read() system calls only (mapped reads disabled)\n"
		"	-I: When multiple paths to the file are given each operation uses\n"
		"	    a different path.  Iterate through them in order with 'rotate'\n"
		"	    or chose then at 'random'.  (defaults to random)\n"
		"	fname: this filename is REQUIRED (no default)\n");
	exit(90);
}

int getnum(char *s, char **e)
{
	int ret = -1;

	*e = NULL;
	ret = strtol(s, e, 0);
	if (*e)
		switch (**e) {
		case 'b':
		case 'B':
			ret *= 512;
			*e = *e + 1;
			break;
		case 'k':
		case 'K':
			ret *= 1024;
			*e = *e + 1;
			break;
		case 'm':
		case 'M':
			ret *= 1024 * 1024;
			*e = *e + 1;
			break;
		case 'w':
		case 'W':
			ret *= 4;
			*e = *e + 1;
			break;
		}
	return (ret);
}

int main(int argc, char **argv)
{
	int i, style, ch;
	char *endp;
	int dirpath = 0;

	goodfile[0] = 0;
	logfile[0] = 0;

	page_size = getpagesize();
	page_mask = page_size - 1;

	setvbuf(stdout, NULL, _IOLBF, 0);	/* line buffered stdout */

	while ((ch = getopt(argc, argv,
			    "b:c:dl:m:no:p:qr:s:t:w:D:I:LN:OP:RS:W"))
	       != EOF)
		switch (ch) {
		case 'b':
			simulatedopcount = getnum(optarg, &endp);
			if (!quiet)
				fprintf(stdout, "Will begin at operation"
					"%ld\n", simulatedopcount);
			if (simulatedopcount == 0)
				usage();
			simulatedopcount -= 1;
			break;
		case 'c':
			closeprob = getnum(optarg, &endp);
			if (!quiet)
				fprintf(stdout,
					"Chance of close/open is 1 in %d\n",
					closeprob);
			if (closeprob <= 0)
				usage();
			break;
		case 'd':
			debug++;
			break;
		case 'l':
			maxfilelen = getnum(optarg, &endp);
			if (maxfilelen <= 0)
				usage();
			break;
		case 'm':
			monitorstart = getnum(optarg, &endp);
			if (monitorstart < 0)
				usage();
			if (!endp || *endp++ != ':')
				usage();
			monitorend = getnum(endp, &endp);
			if (monitorend < 0)
				usage();
			if (monitorend == 0)
				monitorend = -1;	/* aka infinity */
			debug = 1;
		case 'n':
			sizechecks = 0;
			break;
		case 'o':
			maxoplen = getnum(optarg, &endp);
			if (maxoplen <= 0)
				usage();
			break;
		case 'p':
			progressinterval = getnum(optarg, &endp);
			if (progressinterval < 0)
				usage();
			break;
		case 'q':
			quiet = 1;
			break;
		case 'r':
			readbdy = getnum(optarg, &endp);
			if (readbdy <= 0)
				usage();
			break;
		case 's':
			style = getnum(optarg, &endp);
			if (style < 0 || style > 1)
				usage();
			break;
		case 't':
			truncbdy = getnum(optarg, &endp);
			if (truncbdy <= 0)
				usage();
			break;
		case 'w':
			writebdy = getnum(optarg, &endp);
			if (writebdy <= 0)
				usage();
			break;
		case 'D':
			debugstart = getnum(optarg, &endp);
			if (debugstart < 1)
				usage();
			break;
		case 'I':
			assign_fd_policy(optarg);
			break;
		case 'L':
			lite = 1;
			break;
		case 'N':
			numops = getnum(optarg, &endp);
			if (numops < 0)
				usage();
			break;
		case 'O':
			randomoplen = 0;
			break;
		case 'P':
			strncpy(goodfile, optarg, sizeof(goodfile));
			strcat(goodfile, "/");
			strncpy(logfile, optarg, sizeof(logfile));
			strcat(logfile, "/");
			dirpath = 1;
			break;
		case 'R':
			mapped_reads = 0;
			break;
		case 'S':
			seed = getnum(optarg, &endp);
			if (seed == 0)
				seed = time(0) % 10000;
			if (!quiet)
				fprintf(stdout, "Seed set to %d\n", seed);
			if (seed < 0)
				usage();
			break;
		case 'W':
			mapped_writes = 0;
			if (!quiet)
				fprintf(stdout, "mapped writes DISABLED\n");
			break;

		default:
			usage();

		}
	argc -= optind;
	argv += optind;
	if (argc < 1)
		usage();
	fname = argv[0];

	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGPIPE, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGXCPU, cleanup);
	signal(SIGXFSZ, cleanup);
	signal(SIGVTALRM, cleanup);
	signal(SIGUSR1, cleanup);
	signal(SIGUSR2, cleanup);

	initstate(seed, state, 256);
	setstate(state);

	open_test_files(argv, argc);

	strncat(goodfile, dirpath ? basename(fname) : fname, 256);
	strcat(goodfile, ".fsxgood");
	fsxgoodfd = open(goodfile, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fsxgoodfd < 0) {
		prterr(goodfile);
		exit(92);
	}
	strncat(logfile, dirpath ? basename(fname) : fname, 256);
	strcat(logfile, ".fsxlog");
	fsxlogf = fopen(logfile, "w");
	if (fsxlogf == NULL) {
		prterr(logfile);
		exit(93);
	}
	if (lite) {
		off_t ret;
		int fd = get_fd();
		file_size = maxfilelen = lseek(fd, (off_t) 0, SEEK_END);
		if (file_size == (off_t) - 1) {
			prterr(fname);
			warn("main: lseek eof");
			exit(94);
		}
		ret = lseek(fd, (off_t) 0, SEEK_SET);
		if (ret == (off_t) - 1) {
			prterr(fname);
			warn("main: lseek 0");
			exit(95);
		}
	}
	original_buf = malloc(maxfilelen);
	if (original_buf == NULL)
		exit(96);
	for (i = 0; i < maxfilelen; i++)
		original_buf[i] = random() % 256;

	good_buf = malloc(maxfilelen);
	if (good_buf == NULL)
		exit(97);
	memset(good_buf, '\0', maxfilelen);

	temp_buf = malloc(maxoplen);
	if (temp_buf == NULL)
		exit(99);
	memset(temp_buf, '\0', maxoplen);

	if (lite) {		/* zero entire existing file */
		ssize_t written;
		int fd = get_fd();

		written = write(fd, good_buf, (size_t) maxfilelen);
		if (written != maxfilelen) {
			if (written == -1) {
				prterr(fname);
				warn("main: error on write");
			} else
				warn("main: short write, 0x%x bytes instead"
				     "of 0x%x\n",
				     (unsigned)written, maxfilelen);
			exit(98);
		}
	} else
		check_trunc_hack();

	while (numops == -1 || numops--)
		test();

	close_test_files();
	prt("All operations completed A-OK!\n");

	if (tf_buf)
		free(tf_buf);

	free(original_buf);
	free(good_buf);
	free(temp_buf);

	return 0;
}
