// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Write data into a test file using various methods and verify that file
 * contents match what was written.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/statvfs.h>
#include "tst_test.h"
#include "tst_safe_prw.h"

#define MAX_VEC 8
#define TEST_FILENAME "fsplough.dat"

typedef void (*io_func)(void *buf, size_t offset, size_t size);

static char *workdir_arg;
static char *directwr_flag;
static char *directrd_flag;
static char *loop_arg;
static int loop_count;

static int read_fd = -1, write_fd = -1;
static char *writebuf, *filedata;
static size_t blocksize, bufsize, filesize;

static struct tst_test test;
static void do_write(void *buf, size_t offset, size_t size);
static void do_pwrite(void *buf, size_t offset, size_t size);
static void do_writev(void *buf, size_t offset, size_t size);
static void do_pwritev(void *buf, size_t offset, size_t size);
static void do_read(void *buf, size_t offset, size_t size);
static void do_pread(void *buf, size_t offset, size_t size);
static void do_readv(void *buf, size_t offset, size_t size);
static void do_preadv(void *buf, size_t offset, size_t size);

static const io_func write_funcs[] = {
	do_write,
	do_pwrite,
	do_writev,
	do_pwritev
};

static const io_func read_funcs[] = {
	do_read,
	do_pread,
	do_readv,
	do_preadv
};

static size_t fill_buffer(char *buf, size_t size)
{
	size_t i, ret = MAX_VEC + 1 + rand() % (size - MAX_VEC);

	/* Align buffer size to block size */
	if (directwr_flag || directrd_flag)
		ret = MAX(LTP_ALIGN(ret, blocksize), MAX_VEC * blocksize);

	for (i = 0; i < ret; i++)
		buf[i] = rand();

	return ret;
}

static void vectorize_buffer(struct iovec *vec, size_t vec_size, char *buf,
	size_t buf_size, int align)
{
	size_t i, len, chunk = align ? blocksize : 1;

	memset(vec, 0, vec_size * sizeof(struct iovec));
	buf_size /= chunk;

	for (i = 0; buf_size && i < vec_size; i++) {
		len = 1 + rand() % (buf_size + i + 1 - vec_size);
		vec[i].iov_base = buf;
		vec[i].iov_len = len * chunk;
		buf += vec[i].iov_len;
		buf_size -= len;
	}

	vec[vec_size - 1].iov_len += buf_size * chunk;
}

static void update_filedata(const void *buf, size_t offset, size_t size)
{
	memcpy(filedata + offset, buf, size * sizeof(char));
}

static void do_write(void *buf, size_t offset, size_t size)
{
	SAFE_LSEEK(write_fd, offset, SEEK_SET);
	SAFE_WRITE(1, write_fd, buf, size);
}

static void do_pwrite(void *buf, size_t offset, size_t size)
{
	SAFE_PWRITE(1, write_fd, buf, size, offset);
}

static void do_writev(void *buf, size_t offset, size_t size)
{
	struct iovec vec[MAX_VEC] = {};

	vectorize_buffer(vec, MAX_VEC, buf, size, !!directwr_flag);
	SAFE_LSEEK(write_fd, offset, SEEK_SET);
	SAFE_WRITEV(1, write_fd, vec, MAX_VEC);
}

static void do_pwritev(void *buf, size_t offset, size_t size)
{
	struct iovec vec[MAX_VEC] = {};

	vectorize_buffer(vec, MAX_VEC, buf, size, !!directwr_flag);
	SAFE_PWRITEV(1, write_fd, vec, MAX_VEC, offset);
}

static void do_read(void *buf, size_t offset, size_t size)
{
	SAFE_LSEEK(read_fd, offset, SEEK_SET);
	SAFE_READ(1, read_fd, buf, size);
}

static void do_pread(void *buf, size_t offset, size_t size)
{
	SAFE_PREAD(1, read_fd, buf, size, offset);
}

static void do_readv(void *buf, size_t offset, size_t size)
{
	struct iovec vec[MAX_VEC] = {};

	vectorize_buffer(vec, MAX_VEC, buf, size, !!directrd_flag);
	SAFE_LSEEK(read_fd, offset, SEEK_SET);
	SAFE_READV(1, read_fd, vec, MAX_VEC);
}

static void do_preadv(void *buf, size_t offset, size_t size)
{
	struct iovec vec[MAX_VEC] = {};

	vectorize_buffer(vec, MAX_VEC, buf, size, !!directrd_flag);
	SAFE_PREADV(1, read_fd, vec, MAX_VEC, offset);
}

static int open_testfile(int flags)
{
	if ((flags & O_WRONLY) && directwr_flag)
		flags |= O_DIRECT;

	if ((flags & O_RDONLY) && directrd_flag)
		flags |= O_DIRECT;

	return SAFE_OPEN(TEST_FILENAME, flags, 0644);
}

static void setup(void)
{
	struct statvfs statbuf;
	size_t pagesize;
	int runtime;

	srand(time(0));
	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);

	if (workdir_arg)
		SAFE_CHDIR(workdir_arg);

	if (tst_parse_int(loop_arg, &loop_count, 0, INT_MAX))
		tst_brk(TBROK, "Invalid write loop count: %s", loop_arg);

	write_fd = open_testfile(O_WRONLY | O_CREAT | O_TRUNC);
	read_fd = open_testfile(O_RDONLY);
	TEST(fstatvfs(write_fd, &statbuf));

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "fstatvfs() failed");
	else if (TST_RET)
		tst_brk(TBROK | TTERRNO, "Invalid fstatvfs() return value");

	blocksize = statbuf.f_bsize;
	tst_res(TINFO, "Block size: %zu", blocksize);
	bufsize = 4 * MAX_VEC * MAX(pagesize, blocksize);
	filesize = 1024 * MAX(pagesize, blocksize);
	writebuf = SAFE_MMAP(NULL, bufsize, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	filedata = SAFE_MALLOC(filesize);

	if (loop_arg) {
		/*
		 * Executing fixed number of loops. Use calculated runtime
		 * as timeout and apply the timeout multiplier.
		 */
		runtime = bufsize * loop_count / (8 * 1024 * 1024);
		runtime = tst_multiply_timeout(runtime);

		if (runtime > test.runtime)
			tst_set_runtime(runtime);
	}
}

static void run(void)
{
	size_t start, length;
	int i, f, fails = 0;

	/* Test data consistency between random writes */
	for (i = 0; !loop_arg || i < loop_count; i++) {
		if (!tst_remaining_runtime())
			break;

		length = fill_buffer(writebuf, bufsize);
		start = rand() % (filesize + 1 - length);

		/* Align offset to blocksize if needed */
		if (directrd_flag || directwr_flag)
			start = (start + blocksize / 2) & ~(blocksize - 1);

		update_filedata(writebuf, start, length);
		f = rand() % ARRAY_SIZE(write_funcs);
		write_funcs[f](writebuf, start, length);

		memset(writebuf, 0, length);
		f = rand() % ARRAY_SIZE(read_funcs);
		read_funcs[f](writebuf, start, length);

		if (memcmp(writebuf, filedata + start, length)) {
			tst_res(TFAIL, "Partial data mismatch at [%zu:%zu]",
				start, start + length);
			fails++;
		}
	}

	if (i < loop_count / 2) {
		tst_res(TWARN, "Runtime expired, exiting early after %d loops",
			i);
		tst_res(TINFO, "If you are running on slow machine, "
			"try exporting LTP_TIMEOUT_MUL > 1");
	} else if (i < loop_count) {
		tst_res(TINFO, "Runtime expired, exiting early after %d loops",
			i);
	} else if (!loop_arg && i < 10) {
		tst_res(TWARN, "Slow system: test performed only %d loops!", i);
	} else {
		tst_res(TPASS, "Exiting after %d loops", i);
	}

	if (!fails)
		tst_res(TPASS, "Partial data are consistent");

	/* Ensure that the testfile has the expected size */
	do_write(writebuf, filesize - blocksize, blocksize);
	update_filedata(writebuf, filesize - blocksize, blocksize);

	/* Sync the testfile and clear cache */
	SAFE_CLOSE(read_fd);
	SAFE_FSYNC(write_fd);
	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "1");
	read_fd = open_testfile(O_RDONLY);

	/* Check final file contents */
	for (start = 0; start < filesize; start += bufsize) {
		length = MIN(bufsize, filesize - start);
		SAFE_READ(1, read_fd, writebuf, length);

		if (memcmp(writebuf, filedata + start, length)) {
			tst_res(TFAIL, "Final data mismatch at [%zu:%zu]",
				start, start + length);
			return;
		}
	}

	tst_res(TPASS, "Final data are consistent");
}

static void cleanup(void)
{
	SAFE_MUNMAP(writebuf, bufsize);
	free(filedata);

	if (read_fd >= 0)
		SAFE_CLOSE(read_fd);

	if (write_fd >= 0)
		SAFE_CLOSE(write_fd);

	SAFE_UNLINK(TEST_FILENAME);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.runtime = 30,
	.options = (struct tst_option[]) {
		{"c:", &loop_arg,
			"Number of write loops (default: loop for 30 seconds)"},
		{"d:", &workdir_arg, "Path to working directory"},
		{"W", &directwr_flag, "Use direct I/O for writing"},
		{"R", &directrd_flag, "Use direct I/O for reading"},
		{}
	}
};
