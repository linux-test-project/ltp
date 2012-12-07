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
#define _LARGEFILE64_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "fh.h"
#include "util.h"
#include "ffsb.h"
#include "fileops.h"
#include "ffsb_op.h"

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

void fop_bench(ffsb_fs_t * fs, unsigned opnum)
{
	fs_set_opdata(fs, fs_get_datafiles(fs), opnum);
}

void fop_age(ffsb_fs_t * fs, unsigned opnum)
{
	fs_set_opdata(fs, fs_get_agefiles(fs), opnum);
}

static unsigned readfile_helper(int fd, uint64_t size, uint32_t blocksize,
				char *buf, ffsb_thread_t * ft, ffsb_fs_t * fs)
{
	int iterations, a;
	int last;

	iterations = size / blocksize;
	last = size % blocksize;

	for (a = 0; a < iterations; a++)
		fhread(fd, buf, blocksize, ft, fs);
	if (last)
		fhread(fd, buf, last, ft, fs);
	return iterations;
}

static uint64_t get_random_offset(randdata_t * rd, uint64_t filesize,
				  int aligned)
{
	if (!aligned)
		return getllrandom(rd, filesize);

	filesize /= 4096;
	return getllrandom(rd, filesize) * 4096;
}

void ffsb_readfile(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;

	int fd;
	uint64_t filesize;

	char *buf = ft_getbuf(ft);
	int read_random = ft_get_read_random(ft);
	uint64_t read_size = ft_get_read_size(ft);
	uint32_t read_blocksize = ft_get_read_blocksize(ft);
	uint32_t read_skipsize = ft_get_read_skipsize(ft);
	int skip_reads = ft_get_read_skip(ft);
	struct randdata *rd = ft_get_randdata(ft);

	uint64_t iterations = 0;

	curfile = choose_file_reader(bf, rd);
	fd = fhopenread(curfile->name, ft, fs);

	filesize = ffsb_get_filesize(curfile->name);

	assert(filesize >= read_size);

	/* Sequential read, starting at a random point */
	if (!read_random) {
		uint64_t range = filesize - read_size;
		uint64_t offset = 0;
		/* Skip or "stride" reads option */
		if (skip_reads) {
			unsigned i, last;
			uint64_t minfilesize;
			iterations = read_size / read_blocksize;
			last = read_size % read_blocksize;

			/* Double check that the user hasn't specified
			 * a read_size that is too large when combined
			 * with the seeks
			 */
			if (last)
				minfilesize = last + iterations *
				    (read_blocksize + read_skipsize);
			else
				minfilesize = read_blocksize + iterations - 1 *
				    (read_blocksize + read_skipsize);

			if (minfilesize > filesize) {
				printf("Error: read size %llu bytes too big "
				       "w/ skipsize %u and blocksize %u,"
				       " for file of size %llu bytes\n"
				       " aborting\n\n", read_size,
				       read_skipsize, read_blocksize, filesize);
				printf("minimum file size must be at least "
				       " %llu bytes\n", minfilesize);
				exit(1);
			}

			for (i = 0; i < iterations; i++) {
				fhread(fd, buf, read_blocksize, ft, fs);
				fhseek(fd, (uint64_t) read_skipsize, SEEK_CUR,
				       ft, fs);
			}
			if (last) {
				fhread(fd, buf, (uint64_t) last, ft, fs);
				iterations++;
			}
		} else {
			/* Regular sequential reads */
			if (range) {
				offset = get_random_offset(rd, range,
							   fs_get_alignio(fs));
				fhseek(fd, offset, SEEK_SET, ft, fs);
			}
			iterations = readfile_helper(fd, read_size,
						     read_blocksize, buf,
						     ft, fs);
		}
	} else {
		/* Randomized read */
		uint64_t range = filesize - read_blocksize;
		int i;

		iterations = read_size / read_blocksize;

		for (i = 0; i < iterations; i++) {
			uint64_t offset = get_random_offset(rd, range,
							    fs_get_alignio(fs));
			fhseek(fd, offset, SEEK_SET, ft, fs);
			fhread(fd, buf, read_blocksize, ft, fs);
		}
	}

	unlock_file_reader(curfile);
	fhclose(fd, ft, fs);

	ft_incr_op(ft, opnum, iterations, read_size);
	ft_add_readbytes(ft, read_size);
}

/* Just like ffsb_readfile but we read the whole file from start to
 * finish regardless of file size.
 */
void ffsb_readall(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;
	int fd;
	uint64_t filesize;

	char *buf = ft_getbuf(ft);
	uint32_t read_blocksize = ft_get_read_blocksize(ft);
	struct randdata *rd = ft_get_randdata(ft);

	unsigned iterations = 0;

	curfile = choose_file_reader(bf, rd);
	fd = fhopenread(curfile->name, ft, fs);

	filesize = ffsb_get_filesize(curfile->name);
	iterations = readfile_helper(fd, filesize, read_blocksize, buf, ft, fs);

	unlock_file_reader(curfile);
	fhclose(fd, ft, fs);

	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_readbytes(ft, filesize);
}

/* Shared core between ffsb_writefile and ffsb_writefile_fsync.*/

static unsigned ffsb_writefile_core(ffsb_thread_t * ft, ffsb_fs_t * fs,
				    unsigned opnum, uint64_t * filesize_ret,
				    int fsync_file)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;

	int fd;
	uint64_t filesize;

	char *buf = ft_getbuf(ft);
	int write_random = ft_get_write_random(ft);
	uint32_t write_size = ft_get_write_size(ft);
	uint32_t write_blocksize = ft_get_write_blocksize(ft);
	struct randdata *rd = ft_get_randdata(ft);
	unsigned iterations = 0;

	curfile = choose_file_reader(bf, rd);
	fd = fhopenwrite(curfile->name, ft, fs);

	filesize = ffsb_get_filesize(curfile->name);

	assert(filesize >= write_size);

	/* Sequential write, starting at a random point  */
	if (!write_random) {
		uint64_t range = filesize - write_size;
		uint64_t offset = 0;
		if (range) {
			offset = get_random_offset(rd, range,
						   fs_get_alignio(fs));
			fhseek(fd, offset, SEEK_SET, ft, fs);
		}
		iterations = writefile_helper(fd, write_size, write_blocksize,
					      buf, ft, fs);
	} else {
		/* Randomized write */
		uint64_t range = filesize - write_blocksize;
		int i;
		iterations = write_size / write_blocksize;

		for (i = 0; i < iterations; i++) {
			uint64_t offset = get_random_offset(rd, range,
							    fs_get_alignio(fs));
			fhseek(fd, offset, SEEK_SET, ft, fs);
			fhwrite(fd, buf, write_blocksize, ft, fs);
		}
	}

	if (fsync_file) {
		if (fsync(fd)) {
			perror("fsync");
			printf("aborting\n");
			exit(1);
		}
	}
	unlock_file_reader(curfile);
	fhclose(fd, ft, fs);
	*filesize_ret = filesize;
	return iterations;
}

void ffsb_writefile(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_writefile_core(ft, fs, opnum, &filesize, 0);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

void ffsb_writefile_fsync(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_writefile_core(ft, fs, opnum, &filesize, 1);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

/* Shared core between ffsb_writeall and ffsb_writeall_fsync.*/

static unsigned ffsb_writeall_core(ffsb_thread_t * ft, ffsb_fs_t * fs,
				   unsigned opnum, uint64_t * filesize_ret,
				   int fsync_file)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;
	int fd;
	uint64_t filesize;

	char *buf = ft_getbuf(ft);
	uint32_t write_blocksize = ft_get_write_blocksize(ft);
	struct randdata *rd = ft_get_randdata(ft);

	unsigned iterations = 0;

	curfile = choose_file_reader(bf, rd);
	fd = fhopenwrite(curfile->name, ft, fs);

	filesize = ffsb_get_filesize(curfile->name);
	iterations = writefile_helper(fd, filesize, write_blocksize, buf,
				      ft, fs);
	if (fsync_file)
		if (fsync(fd)) {
			perror("fsync");
			printf("aborting\n");
			exit(1);
		}

	unlock_file_reader(curfile);
	fhclose(fd, ft, fs);
	*filesize_ret = filesize;
	return iterations;
}

/* Just like ffsb_writefile but we write the whole file from start to
 * finish regardless of file size
 */
void ffsb_writeall(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_writeall_core(ft, fs, opnum, &filesize, 0);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

void ffsb_writeall_fsync(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_writeall_core(ft, fs, opnum, &filesize, 1);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

static unsigned ffsb_appendfile_core(ffsb_thread_t * ft, ffsb_fs_t * fs,
				     unsigned opnum, uint64_t * filesize_ret,
				     int fsync_file)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile;

	int fd;

	char *buf = ft_getbuf(ft);
	uint32_t write_size = ft_get_write_size(ft);
	uint32_t write_blocksize = ft_get_write_blocksize(ft);
	struct randdata *rd = ft_get_randdata(ft);
	unsigned iterations = 0;

	curfile = choose_file_reader(bf, rd);
	fd = fhopenappend(curfile->name, ft, fs);

	unlock_file_reader(curfile);

	curfile->size += (uint64_t) write_size;

	iterations = writefile_helper(fd, write_size, write_blocksize, buf,
				      ft, fs);
	if (fsync_file)
		if (fsync(fd)) {
			perror("fsync");
			printf("aborting\n");
			exit(1);
		}

	fhclose(fd, ft, fs);
	*filesize_ret = write_size;
	return iterations;
}

void ffsb_appendfile(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_appendfile_core(ft, fs, opnum, &filesize, 0);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

void ffsb_appendfile_fsync(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_appendfile_core(ft, fs, opnum, &filesize, 1);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

static unsigned ffsb_createfile_core(ffsb_thread_t * ft, ffsb_fs_t * fs,
				     unsigned opnum, uint64_t * filesize_ret,
				     int fsync_file)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *newfile = NULL;

	int fd;
	uint64_t size;

	char *buf = ft_getbuf(ft);
	uint32_t write_blocksize = ft_get_write_blocksize(ft);
	struct randdata *rd = ft_get_randdata(ft);
	unsigned iterations = 0;

	if (fs->num_weights) {
		int num = 1 + getrandom(rd, fs->sum_weights);
		int curop = 0;

		while (fs->size_weights[curop].weight < num) {
			num -= fs->size_weights[curop].weight;
			curop++;
		}
		size = fs->size_weights[curop].size;
	} else {
		uint64_t range =
		    fs_get_max_filesize(fs) - fs_get_min_filesize(fs);
		size = fs_get_min_filesize(fs);
		if (range != 0)
			size += getllrandom(rd, range);
	}

	newfile = add_file(bf, size, rd);
	fd = fhopencreate(newfile->name, ft, fs);
	iterations = writefile_helper(fd, size, write_blocksize, buf, ft, fs);

	if (fsync_file)
		if (fsync(fd)) {
			perror("fsync");
			printf("aborting\n");
			exit(1);
		}

	fhclose(fd, ft, fs);
	unlock_file_writer(newfile);
	*filesize_ret = size;
	return iterations;
}

void ffsb_createfile(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_createfile_core(ft, fs, opnum, &filesize, 0);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

void ffsb_createfile_fsync(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	unsigned iterations;
	uint64_t filesize;

	iterations = ffsb_createfile_core(ft, fs, opnum, &filesize, 1);
	ft_incr_op(ft, opnum, iterations, filesize);
	ft_add_writebytes(ft, filesize);
}

void ffsb_deletefile(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;
	randdata_t *rd = ft_get_randdata(ft);
	struct timeval start, end;
	int need_stats = ft_needs_stats(ft, SYS_UNLINK) ||
	    fs_needs_stats(fs, SYS_UNLINK);

	curfile = choose_file_writer(bf, rd);
	remove_file(bf, curfile);

	if (need_stats)
		gettimeofday(&start, NULL);

	if (unlink(curfile->name) == -1) {
		printf("error deleting %s in deletefile\n", curfile->name);
		perror("deletefile");
		exit(0);
	}

	if (need_stats) {
		gettimeofday(&end, NULL);
		do_stats(&start, &end, ft, fs, SYS_UNLINK);
	}

	rw_unlock_write(&curfile->lock);

	ft_incr_op(ft, opnum, 1, 0);
}

void ffsb_open_close(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;
	randdata_t *rd = ft_get_randdata(ft);
	int fd;

	curfile = choose_file_reader(bf, rd);
	fd = fhopenread(curfile->name, ft, fs);
	fhclose(fd, ft, fs);
	unlock_file_reader(curfile);
	ft_incr_op(ft, opnum, 1, 0);
}

void ffsb_stat(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *curfile = NULL;
	randdata_t *rd = ft_get_randdata(ft);

	curfile = choose_file_reader(bf, rd);
	fhstat(curfile->name, ft, fs);
	unlock_file_reader(curfile);

	ft_incr_op(ft, opnum, 1, 0);
}
