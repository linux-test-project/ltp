// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	ported from SPIE, section2/filesuite/stream4.c, by Airong Zhang
 *
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Ensure that `fwrite()` is appending data to stream and `fread()`
 * `fwrite()` are returning the right data.
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define FILENAME "ltp_file"
#define DATA "abcdefghijklmnopqrstuvwxyz"
#define DATASIZE sizeof(DATA)

static char *data;
static char *buff;

static void run(void)
{
	FILE *stream;

	memset(buff, 0, DATASIZE);

	stream = SAFE_FOPEN(FILENAME, "a+");
	TST_EXP_EQ_LI(fwrite(data, 1, DATASIZE, stream), DATASIZE);
	SAFE_FCLOSE(stream);

	stream = SAFE_FOPEN(FILENAME, "r+");
	TST_EXP_EQ_LI(fread(buff, 1, DATASIZE, stream), DATASIZE);
	SAFE_FCLOSE(stream);

	SAFE_UNLINK(FILENAME);

	TST_EXP_EQ_STRN(data, buff, DATASIZE);
}

static void setup(void)
{
	memcpy(data, DATA, DATASIZE);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&data, .size = DATASIZE},
		{&buff, .size = DATASIZE},
		{},
	},
};
