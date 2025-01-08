// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 1991, NeXT Computer, Inc.  All Rights Reserverd.
 *	Author:	Avadis Tevanian, Jr.
 *
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
 *	Conrad Minshall <conrad@mac.com>
 *	Dave Jones <davej@suse.de>
 *	Zach Brown <zab@clusterfs.com>
 *	Joe Sokol, Pat Dirks, Clark Warner, Guy Harris
 *
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This is a complete rewrite of the old fsx-linux tool, created by
 * NeXT Computer, Inc. and Apple Computer, Inc. between 1991 and 2001,
 * then adapted for LTP. Test is actually a file system exerciser: we bring a
 * file and randomly write operations like read/write/map read/map write and
 * truncate, according with input parameters. Then we check if all of them
 * have been completed.
 */

#include <stdlib.h>
#include "tst_test.h"

#define FNAME "ltp-file.bin"

enum {
	OP_READ = 0,
	OP_WRITE,
	OP_TRUNCATE,
	OP_MAPREAD,
	OP_MAPWRITE,
	/* keep counter here */
	OP_TOTAL,
};

static char *str_file_max_size;
static char *str_op_max_size;
static char *str_op_nums;
static char *str_op_write_align;
static char *str_op_read_align;
static char *str_op_trunc_align;

static int file_desc;
static long long file_max_size = 256 * 1024;
static long long op_max_size = 64 * 1024;
static long long file_size;
static int op_write_align = 1;
static int op_read_align = 1;
static int op_trunc_align = 1;
static int op_nums = 1000;
static int page_size;

static char *file_buff;
static char *temp_buff;

struct file_pos_t {
	long long offset;
	long long size;
};

static void op_align_pages(struct file_pos_t *pos)
{
	long long pg_offset;

	pg_offset = pos->offset % page_size;

	pos->offset -= pg_offset;
	pos->size += pg_offset;
}

static void op_file_position(
	const long long fsize,
	const int align,
	struct file_pos_t *pos)
{
	long long diff;

	pos->offset = random() % fsize;
	pos->size = random() % (fsize - pos->offset);

	diff = pos->offset % align;

	if (diff) {
		pos->offset -= diff;
		pos->size += diff;
	}

	if (!pos->size)
		pos->size = 1;
}

static void update_file_size(struct file_pos_t const *pos)
{
	if (pos->offset + pos->size > file_size) {
		file_size = pos->offset + pos->size;
		tst_res(TDEBUG, "File size changed: %llu", file_size);
	}
}

static int memory_compare(
	const char *a,
	const char *b,
	const long long offset,
	const long long size)
{
	int diff;

	for (long long i = 0; i < size; i++) {
		diff = a[i] - b[i];
		if (diff) {
			tst_res(TDEBUG, "File memory differs at offset=%llu ('%c' != '%c')",
				offset + i, a[i], b[i]);
			break;
		}
	}

	return diff;
}

static int op_read(void)
{
	if (!file_size) {
		tst_res(TINFO, "Skipping zero size read");
		return 0;
	}

	struct file_pos_t pos;

	op_file_position(file_size, op_read_align, &pos);

	tst_res(TDEBUG, "Reading at offset=%llu, size=%llu",
		pos.offset, pos.size);

	memset(temp_buff, 0, file_max_size);

	SAFE_LSEEK(file_desc, (off_t)pos.offset, SEEK_SET);
	SAFE_READ(0, file_desc, temp_buff, pos.size);

	int ret = memory_compare(
		file_buff + pos.offset,
		temp_buff,
		pos.offset,
		pos.size);

	if (ret)
		return -1;

	return 1;
}

static int op_write(void)
{
	if (file_size >= file_max_size) {
		tst_res(TINFO, "Skipping max size write");
		return 0;
	}

	struct file_pos_t pos;
	char data;

	op_file_position(file_max_size, op_write_align, &pos);

	for (long long i = 0; i < pos.size; i++) {
		data = random() % 10 + 'a';

		file_buff[pos.offset + i] = data;
		temp_buff[i] = data;
	}

	tst_res(TDEBUG, "Writing at offset=%llu, size=%llu",
		pos.offset, pos.size);

	SAFE_LSEEK(file_desc, (off_t)pos.offset, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, file_desc, temp_buff, pos.size);

	update_file_size(&pos);

	return 1;
}

static int op_truncate(void)
{
	struct file_pos_t pos;

	op_file_position(file_max_size, op_trunc_align, &pos);
	file_size = pos.offset + pos.size;

	tst_res(TDEBUG, "Truncating to %llu", file_size);

	SAFE_FTRUNCATE(file_desc, file_size);
	memset(file_buff + file_size, 0, file_max_size - file_size);

	return 1;
}

static int op_map_read(void)
{
	if (!file_size) {
		tst_res(TINFO, "Skipping zero size read");
		return 0;
	}

	struct file_pos_t pos;
	char *addr;

	op_file_position(file_size, op_read_align, &pos);
	op_align_pages(&pos);

	tst_res(TDEBUG, "Map reading at offset=%llu, size=%llu",
		pos.offset, pos.size);

	addr = SAFE_MMAP(
		0, pos.size,
		PROT_READ,
		MAP_FILE | MAP_SHARED,
		file_desc,
		(off_t)pos.offset);

	memcpy(file_buff + pos.offset, addr, pos.size);

	int ret = memory_compare(
		addr,
		file_buff + pos.offset,
		pos.offset,
		pos.size);

	SAFE_MUNMAP(addr, pos.size);
	if (ret)
		return -1;

	return 1;
}

static int op_map_write(void)
{
	if (file_size >= file_max_size) {
		tst_res(TINFO, "Skipping max size write");
		return 0;
	}

	struct file_pos_t pos;
	char *addr;

	op_file_position(file_max_size, op_write_align, &pos);
	op_align_pages(&pos);

	if (file_size < pos.offset + pos.size)
		SAFE_FTRUNCATE(file_desc, pos.offset + pos.size);

	tst_res(TDEBUG, "Map writing at offset=%llu, size=%llu",
		pos.offset, pos.size);

	for (long long i = 0; i < pos.size; i++)
		file_buff[pos.offset + i] = random() % 10 + 'l';

	addr = SAFE_MMAP(
		0, pos.size,
		PROT_READ | PROT_WRITE,
		MAP_FILE | MAP_SHARED,
		file_desc,
		(off_t)pos.offset);

	memcpy(addr, file_buff + pos.offset, pos.size);
	SAFE_MSYNC(addr, pos.size, MS_SYNC);
	SAFE_MUNMAP(addr, pos.size);
	update_file_size(&pos);

	return 1;
}

static void run(void)
{
	int op;
	int ret;
	int counter = 0;

	file_size = 0;

	memset(file_buff, 0, file_max_size);
	memset(temp_buff, 0, file_max_size);

	SAFE_FTRUNCATE(file_desc, 0);

	while (counter < op_nums) {
		op = random() % OP_TOTAL;

		switch (op) {
		case OP_WRITE:
			ret = op_write();
			break;
		case OP_MAPREAD:
			ret = op_map_read();
			break;
		case OP_MAPWRITE:
			ret = op_map_write();
			break;
		case OP_TRUNCATE:
			ret = op_truncate();
			break;
		case OP_READ:
		default:
			ret = op_read();
			break;
		};

		if (ret == -1)
			break;

		counter += ret;
	}

	if (counter != op_nums)
		tst_brk(TFAIL, "Some file operations failed");
	else
		tst_res(TPASS, "All file operations succeed");
}

static void setup(void)
{
	if (tst_parse_filesize(str_file_max_size, &file_max_size, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid file size '%s'", str_file_max_size);

	if (tst_parse_filesize(str_op_max_size, &op_max_size, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid maximum size for single operation '%s'", str_op_max_size);

	if (tst_parse_int(str_op_nums, &op_nums, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of operations '%s'", str_op_nums);

	if (tst_parse_int(str_op_write_align, &op_write_align, 1, INT_MAX))
		tst_brk(TBROK, "Invalid memory write alignment factor '%s'", str_op_write_align);

	if (tst_parse_int(str_op_read_align, &op_read_align, 1, INT_MAX))
		tst_brk(TBROK, "Invalid memory read alignment factor '%s'", str_op_read_align);

	if (tst_parse_int(str_op_trunc_align, &op_trunc_align, 1, INT_MAX))
		tst_brk(TBROK, "Invalid memory truncate alignment factor '%s'", str_op_trunc_align);

	page_size = (int)sysconf(_SC_PAGESIZE);

	srandom(time(NULL));

	file_desc = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0666);

	file_buff = SAFE_MALLOC(file_max_size);
	temp_buff = SAFE_MALLOC(file_max_size);
}

static void cleanup(void)
{
	if (file_buff)
		free(file_buff);

	if (temp_buff)
		free(temp_buff);

	if (file_desc)
		SAFE_CLOSE(file_desc);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.timeout = 1800,
	.options = (struct tst_option[]) {
		{ "l:", &str_file_max_size, "Maximum size in MB of the test file(s) (default 262144)" },
		{ "o:", &str_op_max_size, "Maximum size for single operation (default 65536)" },
		{ "N:", &str_op_nums, "Total # operations to do (default 1000)" },
		{ "w:", &str_op_write_align, "Write memory page alignment (default 1)" },
		{ "r:", &str_op_read_align, "Read memory page alignment (default 1)" },
		{ "t:", &str_op_trunc_align, "Truncate memory page alignment (default 1)" },
		{},
	},
};
