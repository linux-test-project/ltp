// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	ported from SPIE section2/filesuite/stream1.c, by Airong Zhang
 *
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that `freopen()` substitutes the named file in place of stream.
 *
 * [Algorithm]
 *
 * - fopen() a stream
 * - fwrite() something inside it
 * - perform freopen() creating a new stream pointing to the first one
 * - fwrite() data inside the new stream
 * - check that second write to stream went to the file specified by freopen()
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define FILENAME1 "ltp_file1.txt"
#define FILENAME2 "ltp_file2.txt"

static char *buff_file1 = "abc";
static char *buff_file2 = "def";

static void read_file(const char *file, const char *str, size_t n)
{
	char buf[128];
	FILE *stream;
	size_t len;

	memset(buf, 0, sizeof(buf));

	stream = SAFE_FOPEN(file, "r");
	len = SAFE_FREAD(buf, 1, n, stream);
	SAFE_FCLOSE(stream);

	TST_EXP_EXPR(len == n, "Read the entire %s file buffer", file);
	TST_EXP_EQ_STRN(buf, str, n);
}

static void run(void)
{
	FILE *stream;

	tst_res(TINFO, "Write %s file", FILENAME1);
	stream = SAFE_FOPEN(FILENAME1, "a+");
	SAFE_FWRITE(buff_file1, 1, strlen(buff_file1), stream);

	tst_res(TINFO, "Write %s file streaming into %s file", FILENAME2, FILENAME1);
	stream = SAFE_FREOPEN(FILENAME2, "a+", stream);
	SAFE_FWRITE(buff_file2, 1, strlen(buff_file2), stream);

	SAFE_FCLOSE(stream);

	read_file(FILENAME1, buff_file1, 3);
	read_file(FILENAME2, buff_file2, 3);

	SAFE_UNLINK(FILENAME1);
	SAFE_UNLINK(FILENAME2);
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
