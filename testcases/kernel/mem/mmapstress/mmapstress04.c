// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Red Hat, Inc.
 * 01/02/2003	Port to LTP avenkat@us.ibm.com
 * 06/30/2001	Port to Linux	nsharoff@us.ibm.com
 */
/*
 * Mmap parts of a file, and then write to the file causing single
 * write requests to jump back and forth between mmaped io and regular io.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#define NUM_PAGES (192)
#define TEST_FILE "mmapstress04-testfile"

static int page_size;
static unsigned char *mmap_area;

static void setup(void)
{
	page_size = getpagesize();

	/*
	 * Pick large enough area, PROT_NONE doesn't matter,
	 * because we remap it later.
	 */
	mmap_area = SAFE_MMAP(NULL, page_size * NUM_PAGES, PROT_NONE,
		MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

static void write_fully(int fd, void *buf, int len)
{
	int ret;

	do {
		ret = SAFE_WRITE(0, fd, buf, len);
		buf += ret;
		len -= ret;
	} while (len > 0);
}

static void mmapstress04(void)
{
	int i, j, rofd, rwfd;
	char *buf;
	int mapped_pages = 0;

	if (tst_fill_file(TEST_FILE, 'b', page_size, 1))
		tst_brk(TBROK | TERRNO, "fill_file");

	rofd = SAFE_OPEN(TEST_FILE, O_RDONLY | O_CREAT, 0777);
	/*
	 * Assuming disk blocks are 8k, and logical pages are 4k, there are
	 * two maps per block. In order to test mapping at the beginning and
	 * ends of the block, mapping the whole block, or none of the block
	 * with different mappings on preceding and following blocks, each
	 * 3 blocks with 6 pages can be thought of as a binary number from 0 to
	 * 64 with a bit set for mapped or cleared for unmapped. This number
	 * is represented by i. The value j is used to look at the bits of i
	 * and decided to map the page or not.
	 * NOTE: None of the above assumptions are critical.
	 */
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 6; j++) {
			off_t mapoff;

			if (!(i & (1 << j)))
				continue;

			mapoff = page_size * (off_t)(6 * i + j);
			SAFE_MMAP(mmap_area + page_size * mapped_pages++,
				 page_size, PROT_READ,
				 MAP_FILE | MAP_PRIVATE | MAP_FIXED,
				 rofd, mapoff);
		}
	}
	SAFE_CLOSE(rofd);

	/* write out 6 pages of stuff into each of the 64 six page sections */
	rwfd = SAFE_OPEN(TEST_FILE, O_RDWR);
	buf = SAFE_MALLOC(page_size);
	memset(buf, 'a', page_size);
	for (i = 0; i < 6 * 64; i++)
		write_fully(rwfd, buf, page_size);
	free(buf);
	SAFE_CLOSE(rwfd);

	/*
	 * Just finished scribbling all over interwoven mmapped and unmapped
	 * regions. Check the data.
	 */
	for (i = 0; i < mapped_pages * page_size; i++) {
		unsigned char val = *(mmap_area + i);

		if (val != 'a') {
			tst_res(TFAIL, "unexpected value in map, "
				"i=%d,val=0x%x", i, val);
			goto done;
		}
	}
	tst_res(TPASS, "blocks have expected data");
done:
	SAFE_UNLINK(TEST_FILE);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = mmapstress04,
	.setup = setup,
};
