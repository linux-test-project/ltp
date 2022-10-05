// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * AUTHOR: Barrie Kletscher
 * CO-PILOT: Dave Baumgartner
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <errno.h>
#include <stdlib.h>
#include "tst_test.h"

/* Temporary file names */
#define FNAME1	"stat02.1"
#define FNAME2	"stat02.2"

/* Number of times each buffer is written */
#define NUM_WRITES	(10)
#define BUFSIZE		(4096)

char *buffer;

static struct test_case {
	const char *filename;
	size_t bytes_to_write;
	size_t decrement;
} tcases[] = {
	/* Write and verify BUFSIZE byte chunks */
	{ FNAME1, BUFSIZE, BUFSIZE },
	/* Write and verify decreasing chunk sizes */
	{ FNAME2, BUFSIZE, 1000 }
};

void verify(const char *fname, size_t bytes, size_t decrement)
{
	struct stat s;
	int fd, i;
	size_t bytes_written = 0;

	fd = SAFE_OPEN(fname, O_CREAT | O_TRUNC | O_RDWR, 0777);
	while (bytes > 0) {
		for (i = 0; i < NUM_WRITES; i++) {
			SAFE_WRITE(SAFE_WRITE_ALL, fd, buffer, bytes);
			bytes_written += bytes;
		}
		bytes -= bytes > decrement ? decrement : bytes;
	}

	SAFE_CLOSE(fd);
	SAFE_STAT(fname, &s);
	SAFE_UNLINK(fname);

	/*
	 *  Now check to see if the number of bytes written was
	 *  the same as the number of bytes in the file.
	 */
	if (s.st_size != (off_t) bytes_written) {
		tst_res(TFAIL, "file size (%zu) not as expected (%zu) bytes",
				s.st_size, bytes_written);
		return;
	}

	tst_res(TPASS, "File size reported as expected");
}

void verify_stat_size(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	verify(tcase->filename, tcase->bytes_to_write, tcase->decrement);
}

void setup(void)
{
	buffer = SAFE_MALLOC(BUFSIZE);
}

void cleanup(void)
{
	free(buffer);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.test = verify_stat_size,
	.setup = setup,
	.cleanup = cleanup,
};
