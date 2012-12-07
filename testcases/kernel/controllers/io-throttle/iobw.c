/*
 * iobw.c - simple I/O bandwidth benchmark
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 *
 * Copyright (C) 2008 Andrea Righi <righi.andrea@gmail.com>
 */

#define _GNU_SOURCE
#define __USE_GNU

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#endif

#define align(x,a)		__align_mask(x,(typeof(x))(a)-1)
#define __align_mask(x,mask)	(((x)+(mask))&~(mask))
#define kb(x)			((x) >> 10)

const char usage[] = "Usage: iobw [-direct] threads chunk_size data_size\n";
const char child_fmt[] = "(%s) task %3d: time %4lu.%03lu bw %7lu KiB/s (%s)\n";
const char parent_fmt[] =
    "(%s) parent %d: time %4lu.%03lu bw %7lu KiB/s (%s)\n";

static int directio = 0;
static size_t data_size = 0;
static size_t chunk_size = 0;

typedef enum {
	OP_WRITE,
	OP_READ,
	NUM_IOPS,
} iops_t;

static const char *iops[] = {
	"WRITE",
	"READ ",
	"TOTAL",
};

static int threads;
pid_t *children;

char *mygroup;

static void print_results(int id, iops_t op, size_t bytes, struct timeval *diff)
{
	fprintf(stdout, id ? child_fmt : parent_fmt,
		mygroup, id, diff->tv_sec, diff->tv_usec / 1000,
		(bytes / (diff->tv_sec * 1000000L + diff->tv_usec))
		* 1000000L / 1024, iops[op]);
}

static void thread(int id)
{
	struct timeval start, stop, diff;
	int fd, i, ret;
	size_t n;
	void *buf;
	int flags = O_CREAT | O_RDWR | O_LARGEFILE;
	char filename[32];

	ret = posix_memalign(&buf, PAGE_SIZE, chunk_size);
	if (ret < 0) {
		fprintf(stderr,
			"ERROR: task %d couldn't allocate %zu bytes (%s)\n",
			id, chunk_size, strerror(errno));
		exit(1);
	}
	memset(buf, 0xaa, chunk_size);

	snprintf(filename, sizeof(filename), "%s-%d-iobw.tmp", mygroup, id);
	if (directio)
		flags |= O_DIRECT;
	fd = open(filename, flags, 0600);
	if (fd < 0) {
		fprintf(stderr, "ERROR: task %d couldn't open %s (%s)\n",
			id, filename, strerror(errno));
		free(buf);
		exit(1);
	}

	/* Write */
	lseek(fd, 0, SEEK_SET);
	n = 0;
	gettimeofday(&start, NULL);
	while (n < data_size) {
		i = write(fd, buf, chunk_size);
		if (i < 0) {
			fprintf(stderr, "ERROR: task %d writing to %s (%s)\n",
				id, filename, strerror(errno));
			ret = 1;
			goto out;
		}
		n += i;
	}
	gettimeofday(&stop, NULL);
	timersub(&stop, &start, &diff);
	print_results(id + 1, OP_WRITE, data_size, &diff);

	/* Read */
	lseek(fd, 0, SEEK_SET);
	n = 0;
	gettimeofday(&start, NULL);
	while (n < data_size) {
		i = read(fd, buf, chunk_size);
		if (i < 0) {
			fprintf(stderr, "ERROR: task %d reading to %s (%s)\n",
				id, filename, strerror(errno));
			ret = 1;
			goto out;
		}
		n += i;
	}
	gettimeofday(&stop, NULL);
	timersub(&stop, &start, &diff);
	print_results(id + 1, OP_READ, data_size, &diff);
out:
	close(fd);
	unlink(filename);
	free(buf);
	exit(ret);
}

static void spawn(int id)
{
	pid_t pid;

	pid = fork();
	switch (pid) {
	case -1:
		fprintf(stderr, "ERROR: couldn't fork thread %d\n", id);
		exit(1);
	case 0:
		thread(id);
	default:
		children[id] = pid;
	}
}

void signal_handler(int sig)
{
	char filename[32];
	int i;

	for (i = 0; i < threads; i++)
		if (children[i])
			kill(children[i], SIGKILL);

	for (i = 0; i < threads; i++) {
		struct stat mystat;

		snprintf(filename, sizeof(filename), "%s-%d-iobw.tmp",
			 mygroup, i);
		if (stat(filename, &mystat) < 0)
			continue;
		unlink(filename);
	}

	fprintf(stdout, "received signal %d, exiting\n", sig);
	exit(0);
}

unsigned long long memparse(char *ptr, char **retptr)
{
	unsigned long long ret = strtoull(ptr, retptr, 0);

	switch (**retptr) {
	case 'G':
	case 'g':
		ret <<= 10;
	case 'M':
	case 'm':
		ret <<= 10;
	case 'K':
	case 'k':
		ret <<= 10;
		(*retptr)++;
	default:
		break;
	}
	return ret;
}

int main(int argc, char *argv[])
{
	struct timeval start, stop, diff;
	char *end;
	int i;

	if (argv[1] && strcmp(argv[1], "-direct") == 0) {
		directio = 1;
		argc--;
		argv++;
	}
	if (argc != 4) {
		fprintf(stderr, usage);
		exit(1);
	}
	if ((threads = atoi(argv[1])) == 0) {
		fprintf(stderr, usage);
		exit(1);
	}
	chunk_size = align(memparse(argv[2], &end), PAGE_SIZE);
	if (*end) {
		fprintf(stderr, usage);
		exit(1);
	}
	data_size = align(memparse(argv[3], &end), PAGE_SIZE);
	if (*end) {
		fprintf(stderr, usage);
		exit(1);
	}

	/* retrieve group name */
	mygroup = getenv("MYGROUP");
	if (!mygroup) {
		fprintf(stderr,
			"ERROR: undefined environment variable MYGROUP\n");
		exit(1);
	}

	children = malloc(sizeof(pid_t) * threads);
	if (!children) {
		fprintf(stderr, "ERROR: not enough memory\n");
		exit(1);
	}

	/* handle user interrupt */
	signal(SIGINT, signal_handler);
	/* handle kill from shell */
	signal(SIGTERM, signal_handler);

	fprintf(stdout, "chunk_size %zuKiB, data_size %zuKiB\n",
		kb(chunk_size), kb(data_size));
	fflush(stdout);

	gettimeofday(&start, NULL);
	for (i = 0; i < threads; i++)
		spawn(i);
	for (i = 0; i < threads; i++) {
		int status;
		wait(&status);
		if (!WIFEXITED(status))
			exit(1);
	}
	gettimeofday(&stop, NULL);

	timersub(&stop, &start, &diff);
	print_results(0, NUM_IOPS, data_size * threads * NUM_IOPS, &diff);
	fflush(stdout);
	free(children);

	exit(0);
}
