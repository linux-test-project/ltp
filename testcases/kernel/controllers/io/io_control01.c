// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC <rpalethorpe@suse.com>
 */
/*\
 *
 * Perform some I/O on a file and check if at least some of it is
 * recorded by the I/O controller.
 *
 * The exact amount of I/O performed is dependent on the file system,
 * page cache, scheduler and block driver. We call sync and drop the
 * file's page cache to force reading and writing. We also write
 * random data to try to prevent compression.
 *
 * The pagecache is a particular issue for reading. If the call to
 * fadvise is ignored then the data may only be read from the
 * cache. So that no I/O requests are made.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>

#include "tst_test.h"

struct io_stats {
	unsigned int mjr;
	unsigned int mnr;
	unsigned long rbytes;
	unsigned long wbytes;
	unsigned long rios;
	unsigned long wios;
	unsigned long dbytes;
	unsigned long dios;
};

static unsigned int dev_major, dev_minor;

static int read_io_stats(const char *const line, struct io_stats *const stat)
{
	return sscanf(line,
		      "%u:%u rbytes=%lu wbytes=%lu rios=%lu wios=%lu dbytes=%lu dios=%lu",
		      &stat->mjr, &stat->mnr,
		      &stat->rbytes, &stat->wbytes, &stat->rios, &stat->wios,
		      &stat->dbytes, &stat->dios);
}

static void run(void)
{
	int i, fd;
	char *line, *buf_ptr;
	const size_t pgsz = SAFE_SYSCONF(_SC_PAGESIZE);
	char *buf = SAFE_MALLOC(MAX((size_t)BUFSIZ, pgsz));
	struct io_stats start;

	memset(&start, 0, sizeof(struct io_stats));
	SAFE_CG_READ(tst_cg, "io.stat", buf, BUFSIZ - 1);
	line = strtok_r(buf, "\n", &buf_ptr);
	while (line) {
		const int convs = read_io_stats(line, &start);

		if (convs < 2)
			continue;

		if (start.mjr == dev_major && start.mnr == dev_minor) {
			tst_res(TINFO, "Found %u:%u in io.stat", dev_major, dev_minor);
			break;
		}
		line = strtok_r(NULL, "\n", &buf_ptr);
	}

	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());

	fd = SAFE_OPEN("/dev/urandom", O_RDONLY, 0600);
	SAFE_READ(1, fd, buf, pgsz);
	SAFE_CLOSE(fd);

	fd = SAFE_OPEN("mnt/dat", O_WRONLY | O_CREAT, 0600);

	for (i = 0; i < 4; i++) {
		SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, pgsz);
		SAFE_FSYNC(fd);
		TST_EXP_PASS_SILENT(posix_fadvise(fd, pgsz * i, pgsz, POSIX_FADV_DONTNEED));
	}

	SAFE_CLOSE(fd);
	fd = SAFE_OPEN("mnt/dat", O_RDONLY, 0600);

	for (i = 0; i < 4; i++)
		SAFE_READ(1, fd, buf, pgsz);

	tst_res(TPASS, "Did some IO in the IO controller");

	SAFE_CG_READ(tst_cg, "io.stat", buf, BUFSIZ - 1);
	line = strtok_r(buf, "\n", &buf_ptr);
	while (line) {
		struct io_stats end;
		const int convs = read_io_stats(line, &end);

		if (convs < 8)
			break;

		if (end.mjr != dev_major || end.mnr != dev_minor) {
			line = strtok_r(NULL, "\n", &buf_ptr);
			continue;
		}

		tst_res(TPASS, "Found %u:%u in io.stat", dev_major, dev_minor);
		TST_EXP_EXPR(end.rbytes > start.rbytes,
			     "(rbytes=%lu) > (st_rbytes=%lu)",
			     end.rbytes, start.rbytes);
		TST_EXP_EXPR(end.wbytes > start.wbytes,
			     "(wbytes=%lu) > (st_wbytes=%lu)",
			     end.wbytes, start.wbytes);
		TST_EXP_EXPR(end.rios > start.rios,
			     "(rios=%lu) > (st_rios=%lu)",
			     end.rios, start.rios);
		TST_EXP_EXPR(end.wios > start.wios,
			     "(wios=%lu) > (st_wios=%lu)",
			     end.wios, start.wios);

		goto out;
	}

	tst_res(TINFO, "io.stat:\n%s", buf);
	tst_res(TFAIL, "Did not find %u:%u in io.stat", dev_major, dev_minor);
out:
	free(buf);
	SAFE_CLOSE(fd);
	SAFE_UNLINK("mnt/dat");
}

static void setup(void)
{
	char buf[PATH_MAX] = { 0 };
	char *path = SAFE_GETCWD(buf, PATH_MAX - sizeof("mnt") - 1);
	struct stat st;

	strcpy(path + strlen(path), "/mnt");

	tst_stat_mount_dev(path, &st);
	dev_major = major(st.st_rdev);
	dev_minor = minor(st.st_rdev);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.mntpoint = "mnt",
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]){ "ntfs", "tmpfs", NULL },
	.needs_cgroup_ver = TST_CG_V2,
	.needs_cgroup_ctrls = (const char *const[]){ "io", NULL },
};
