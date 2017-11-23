/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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
 */

/*
 * Runs several threads that fills up the filesystem repeatedly.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "tst_safe_pthread.h"
#include "tst_test.h"

#define MNTPOINT "mntpoint"

static volatile int run;
static unsigned int nthreads;
static int enospc_cnt;
static struct worker *workers;

struct worker {
	char dir[PATH_MAX];
};

static void *worker(void *p)
{
	struct worker *w = p;
	DIR *d;
	struct dirent *ent;
	char file[PATH_MAX];

	while (run) {
		tst_fill_fs(w->dir, 0);

		tst_atomic_inc(&enospc_cnt);

		d = SAFE_OPENDIR(w->dir);
		while ((ent = SAFE_READDIR(d))) {

			if (!strcmp(ent->d_name, ".") ||
			    !strcmp(ent->d_name, ".."))
				continue;

			snprintf(file, sizeof(file), "%s/%s",
				 w->dir, ent->d_name);

			tst_res(TINFO, "Unlinking %s", file);

			SAFE_UNLINK(file);
			break;
		}
		SAFE_CLOSEDIR(d);
	}

	return NULL;
}

static void testrun(void)
{
	pthread_t threads[nthreads];
	unsigned int i, ms;

	run = 1;
	for (i = 0; i < nthreads; i++)
		SAFE_PTHREAD_CREATE(&threads[i], NULL, worker, &workers[i]);

	for (ms = 0; ; ms++) {
		usleep(1000);

		if (ms >= 1000 && enospc_cnt)
			break;

		if (enospc_cnt > 100)
			break;
	}

	run = 0;
	for (i = 0; i < nthreads; i++)
		SAFE_PTHREAD_JOIN(threads[i], NULL);

	tst_res(TPASS, "Got %i ENOSPC runtime %ims", enospc_cnt, ms);
}

static void setup(void)
{
	unsigned int i;

	nthreads = tst_ncpus_conf() + 2;
	workers = SAFE_MALLOC(sizeof(struct worker) * nthreads);

	for (i = 0; i < nthreads; i++) {
		snprintf(workers[i].dir, sizeof(workers[i].dir),
			 MNTPOINT "/thread%i", i + 1);
		SAFE_MKDIR(workers[i].dir, 0700);
	}

	tst_res(TINFO, "Running %i writer threads", nthreads);
}

static void cleanup(void)
{
	free(workers);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = testrun,
};
