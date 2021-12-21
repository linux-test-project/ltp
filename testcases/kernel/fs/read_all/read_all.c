// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 */
/*
 * Perform a small read on every file in a directory tree.
 *
 * Useful for testing file systems like proc, sysfs and debugfs or anything
 * which exposes a file like API so long as it respects O_NONBLOCK. This test
 * is not concerned if a particular file in one of these file systems conforms
 * exactly to its specific documented behavior. Just whether reading from that
 * file causes a serious error such as a NULL pointer dereference.
 *
 * It is not required to run this as root, but test coverage will be much
 * higher with full privileges.
 *
 * The reads are preformed by worker processes which are given file paths by a
 * single parent process. The parent process recursively scans a given
 * directory and passes the file paths it finds to the child processes using a
 * queue structure stored in shared memory.
 *
 * This allows the file system and individual files to be accessed in
 * parallel. Passing the 'reads' parameter (-r) will encourage this. The
 * number of worker processes is based on the number of available
 * processors. However this is limited by default to 15 to avoid this becoming
 * an IPC stress test on systems with large numbers of weak cores. This can be
 * overridden with the 'w' parameters.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <lapi/fnmatch.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fnmatch.h>
#include <semaphore.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include "tst_test.h"

#define QUEUE_SIZE 16384
#define BUFFER_SIZE 1024
#define MAX_PATH 4096
#define MAX_DISPLAY 40

struct queue {
	sem_t sem;
	int front;
	int back;
	char data[QUEUE_SIZE];
};

struct worker {
	pid_t pid;
	struct queue *q;
};

enum dent_action {
	DA_UNKNOWN,
	DA_IGNORE,
	DA_READ,
	DA_VISIT,
};

static char *verbose;
static char *quiet;
static char *root_dir;
static char *str_reads;
static int reads = 1;
static char *str_worker_count;
static long worker_count;
static char *str_max_workers;
static long max_workers = 15;
static struct worker *workers;
static char *drop_privs;

static char *blacklist[] = {
	NULL, /* reserved for -e parameter */
	"/sys/power/wakeup_count",
	"/sys/kernel/debug/*",
	"/sys/devices/platform/*/eeprom",
	"/sys/devices/platform/*/nvmem",
	"/sys/*/cpu??*(?)/*",	/* cpu* entries with 2 or more digits */
};

static int queue_pop(struct queue *q, char *buf)
{
	int i = q->front, j = 0;

	sem_wait(&q->sem);

	if (!q->data[i])
		return 0;

	while (q->data[i]) {
		buf[j] = q->data[i];

		if (++j >= BUFFER_SIZE - 1)
			tst_brk(TBROK, "Buffer is too small for path");

		 i = (i + 1) % QUEUE_SIZE;
	}

	buf[j] = '\0';
	tst_atomic_store((i + 1) % QUEUE_SIZE, &q->front);

	return 1;
}

static int queue_push(struct queue *q, const char *buf)
{
	int i = q->back, j = 0;
	int front = tst_atomic_load(&q->front);

	do {
		q->data[i] = buf[j];

		i = (i + 1) % QUEUE_SIZE;

		if (i == front)
			return 0;

	} while (buf[j++]);

	q->back = i;
	sem_post(&q->sem);

	return 1;
}

static struct queue *queue_init(void)
{
	struct queue *q = SAFE_MMAP(NULL, sizeof(*q),
				    PROT_READ | PROT_WRITE,
				    MAP_SHARED | MAP_ANONYMOUS,
				    0, 0);

	sem_init(&q->sem, 1, 0);
	q->front = 0;
	q->back = 0;

	return q;
}

static void queue_destroy(struct queue *q, int is_worker)
{
	if (is_worker)
		sem_destroy(&q->sem);

	SAFE_MUNMAP(q, sizeof(*q));
}

static void sanitize_str(char *buf, ssize_t count)
{
	int i;

	for (i = 0; i < MIN(count, MAX_DISPLAY); i++)
		if (!isprint(buf[i]))
			buf[i] = ' ';

	if (count <= MAX_DISPLAY)
		buf[count] = '\0';
	else
		strcpy(buf + MAX_DISPLAY, "...");
}

static int is_blacklisted(const char *path)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(blacklist); i++) {
		if (blacklist[i] && !fnmatch(blacklist[i], path, FNM_EXTMATCH)) {
			if (verbose)
				tst_res(TINFO, "Ignoring %s", path);
			return 1;
		}
	}

	return 0;
}

static void read_test(const char *path)
{
	char buf[BUFFER_SIZE];
	int fd;
	ssize_t count;

	if (is_blacklisted(path))
		return;

	if (verbose)
		tst_res(TINFO, "%s(%s)", __func__, path);

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		if (!quiet)
			tst_res(TINFO | TERRNO, "open(%s)", path);
		return;
	}

	count = read(fd, buf, sizeof(buf) - 1);
	if (count > 0 && verbose) {
		sanitize_str(buf, count);
		tst_res(TINFO, "read(%s, buf) = %zi, buf = %s",
			path, count, buf);
	} else if (!count && verbose) {
		tst_res(TINFO, "read(%s) = EOF", path);
	} else if (count < 0 && !quiet) {
		tst_res(TINFO | TERRNO, "read(%s)", path);
	}

	SAFE_CLOSE(fd);
}

static int worker_run(struct worker *self)
{
	char buf[BUFFER_SIZE];
	struct sigaction term_sa = {
		.sa_handler = SIG_IGN,
		.sa_flags = 0,
	};
	struct queue *q = self->q;

	sigaction(SIGTTIN, &term_sa, NULL);

	while (1) {
		if (!queue_pop(q, buf))
			break;

		read_test(buf);
	}

	queue_destroy(q, 1);
	tst_flush();
	return 0;
}

static void maybe_drop_privs(void)
{
	struct passwd *nobody;

	if (!drop_privs)
		return;

	TEST(setgroups(0, NULL));
	if (TST_RET < 0 && TST_ERR != EPERM) {
		tst_brk(TBROK | TTERRNO,
			"Failed to clear suplementary group set");
	}

	nobody = SAFE_GETPWNAM("nobody");

	TEST(setgid(nobody->pw_gid));
	if (TST_RET < 0 && TST_ERR != EPERM)
		tst_brk(TBROK | TTERRNO, "Failed to use nobody gid");

	TEST(setuid(nobody->pw_uid));
	if (TST_RET < 0 && TST_ERR != EPERM)
		tst_brk(TBROK | TTERRNO, "Failed to use nobody uid");
}

static void spawn_workers(void)
{
	int i;
	struct worker *wa = workers;

	memset(workers, 0, worker_count * sizeof(*workers));

	for (i = 0; i < worker_count; i++) {
		wa[i].q = queue_init();
		wa[i].pid = SAFE_FORK();
		if (!wa[i].pid) {
			maybe_drop_privs();
			exit(worker_run(wa + i));
		}
	}
}

static void work_push_retry(int worker, const char *buf)
{
	int i, ret, worker_min, worker_max, usleep_time = 100;

	if (worker < 0) {
		/* pick any, try -worker first */
		worker_min = worker * (-1);
		worker_max = worker_count;
	} else {
		/* keep trying worker */
		worker_min = worker;
		worker_max = worker + 1;
	}
	i = worker_min;

	for (;;) {
		ret = queue_push(workers[i].q, buf);
		if (ret == 1)
			break;

		if (++i >= worker_max) {
			i = worker_min;
			if (usleep_time < 100000)
				usleep_time *= 2;
			usleep(usleep_time);
		}
	}
}

static void stop_workers(void)
{
	const char stop_code[1] = { '\0' };
	int i;

	if (!workers)
		return;

	for (i = 0; i < worker_count; i++) {
		if (workers[i].q)
			work_push_retry(i, stop_code);
	}

	for (i = 0; i < worker_count; i++) {
		if (workers[i].q) {
			queue_destroy(workers[i].q, 0);
			workers[i].q = 0;
		}
	}
}

static void rep_sched_work(const char *path, int rep)
{
	int i, j;

	for (i = j = 0; i < rep; i++, j++) {
		if (j >= worker_count)
			j = 0;
		work_push_retry(-j, path);
	}
}

static void setup(void)
{
	if (tst_parse_int(str_reads, &reads, 1, INT_MAX))
		tst_brk(TBROK,
			"Invalid reads (-r) argument: '%s'", str_reads);

	if (tst_parse_long(str_max_workers, &max_workers, 1, LONG_MAX)) {
		tst_brk(TBROK,
			"Invalid max workers (-w) argument: '%s'",
			str_max_workers);
	}

	if (tst_parse_long(str_worker_count, &worker_count, 1, LONG_MAX)) {
		tst_brk(TBROK,
			"Invalid worker count (-W) argument: '%s'",
			str_worker_count);
	}

	if (!root_dir)
		tst_brk(TBROK, "The directory argument (-d) is required");

	if (!worker_count)
		worker_count = MIN(MAX(tst_ncpus() - 1, 1), max_workers);
	workers = SAFE_MALLOC(worker_count * sizeof(*workers));
}

static void cleanup(void)
{
	stop_workers();
	free(workers);
}

static void visit_dir(const char *path)
{
	DIR *dir;
	struct dirent *dent;
	struct stat dent_st;
	char dent_path[MAX_PATH];
	enum dent_action act;

	dir = opendir(path);
	if (!dir) {
		tst_res(TINFO | TERRNO, "opendir(%s)", path);
		return;
	}

	while (1) {
		errno = 0;
		dent = readdir(dir);
		if (!dent && errno) {
			tst_res(TINFO | TERRNO, "readdir(%s)", path);
			break;
		} else if (!dent) {
			break;
		}

		if (!strcmp(dent->d_name, ".") ||
		    !strcmp(dent->d_name, ".."))
			continue;

		if (dent->d_type == DT_DIR)
			act = DA_VISIT;
		else if (dent->d_type == DT_LNK)
			act = DA_IGNORE;
		else if (dent->d_type == DT_UNKNOWN)
			act = DA_UNKNOWN;
		else
			act = DA_READ;

		snprintf(dent_path, MAX_PATH,
			 "%s/%s", path, dent->d_name);

		if (act == DA_UNKNOWN) {
			if (lstat(dent_path, &dent_st))
				tst_res(TINFO | TERRNO, "lstat(%s)", path);
			else if ((dent_st.st_mode & S_IFMT) == S_IFDIR)
				act = DA_VISIT;
			else if ((dent_st.st_mode & S_IFMT) == S_IFLNK)
				act = DA_IGNORE;
			else
				act = DA_READ;
		}

		if (act == DA_VISIT)
			visit_dir(dent_path);
		else if (act == DA_READ)
			rep_sched_work(dent_path, reads);
	}

	if (closedir(dir))
		tst_res(TINFO | TERRNO, "closedir(%s)", path);
}

static void run(void)
{
	spawn_workers();
	visit_dir(root_dir);
	stop_workers();

	tst_reap_children();
	tst_res(TPASS, "Finished reading files");
}

static struct tst_test test = {
	.options = (struct tst_option[]) {
		{"v", &verbose,
		 "Print information about successful reads."},
		{"q", &quiet,
		 "Don't print file read or open errors."},
		{"d:", &root_dir,
		 "Path to the directory to read from, defaults to /sys."},
		{"e:", &blacklist[0],
		 "Pattern Ignore files which match an 'extended' pattern, see fnmatch(3)."},
		{"r:", &str_reads,
		 "Count The number of times to schedule a file for reading."},
		{"w:", &str_max_workers,
		 "Count Set the worker count limit, the default is 15."},
		{"W:", &str_worker_count,
		 "Count Override the worker count. Ignores (-w) and the processor count."},
		{"p", &drop_privs,
		 "Drop privileges; switch to the nobody user."},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
};

