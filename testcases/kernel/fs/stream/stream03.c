// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	ported from SPIE, section2/filesuite/stream3.c, by Airong Zhang
 *
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that `ftell()` reports the correct current byte offset after
 * moving it.
 */

#include "tst_test.h"
#include "tst_rand_data.h"
#include "tst_safe_stdio.h"

#define FILENAME "ltp_file"
#define DATASIZE 30
#define BUFFSIZE 10

static char *data;
static char *buff;

static void run(void)
{
	FILE *stream;

	memset(buff, 0, BUFFSIZE);

	stream = SAFE_FOPEN(FILENAME, "a+");
	TST_EXP_EQ_LI(SAFE_FTELL(stream), 0);

	SAFE_FWRITE(data, 1, DATASIZE, stream);
	TST_EXP_EQ_LI(SAFE_FTELL(stream), DATASIZE);

	rewind(stream);
	TST_EXP_EQ_LI(SAFE_FTELL(stream), SEEK_SET);

	SAFE_FSEEK(stream, 10, SEEK_CUR);
	TST_EXP_EQ_LI(SAFE_FTELL(stream), 10);

	SAFE_FSEEK(stream, 0, SEEK_END);
	TST_EXP_EQ_LI(SAFE_FTELL(stream), DATASIZE);

	SAFE_FSEEK(stream, 0, SEEK_SET);
	TST_EXP_EQ_LI(SAFE_FTELL(stream), 0);

	while (fgets(buff, BUFFSIZE, stream))
		;
	TST_EXP_EQ_LI(SAFE_FTELL(stream), DATASIZE);

	SAFE_FCLOSE(stream);
	SAFE_UNLINK(FILENAME);
}

static void setup(void)
{
	memcpy(data, tst_rand_data, DATASIZE);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers[]) {
		{&data, .size = DATASIZE},
		{&buff, .size = BUFFSIZE},
		{},
	},
};
