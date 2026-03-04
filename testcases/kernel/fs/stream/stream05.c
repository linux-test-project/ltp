// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	ported from SPIE section2/filesuite/stream.c, by Airong Zhang
 *
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that it's possible to read/write on a file descriptor returned by
 * `fileno()` and to close it, leaving `fclose()` to raise EBADF error.
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_rand_data.h"

#define FILENAME "ltp_file"
#define DATASIZE 3
#define BUFFSIZE (2 * DATASIZE)

static char *data;
static char *buff;

static void run(void)
{
	int fd;
	FILE *stream;

	memset(buff, 0, BUFFSIZE);

	stream = SAFE_FOPEN(FILENAME, "a+");
	SAFE_FWRITE(data, 1, DATASIZE, stream);
	SAFE_FFLUSH(stream);

	fd = SAFE_FILENO(stream);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, data, DATASIZE);

	SAFE_FSYNC(fd);
	SAFE_FSEEK(stream, 0, SEEK_SET);
	SAFE_READ(1, fd, buff, BUFFSIZE);

	TST_EXP_EQ_STRN(data, buff, DATASIZE);
	TST_EXP_EQ_STRN(data, buff + 3, DATASIZE);

	SAFE_CLOSE(fd);
	TST_EXP_FAIL(fclose(stream), EBADF);

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
	.bufs = (struct tst_buffers []) {
		{&data, .size = DATASIZE},
		{&buff, .size = BUFFSIZE},
		{},
	},
};
