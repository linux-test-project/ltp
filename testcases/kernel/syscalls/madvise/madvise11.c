// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Oracle and/or its affiliates.
 */

/*\
 * [Description]
 *
 * Stress a possible race condition between memory pages allocation
 * and soft-offline of unrelated pages as explained in the commit:
 *   d4ae9916ea29 (mm: soft-offline: close the race against page allocation)
 *
 * Control that soft-offlined pages get correctly replaced: with the
 * same content and without SIGBUS generation when accessed.
 */

#include <errno.h>
#include <mntent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/klog.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "tst_safe_stdio.h"
#include "lapi/mmap.h"

#define NUM_LOOPS	5
#define NUM_PAGES	32
#define NUM_PAGES_OFFSET	5

/* Needed module to online back memory pages */
#define HW_MODULE	"hwpoison_inject"

static pthread_t *thread_ids;
static int number_threads;
static int run_iterations;
static int maximum_pfns;

static volatile int sigbus_received;
static pthread_cond_t sigbus_received_cv;
static pthread_mutex_t sigbus_received_mtx = PTHREAD_MUTEX_INITIALIZER;

static long pagesize;
static char beginning_tag[BUFSIZ];
static int hwpoison_probe;

static void my_yield(void)
{
	static const struct timespec t0 = { 0, 0 };

	nanosleep(&t0, NULL);
}

/* a SIGBUS received is a confirmation of test failure */
static void sigbus_handler(int signum LTP_ATTRIBUTE_UNUSED)
{
	pthread_mutex_lock(&sigbus_received_mtx);
	sigbus_received++;
	pthread_cond_signal(&sigbus_received_cv);
	pthread_mutex_unlock(&sigbus_received_mtx);
	pause();
}

static void *sigbus_monitor(void *arg LTP_ATTRIBUTE_UNUSED)
{
	pthread_mutex_lock(&sigbus_received_mtx);
	while (!sigbus_received)
		pthread_cond_wait(&sigbus_received_cv, &sigbus_received_mtx);
	pthread_mutex_unlock(&sigbus_received_mtx);
	tst_res(TFAIL, "SIGBUS Received");
	exit(1);
}

/*
 * Allocate a page and write a sentinel value into it.
 */
static void *allocate_write(int sentinel)
{
	void *p;
	int *s;

	p = SAFE_MMAP(NULL, pagesize, PROT_READ|PROT_WRITE,
		      MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	s = (int *)p;
	*s = sentinel;
	return p;
}

/*
 * Verify and unmap the given page.
 */
static int verif_unmap(void *page, int sentinel)
{
	int *s = (int *)page;

	if (*s != sentinel) {
		tst_res(TFAIL, "pid[%d]: fail: bad sentinel value seen: %d expected: %d\n", getpid(), *s, sentinel);
		return 1;
	}

	return SAFE_MUNMAP(page, pagesize);
}

/*
 * allocate_offline() - Allocate and offline test called per-thread
 *
 * This function does the allocation and offline by mmapping an
 * anonymous page and offlining it.
 */
static int allocate_offline(int tnum)
{
	int loop;

	for (loop = 0; loop < NUM_LOOPS; loop++) {
		long *ptrs[NUM_PAGES];
		int num_alloc;
		int i;

		for (num_alloc = 0; num_alloc < NUM_PAGES; num_alloc++) {

			ptrs[num_alloc] = allocate_write((tnum << NUM_PAGES_OFFSET) | num_alloc);
			if (ptrs[num_alloc] == NULL)
				return -1;

			if (madvise(ptrs[num_alloc], pagesize, MADV_SOFT_OFFLINE) == -1) {
				if (errno != EINVAL)
					tst_res(TFAIL | TERRNO, "madvise failed");
				if (errno == EINVAL)
					tst_res(TCONF, "madvise() didn't support MADV_SOFT_OFFLINE");
				return errno;
			}
		}

		for (i = 0; i < num_alloc; i++) {
			if (verif_unmap(ptrs[i], (tnum << NUM_PAGES_OFFSET) | i) != 0)
				return 1;
		}

		my_yield();
		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Thread [%d]: Test runtime is over, exiting", tnum);
			break;
		}
	}

	return 0;
}

static void *alloc_mem(void *threadnum)
{
	int err;
	int tnum = (int)(uintptr_t)threadnum;

	/* waiting for other threads starting */
	TST_CHECKPOINT_WAIT(0);

	err = allocate_offline(tnum);
	tst_res(TINFO,
		"Thread [%d] returned %d, %s.", tnum, err, (err ? "failed" : "succeeded"));
	return (void *)(uintptr_t) (err ? -1 : 0);
}

static void stress_alloc_offl(void)
{
	int thread_index;
	int thread_failure = 0;
	pthread_t sigbus_monitor_t;

	run_iterations++;

	SAFE_PTHREAD_CREATE(&sigbus_monitor_t, NULL, sigbus_monitor, NULL);
	pthread_detach(sigbus_monitor_t);

	for (thread_index = 0; thread_index < number_threads; thread_index++) {
		SAFE_PTHREAD_CREATE(&thread_ids[thread_index], NULL, alloc_mem,
				    (void *)(uintptr_t)thread_index);
	}

	TST_CHECKPOINT_WAKE2(0, number_threads);

	for (thread_index = 0; thread_index < number_threads; thread_index++) {
		void *status;

		SAFE_PTHREAD_JOIN(thread_ids[thread_index], &status);
		if ((intptr_t)status != 0) {
			tst_res(TFAIL, "thread [%d] - exited with errors",
				thread_index);
			thread_failure++;
		}
	}

	if (thread_failure == 0)
		tst_res(TPASS, "soft-offline / mmap race still clean");
}

/*
 * ------------
 * Cleanup code:
 * The idea is to retrieve all the pfn numbers that have been soft-offined
 * (generating a "Soft offlining pfn 0x..." message in the kernel ring buffer)
 * by the current test (since a "beginning_tag" message we write when starting).
 * And to put these pages back online by writing the pfn number to the
 * <debugfs>/hwpoison/unpoison-pfn special file.
 * ------------
 */
#define OFFLINE_PATTERN "Soft offlining pfn 0x"
#define OFFLINE_PATTERN_LEN sizeof(OFFLINE_PATTERN)

/* return the pfn if the kmsg msg is a soft-offline indication*/
static unsigned long parse_kmsg_soft_offlined_pfn(char *line, ssize_t len)
{
	char *pos;
	unsigned long addr = 0UL;

	pos = strstr(line, OFFLINE_PATTERN);
	if (pos == NULL)
		return 0UL;

	pos += OFFLINE_PATTERN_LEN-1;
	if (pos > (line + len))
		return 0UL;

	addr = strtoul(pos, NULL, 16);
	if ((addr == ULONG_MAX) && (errno == ERANGE))
		return 0UL;

	return addr;
}

/* return the pfns seen in kernel message log */
static int populate_from_klog(char *begin_tag, unsigned long *pfns, int max)
{
	int found = 0, fd, beginning_tag_found = 0;
	ssize_t sz;
	unsigned long pfn;
	char buf[BUFSIZ];

	fd = SAFE_OPEN("/dev/kmsg", O_RDONLY|O_NONBLOCK);

	while (found < max) {
		sz = read(fd, buf, sizeof(buf));
		/* kmsg returns EPIPE if record was modified while reading */
		if (sz < 0 && errno == EPIPE)
			continue;
		if (sz <= 0)
			break;
		if (!beginning_tag_found) {
			if (strstr(buf, begin_tag))
				beginning_tag_found = 1;
			continue;
		}
		pfn = parse_kmsg_soft_offlined_pfn(buf, sz);
		if (pfn)
			pfns[found++] = pfn;
	}
	SAFE_CLOSE(fd);
	return found;
}

/*
 * Read the given file to search for the key.
 * Return 1 if the key is found.
 */
static int find_in_file(char *path, char *key)
{
	char line[4096];
	int found = 0;
	FILE *file = SAFE_FOPEN(path, "r");

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, key)) {
			found = 1;
			break;
		}
	}
	SAFE_FCLOSE(file);
	return found;
}

static void unpoison_this_pfn(unsigned long pfn, int fd)
{
	char pfn_str[19];

	snprintf(pfn_str, sizeof(pfn_str), "0x%lx", pfn);
	SAFE_WRITE(0, fd, pfn_str, strlen(pfn_str));
}

/* Find and open the <debugfs>/hwpoison/unpoison-pfn special file */
static int open_unpoison_pfn(void)
{
	char *added_file_path = "/hwpoison/unpoison-pfn";
	const char *const cmd_modprobe[] = {"modprobe", HW_MODULE, NULL};
	char debugfs_fp[4096];
	struct mntent *mnt;
	FILE *mntf;

	if (!find_in_file("/proc/modules", HW_MODULE) && tst_check_builtin_driver(HW_MODULE))
		hwpoison_probe = 1;

	/* probe hwpoison only if it isn't already there */
	if (hwpoison_probe)
		SAFE_CMD(cmd_modprobe, NULL, NULL);

	/* debugfs mount point */
	mntf = setmntent("/etc/mtab", "r");
	if (!mntf) {
		tst_brk(TBROK | TERRNO, "Can't open /etc/mtab");
		return -1;
	}
	while ((mnt = getmntent(mntf)) != NULL) {
		if (strcmp(mnt->mnt_type, "debugfs") == 0) {
			strcpy(debugfs_fp, mnt->mnt_dir);
			strcat(debugfs_fp, added_file_path);
			break;
		}
	}
	endmntent(mntf);
	if (!mnt)
		return -1;

	return SAFE_OPEN(debugfs_fp, O_WRONLY);
}

/*
 * Get all the Offlined PFNs indicated in the dmesg output
 * starting after the given beginning tag, and request a debugfs
 * hwpoison/unpoison-pfn for each of them.
 */
static void unpoison_pfn(char *begin_tag)
{
	unsigned long *pfns;
	const char *const cmd_rmmod[] = {"rmmod", HW_MODULE, NULL};
	int found_pfns, fd;

	pfns = SAFE_MALLOC(sizeof(pfns) * maximum_pfns * run_iterations);

	fd = open_unpoison_pfn();
	if (fd >= 0) {
		found_pfns = populate_from_klog(begin_tag, pfns, maximum_pfns * run_iterations);

		tst_res(TINFO, "Restore %d Soft-offlined pages", found_pfns);
		/* unpoison in reverse order */
		while (found_pfns-- > 0)
			unpoison_this_pfn(pfns[found_pfns], fd);

		SAFE_CLOSE(fd);
	}
	/* remove hwpoison only if we probed it */
	if (hwpoison_probe)
		SAFE_CMD(cmd_rmmod, NULL, NULL);
}

/*
 * Create and write a beginning tag to the kernel buffer to be used on cleanup
 * when trying to restore the soft-offlined pages of our test run.
 */
static void write_beginning_tag_to_kmsg(void)
{
	int fd;

	fd = SAFE_OPEN("/dev/kmsg", O_WRONLY);
	snprintf(beginning_tag, sizeof(beginning_tag),
		 "Soft-offlining pages test starting (pid: %ld)",
		 (long)getpid());
	SAFE_WRITE(1, fd, beginning_tag, strlen(beginning_tag));
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	struct sigaction my_sigaction;

	number_threads = (int)sysconf(_SC_NPROCESSORS_ONLN) * 2;
	if (number_threads <= 1)
		number_threads = 2;
	else if (number_threads > 5)
		number_threads = 5;

	maximum_pfns = number_threads * NUM_LOOPS * NUM_PAGES;
	thread_ids = SAFE_MALLOC(sizeof(pthread_t) * number_threads);
	pagesize = sysconf(_SC_PAGESIZE);

	/* SIGBUS is the main failure criteria */
	my_sigaction.sa_handler = sigbus_handler;
	if (sigaction(SIGBUS, &my_sigaction, NULL) == -1)
		tst_res(TFAIL | TERRNO, "Signal handler attach failed");

	write_beginning_tag_to_kmsg();
	tst_res(TINFO, "Spawning %d threads, with a total of %d memory pages",
		number_threads, maximum_pfns);
}

static void cleanup(void)
{
	unpoison_pfn(beginning_tag);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_drivers = (const char *const []) {
		HW_MODULE,
		NULL
	},
	.needs_cmds = (const char *[]) {
		"modprobe",
		"rmmod",
		NULL
	},
	.max_runtime = 30,
	.needs_checkpoints = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = stress_alloc_offl,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d4ae9916ea29"},
		{}
	}
};
