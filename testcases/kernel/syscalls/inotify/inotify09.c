/*
 * Copyright (c) 2018 SUSE Linux.  All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Started by Jan Kara <jack@suse.cz>
 * Chnaged to use fzsync library by Cyril Hrubis <chrubis@suse.cz>
 *
 * DESCRIPTION
 * Test for inotify mark connector destruction race.
 *
 * Kernels prior to 4.17 have a race when the last fsnotify mark on the inode
 * is being deleted while another process reports event happening on that
 * inode. When the race is hit, the kernel crashes or loops.
 *
 * The problem has been fixed by commit:
 * d90a10e2444b "fsnotify: Fix fsnotify_mark_connector race"
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "tst_fuzzy_sync.h"
#include "inotify.h"

#define FNAME "stress_fname"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

static struct tst_fzsync_pair fzsync_pair;
static int fd;

static void *write_seek(void *unused)
{
	char buf[64];

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		SAFE_WRITE(0, fd, buf, sizeof(buf));
		SAFE_LSEEK(fd, 0, SEEK_SET);
		tst_fzsync_end_race_b(&fzsync_pair);
	}
	return unused;
}

static void setup(void)
{
	fd = SAFE_OPEN(FNAME, O_CREAT | O_RDWR, 0600);
	tst_fzsync_pair_init(&fzsync_pair);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	tst_fzsync_pair_cleanup(&fzsync_pair);
}

static void verify_inotify(void)
{
	int inotify_fd;
	int wd;

	inotify_fd = myinotify_init1(0);
	if (inotify_fd < 0)
		tst_brk(TBROK | TERRNO, "inotify_init failed");

	tst_fzsync_pair_reset(&fzsync_pair, write_seek);
	while (tst_fzsync_run_a(&fzsync_pair)) {
		wd = myinotify_add_watch(inotify_fd, FNAME, IN_MODIFY);
		if (wd < 0)
			tst_brk(TBROK | TERRNO, "inotify_add_watch() failed.");

		tst_fzsync_start_race_a(&fzsync_pair);
		wd = myinotify_rm_watch(inotify_fd, wd);
		tst_fzsync_end_race_a(&fzsync_pair);
		if (wd < 0)
			tst_brk(TBROK | TERRNO, "inotify_rm_watch() failed.");
	}
	SAFE_CLOSE(inotify_fd);
	/* We survived for given time - test succeeded */
	tst_res(TPASS, "kernel survived inotify beating");
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
