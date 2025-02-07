// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2000 Juan Quintela <quintela@fi.udc.es>
 *                    Aaron Laffin <alaffin@sgi.com>
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that we can use mmap() to map a large file, write to it via memory
 * access, and read back the data from the file.
 */

#include "tst_test.h"

#define FILE_NAME "testfile"

static char *str_pages;
static long long pages = 1000;
static long long memory_size;
static char *memory_data;
static char *buff;

static void run(void)
{
	int fd;
	pid_t pid;

	tst_res(TINFO, "mmap()ing file of %llu bytes", memory_size);

	fd = SAFE_OPEN(FILE_NAME, O_RDWR | O_CREAT, 0666);
	SAFE_LSEEK(fd, memory_size, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "\0", 1);

	memory_data = SAFE_MMAP(0, memory_size, PROT_WRITE, MAP_SHARED, fd, 0);

	pid = SAFE_FORK();
	if (!pid) {
		tst_res(TINFO, "Touching mapped memory");

		for (int i = 0; i < memory_size; i++)
			memory_data[i] = (char)i;

		exit(0);
	}

	tst_reap_children();

	SAFE_MSYNC(memory_data, memory_size, MS_SYNC);

	memset(buff, 0, memory_size);

	SAFE_LSEEK(fd, 0, SEEK_SET);
	SAFE_READ(0, fd, buff, memory_size);
	SAFE_CLOSE(fd);

	for (int i = 0; i < memory_size; i++) {
		if (buff[i] != (char)i) {
			tst_res(TFAIL, "Mapped file has not been updated at byte %d", i);
			goto exit;
		}
	}

	tst_res(TPASS, "Mapped file has been updated");

exit:
	SAFE_MUNMAP(memory_data, memory_size);
	memory_data = NULL;

	SAFE_UNLINK(FILE_NAME);
}

static void setup(void)
{
	if (tst_parse_filesize(str_pages, &pages, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid number of pages: %s", str_pages);

	memory_size = pages * getpagesize();

	buff = SAFE_MALLOC(memory_size);
}

static void cleanup(void)
{
	if (buff)
		free(buff);

	if (memory_data)
		SAFE_MUNMAP(memory_data, memory_size);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"m:", &str_pages, "Number of pages (default 1000)"},
		{}
	},
};
