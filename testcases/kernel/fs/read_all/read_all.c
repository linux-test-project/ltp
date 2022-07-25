// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2022 Richard Palethorpe <rpalethorpe@suse.com>
 */
/*
 * Perform a small read on every file in a directory tree.
 *
 * Useful for testing file systems like proc, sysfs and debugfs or
 * anything which exposes a file like API. This test is not concerned
 * if a particular file in one of these file systems conforms exactly
 * to its specific documented behavior. Just whether reading from that
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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fnmatch.h>
#include <lapi/fnmatch.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <semaphore.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include "tst_atomic.h"
#include "tst_safe_clocks.h"
#include "tst_test.h"
#include "tst_timer.h"

#define QUEUE_SIZE 16384
#define BUFFER_SIZE 1024
#define MAX_PATH 4096
#define MAX_DISPLAY 40

struct queue {
	sem_t sem;
	int front;
	int back;
	char data[QUEUE_SIZE];
	char popped[BUFFER_SIZE];
};

struct worker {
	int i;
	pid_t pid;
	struct queue *q;
	int last_seen;
	unsigned int kill_sent:1;
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
static char *str_worker_timeout;
static int worker_timeout;
static int timeout_warnings_left = 15;

static char *blacklist[] = {
	NULL, /* reserved for -e parameter */
	"/sys/kernel/debug/*",
	"/sys/devices/platform/*/eeprom",
	"/sys/devices/platform/*/nvmem",
	"/sys/*/cpu??*(?)/*",	/* cpu* entries with 2 or more digits */
};

static long long epoch;

static int atomic_timestamp(void)
{
	struct timespec now;

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC_RAW, &now);

	return tst_timespec_to_us(now) - epoch;
}

static int queue_pop(struct queue *q)
{
	int i = q->front, j = 0;

	sem_wait(&q->sem);

	if (!q->data[i])
		return 0;

	while (q->data[i]) {
		q->popped[j] = q->data[i];

		if (++j >= BUFFER_SIZE - 1)
			tst_brk(TBROK, "Buffer is too small for path");

		 i = (i + 1) % QUEUE_SIZE;
	}

	q->popped[j] = '\0';
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

	for (i = 0; i < MIN(count, (ssize_t)MAX_DISPLAY); i++)
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

static void worker_heartbeat(const int worker)
{
	tst_atomic_store(atomic_timestamp(), &workers[worker].last_seen);
}

static int worker_elapsed(const int worker)
{
	struct worker *const w = workers + worker;

	return atomic_timestamp() - tst_atomic_load(&w->last_seen);
}

static int worker_ttl(const int worker)
{
	return MAX(0, worker_timeout - worker_elapsed(worker));
}

static void read_test(const int worker, const char *const path)
{
	char buf[BUFFER_SIZE];
	int fd;
	ssize_t count;
	const pid_t pid = workers[worker].pid;
	int elapsed;

	if (is_blacklisted(path))
		return;

	if (verbose)
		tst_res(TINFO, "Worker %d: %s(%s)", pid, __func__, path);

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		if (!quiet) {
			tst_res(TINFO | TERRNO, "Worker %d (%d): open(%s)",
				pid, worker, path);
		}
		return;
	}

	worker_heartbeat(worker);
	count = read(fd, buf, sizeof(buf) - 1);
	elapsed = worker_elapsed(worker);

	if (count > 0 && verbose) {
		sanitize_str(buf, count);
		tst_res(TINFO,
			"Worker %d (%d): read(%s, buf) = %zi, buf = %s, elapsed = %dus",
			pid, worker, path, count, buf, elapsed);
	} else if (!count && verbose) {
		tst_res(TINFO,
			"Worker %d (%d): read(%s) = EOF, elapsed = %dus",
			pid, worker, path, elapsed);
	} else if (count < 0 && !quiet) {
		tst_res(TINFO | TERRNO,
			"Worker %d (%d): read(%s), elapsed = %dus",
			pid, worker, path, elapsed);
	}

	SAFE_CLOSE(fd);
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

static int worker_run(int worker)
{
	struct sigaction term_sa = {
		.sa_handler = SIG_IGN,
		.sa_flags = 0,
	};
	struct worker *const self = workers + worker;
	struct queue *q = self->q;

	sigaction(SIGTTIN, &term_sa, NULL);
	maybe_drop_privs();
	self->pid = getpid();

	if (!worker_ttl(self->i)) {
		tst_brk(TBROK,
			"Worker timeout is too short; restarts take >%dus",
			worker_elapsed(self->i));
	}

	while (1) {
		worker_heartbeat(worker);

		if (!queue_pop(q))
			break;

		read_test(worker, q->popped);
	}

	queue_destroy(q, 1);
	tst_flush();
	return 0;
}

static void spawn_workers(void)
{
	int i;
	struct worker *wa = workers;

	memset(workers, 0, worker_count * sizeof(*workers));

	for (i = 0; i < worker_count; i++) {
		wa[i].i = i;
		wa[i].q = queue_init();
		wa[i].last_seen = atomic_timestamp();
		wa[i].pid = SAFE_FORK();
		if (!wa[i].pid)
			exit(worker_run(i));
	}
}

static void restart_worker(const int worker)
{
	struct worker *const w = workers + worker;
	int wstatus, ret, i, q_len;

	if (!w->kill_sent) {
		SAFE_KILL(w->pid, SIGKILL);
		w->kill_sent = 1;
		worker_heartbeat(worker);
	}

	ret = waitpid(w->pid, &wstatus, WNOHANG);

	if (!ret) {
		if (worker_ttl(worker) > 0)
			return;

		if (!quiet || timeout_warnings_left) {
			tst_res(TINFO,
				"Worker %d (%d): Timeout waiting after kill",
				w->pid, worker);
		}
	} else if (ret != w->pid) {
		tst_brk(TBROK | TERRNO, "Worker %d (%d): waitpid = %d",
			w->pid, worker, ret);
	}

	w->kill_sent = 0;

	if (!w->q->popped[0]) {
		tst_brk(TBROK,
			"Worker %d (%d): Timed out, but doesn't appear to be reading anything",
			w->pid, worker);
	}

	if (!quiet || timeout_warnings_left) {
		tst_res(TINFO, "Worker %d (%d): Last popped '%s'",
			w->pid, worker, w->q->popped);
	}

	/* Make sure the queue length and semaphore match. Threre is a
	 * race in qeue_pop where the semaphore can be decremented
	 * then the worker killed before updating q->front
	 */
	q_len = 0;
	i = w->q->front;
	while (i != w->q->back) {
		if (!w->q->data[i])
			q_len++;

		i = (i + 1) % QUEUE_SIZE;
	}

	ret = sem_destroy(&w->q->sem);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "sem_destroy");
	ret = sem_init(&w->q->sem, 1, q_len);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "sem_init");

	worker_heartbeat(worker);
	w->pid = SAFE_FORK();

	if (!w->pid)
		exit(worker_run(worker));
}

static void check_timeout_warnings_limit(void)
{
	if (!quiet)
		return;

	timeout_warnings_left--;

	if (timeout_warnings_left)
		return;

	tst_res(TINFO,
		"Silencing timeout warnings; consider increasing LTP_RUNTIME_MUL or removing -q");
}

static int try_push_work(const int worker, const char *buf)
{
	int ret = 0;
	int elapsed;
	struct worker *const w = workers + worker;

	if (w->kill_sent) {
		restart_worker(worker);
		return 0;
	}

	ret = queue_push(w->q, buf);
	if (ret)
		return 1;

	elapsed = worker_elapsed(worker);

	if (elapsed > worker_timeout) {
		if (!quiet || timeout_warnings_left) {
			tst_res(TINFO,
				"Worker %d (%d): Stuck for %dus, restarting it",
				w->pid, worker, elapsed);
			check_timeout_warnings_limit();
		}
		restart_worker(worker);
	}

	return 0;
}

static void push_work(const int worker, const char *buf)
{
	int sleep_time = 1;

	while (!try_push_work(worker, buf)) {
		const int ttl = worker_ttl(worker);

		sleep_time = MIN(2 * sleep_time, ttl);
		usleep(sleep_time);
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
			push_work(i, stop_code);
	}
}

static void destroy_workers(void)
{
	int i;

	if (!workers)
		return;

	for (i = 0; i < worker_count; i++) {
		if (workers[i].q) {
			queue_destroy(workers[i].q, 0);
			workers[i].q = 0;
		}
	}
}

static int sched_work(const int first_worker,
		      const char *path, int repetitions)
{
	int i, j;
	int min_ttl = worker_timeout, sleep_time = 1;
	int pushed, workers_pushed = 0;

	for (i = 0, j = first_worker; i < repetitions; j++) {
		if (j >= worker_count)
			j = 0;

		if (j == first_worker && !workers_pushed) {
			sleep_time = MIN(2 * sleep_time, min_ttl);
			usleep(sleep_time);
			min_ttl = worker_timeout;
		}

		if (j == first_worker)
			workers_pushed = 0;

		pushed = try_push_work(j, path);
		i += pushed;
		workers_pushed += pushed;

		if (!pushed)
			min_ttl = MIN(min_ttl, worker_ttl(j));
	}

	return j;
}

static void setup(void)
{
	struct timespec now;

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
		worker_count = MIN(MAX(tst_ncpus() - 1, 1L), max_workers);
	workers = SAFE_MALLOC(worker_count * sizeof(*workers));

	if (tst_parse_int(str_worker_timeout, &worker_timeout, 1, INT_MAX)) {
		tst_brk(TBROK,
			"Invalid worker timeout (-t) argument: '%s'",
			str_worker_count);
	}

	if (worker_timeout) {
		tst_res(TINFO, "Worker timeout forcibly set to %dms",
			worker_timeout);
	} else {
		worker_timeout = 10 * tst_remaining_runtime();
		tst_res(TINFO, "Worker timeout set to 10%% of max_runtime: %dms",
			worker_timeout);
	}
	worker_timeout *= 1000;

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC_RAW, &now);
	epoch = tst_timespec_to_us(now);
}

static void reap_children(void)
{
	int status, bad_exit = 0;
	pid_t pid;

	for (;;) {
		pid = wait(&status);

		if (pid > 0) {
			if (!WIFEXITED(status))
				bad_exit = 1;

			continue;
		}

		if (errno == ECHILD)
			break;

		if (errno == EINTR)
			continue;

		tst_brk(TBROK | TERRNO, "wait() failed");
	}

	if (!bad_exit)
		return;

	tst_res(TINFO,
		"Zombie workers detected; consider increasing LTP_RUNTIME_MUL");
}

static void cleanup(void)
{
	stop_workers();
	reap_children();
	destroy_workers();
	free(workers);
}

static void visit_dir(const char *path)
{
	DIR *dir;
	struct dirent *dent;
	struct stat dent_st;
	char dent_path[MAX_PATH];
	enum dent_action act;
	int last_sched = 0;

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
			last_sched = sched_work(last_sched, dent_path, reads);
	}

	if (closedir(dir))
		tst_res(TINFO | TERRNO, "closedir(%s)", path);
}

static void run(void)
{
	spawn_workers();
	visit_dir(root_dir);

	stop_workers();
	reap_children();
	destroy_workers();

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
		{"t:", &str_worker_timeout,
		 "Milliseconds a worker has to read a file before it is restarted"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.max_runtime = 100,
};
