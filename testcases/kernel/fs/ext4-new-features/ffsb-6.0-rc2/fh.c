/*
 *   Copyright (c) International Business Machines Corp., 2001-2004
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <assert.h>

#include "ffsb.h"
#include "fh.h"

#include "config.h"

/* !!! ugly */
#ifndef HAVE_OPEN64
#define open64 open
#endif

#ifndef HAVE_FSEEKO64
#define lseek64 lseek
#endif

/* All these functions read the global mainconfig->bufferedio variable
 * to determine if they are to do buffered i/o or normal.
 *
 * ha, well, they're supposed to anyway...!!! TODO -SR 2006/05/14
 */

static void do_stats(struct timeval *start, struct timeval *end,
		     ffsb_thread_t * ft, ffsb_fs_t * fs, syscall_t sys)
{
	struct timeval diff;
	uint32_t value = 0;

	if (!ft && !fs)
		return;

	timersub(end, start, &diff);

	value = 1000000 * diff.tv_sec + diff.tv_usec;

	if (ft && ft_needs_stats(ft, sys))
		ft_add_stat(ft, sys, value);
	if (fs && fs_needs_stats(fs, sys))
		fs_add_stat(fs, sys, value);
}

static int fhopenhelper(char *filename, char *bufflags, int flags,
			ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	int fd = 0;
	struct timeval start, end;
	int need_stats = ft_needs_stats(ft, SYS_OPEN) ||
	    fs_needs_stats(fs, SYS_OPEN);

	flags |= O_LARGEFILE;

	if (need_stats)
		gettimeofday(&start, NULL);

	fd = open64(filename, flags, S_IRWXU);
	if (fd < 0) {
		perror(filename);
		exit(0);
	}

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_OPEN);
	}

	return fd;
}

int fhopenread(char *filename, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	int flags = O_RDONLY;
	int directio = fs_get_directio(fs);

	if (directio)
		flags |= O_DIRECT;
	return fhopenhelper(filename, "r", flags, ft, fs);
}

int fhopenappend(char *filename, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	int flags = O_APPEND | O_WRONLY;
	int directio = fs_get_directio(fs);

	if (directio)
		flags |= O_DIRECT;
	return fhopenhelper(filename, "a", flags, ft, fs);
}

int fhopenwrite(char *filename, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	int flags = O_WRONLY;
	int directio = fs_get_directio(fs);

	if (directio)
		flags |= O_DIRECT;
	return fhopenhelper(filename, "w", flags, ft, fs);
}

int fhopencreate(char *filename, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	int flags = O_CREAT | O_RDWR | O_TRUNC;
	int directio = fs_get_directio(fs);

	if (directio)
		flags |= O_DIRECT;
	return fhopenhelper(filename, "rw", flags, ft, fs);
}

void fhread(int fd, void *buf, uint64_t size, ffsb_thread_t * ft,
	    ffsb_fs_t * fs)
{
	ssize_t realsize;
	struct timeval start, end;
	int need_stats = ft_needs_stats(ft, SYS_READ) ||
	    fs_needs_stats(fs, SYS_READ);

	assert(size <= SIZE_MAX);
	if (need_stats)
		gettimeofday(&start, NULL);
	realsize = read(fd, buf, size);

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_READ);
	}

	if (realsize != size) {
		printf("Read %lld instead of %llu bytes.\n",
		       (unsigned long long)realsize, (unsigned long long)size);
		perror("read");
		exit(1);
	}
}

void fhwrite(int fd, void *buf, uint32_t size, ffsb_thread_t * ft,
	     ffsb_fs_t * fs)
{
	ssize_t realsize;
	struct timeval start, end;
	int need_stats = ft_needs_stats(ft, SYS_WRITE) ||
	    fs_needs_stats(fs, SYS_WRITE);

	assert(size <= SIZE_MAX);
	if (need_stats)
		gettimeofday(&start, NULL);

	realsize = write(fd, buf, size);

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_WRITE);
	}

	if (realsize != size) {
		printf("Wrote %d instead of %d bytes.\n"
		       "Probably out of disk space\n", realsize, size);
		perror("write");
		exit(1);
	}
}

void fhseek(int fd, uint64_t offset, int whence, ffsb_thread_t * ft,
	    ffsb_fs_t * fs)
{
	uint64_t res;
	struct timeval start, end;
	int need_stats = ft_needs_stats(ft, SYS_LSEEK) ||
	    fs_needs_stats(fs, SYS_LSEEK);

	if ((whence == SEEK_CUR) && (offset == 0))
		return;

	if (need_stats)
		gettimeofday(&start, NULL);

	res = lseek64(fd, offset, whence);

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_LSEEK);
	}
	if ((whence == SEEK_SET) && (res != offset))
		perror("seek");

	if (res == -1) {
		if (whence == SEEK_SET)
			fprintf(stderr, "tried to seek to %lld\n", offset);
		else
			fprintf(stderr, "tried to seek from current "
				"position to %lld\n", offset);

		perror("seek");
		exit(1);
	}
}

void fhclose(int fd, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	struct timeval start, end;
	int need_stats = ft_needs_stats(ft, SYS_CLOSE) ||
	    fs_needs_stats(fs, SYS_CLOSE);

	if (need_stats)
		gettimeofday(&start, NULL);

	close(fd);

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_CLOSE);
	}
}

void fhstat(char *name, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	struct timeval start, end;
	struct stat tmp_stat;

	int need_stats = ft_needs_stats(ft, SYS_STAT) ||
	    fs_needs_stats(fs, SYS_CLOSE);

	if (need_stats)
		gettimeofday(&start, NULL);

	if (stat(name, &tmp_stat)) {
		fprintf(stderr, "stat call failed for file %s\n", name);
		exit(1);
	}

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_STAT);
	}
}

int writefile_helper(int fd, uint64_t size, uint32_t blocksize, char *buf,
		     struct ffsb_thread *ft, struct ffsb_fs *fs)
{
	uint64_t iterations, a;
	uint64_t last;

	iterations = size / blocksize;
	last = size % blocksize;

	for (a = 0; a < iterations; a++)
		fhwrite(fd, buf, blocksize, ft, fs);

	if (last) {
		a++;
		fhwrite(fd, buf, last, ft, fs);
	}
	return a;
}
