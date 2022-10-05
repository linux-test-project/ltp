// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Jan Stancek. All rights reserved.
 */
/*
 * Test: Spawn 2 threads. First thread maps, writes and unmaps
 * an area. Second thread tries to read from it. Second thread
 * races against first thread. There is no synchronization
 * between threads, but each mmap/munmap increases a counter
 * that is checked to determine when has read occurred. If a read
 * hit SIGSEGV in between mmap/munmap it is a failure. If a read
 * between mmap/munmap worked, then its value must match expected
 * value.
 *
 * Can trigger panics/stalls since at least 4.14 on some arches:
 *   fc8efd2ddfed ("mm/memory.c: do_fault: avoid usage of stale vm_area_struct")
 * Can trigger user-space stalls on aarch64:
 *   7a30df49f63a ("mm: mmu_gather: remove __tlb_reset_range() for force flush")
 *   https://lore.kernel.org/linux-mm/1817839533.20996552.1557065445233.JavaMail.zimbra@redhat.com
 * Can trigger "still mapped when deleted" BUG at mm/filemap.c:171, on aarch64 since 4.20
 *   e1b98fa31664 ("locking/rwsem: Add missing ACQUIRE to read_slowpath exit when queue is empty")
 *   99143f82a255 ("lcoking/rwsem: Add missing ACQUIRE to read_slowpath sleep loop")
 */
#include <errno.h>
#include <float.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "lapi/abisize.h"
#include "tst_test.h"
#include "tst_safe_pthread.h"

#define GIGABYTE (1L*1024*1024*1024)
#define TEST_FILENAME "ashfile"

/* seconds remaining before reaching timeout */
#define STOP_THRESHOLD 10

#define PROGRESS_SEC 3

static int file_size = 1024;
static int num_iter = 5000;

static void *distant_area;
static jmp_buf jmpbuf;
static volatile unsigned char *map_address;
static unsigned long page_sz;

static unsigned long mapped_sigsegv_count;
static unsigned long map_count;
static unsigned long threads_spawned;
static unsigned long data_matched;
static unsigned long repeated_reads;

/* sequence id for each map/unmap performed */
static int mapcnt, unmapcnt;
/* stored sequence id before making read attempt */
static int br_map, br_unmap;

/* compare "before read" counters  with "after read" counters */
static inline int was_area_mapped(int br_m, int br_u, int ar_m, int ar_u)
{
	return (br_m == ar_m && br_u == ar_u && br_m > br_u);
}

static void sig_handler(int signal, siginfo_t *info,
	LTP_ATTRIBUTE_UNUSED void *ut)
{
	int ar_m, ar_u;

	switch (signal) {
	case SIGSEGV:
		/* if we hit SIGSEGV between map/unmap, something is wrong */
		ar_u = tst_atomic_load(&unmapcnt);
		ar_m = tst_atomic_load(&mapcnt);
		if (was_area_mapped(br_map, br_unmap, ar_m, ar_u)) {
			tst_res(TFAIL, "got sigsegv while mapped");
			_exit(TFAIL);
		}

		mapped_sigsegv_count++;
		longjmp(jmpbuf, 1);
		break;
	default:
		tst_res(TFAIL, "Unexpected signal - %d, addr: %p, exiting",
			signal, info->si_addr);
		_exit(TBROK);
	}
}

void *map_write_unmap(void *ptr)
{
	int *fd = ptr;
	void *tmp;
	int i, j;

	for (i = 0; i < num_iter; i++) {
		map_address = SAFE_MMAP(distant_area,
			(size_t) file_size, PROT_WRITE | PROT_READ,
			MAP_SHARED, *fd, 0);
		tst_atomic_inc(&mapcnt);

		for (j = 0; j < file_size; j++)
			map_address[j] = 'b';

		tmp = (void *)map_address;
		tst_atomic_inc(&unmapcnt);
		SAFE_MUNMAP(tmp, file_size);

		map_count++;
	}

	return NULL;
}

void *read_mem(LTP_ATTRIBUTE_UNUSED void *ptr)
{
	volatile int i; /* longjmp could clobber i */
	int j, ar_map, ar_unmap;
	unsigned char c;

	for (i = 0; i < num_iter; i++) {
		if (setjmp(jmpbuf) == 1)
			continue;

		for (j = 0; j < file_size; j++) {
read_again:
			br_map = tst_atomic_load(&mapcnt);
			br_unmap = tst_atomic_load(&unmapcnt);

			c = map_address[j];

			ar_unmap = tst_atomic_load(&unmapcnt);
			ar_map = tst_atomic_load(&mapcnt);

			/*
			 * Read above is racing against munmap and mmap
			 * in other thread. While the address might be valid
			 * the mapping could be in various stages of being
			 * 'ready'. We only check the value, if we can be sure
			 * read hapenned in between single mmap and munmap as
			 * observed by first thread.
			 */
			if (was_area_mapped(br_map, br_unmap, ar_map,
				ar_unmap)) {
				switch (c) {
				case 'a':
					repeated_reads++;
					goto read_again;
				case 'b':
					data_matched++;
					break;
				default:
					tst_res(TFAIL, "value[%d] is %c", j, c);
					break;
				}
			}
		}
	}

	return NULL;
}

int mkfile(int size)
{
	int fd, i;

	fd = SAFE_OPEN(TEST_FILENAME, O_RDWR | O_CREAT, 0600);
	SAFE_UNLINK(TEST_FILENAME);

	for (i = 0; i < size; i++)
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "a", 1);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "\0", 1);

	if (fsync(fd) == -1)
		tst_brk(TBROK | TERRNO, "fsync()");

	return fd;
}

static void setup(void)
{
	struct sigaction sigptr;
	size_t distant_mmap_size;
	size_t mem_total;

	page_sz = getpagesize();
	mem_total = SAFE_READ_MEMINFO("MemTotal:");
	mem_total *= 1024;

#ifdef TST_ABI32
	distant_mmap_size = 256*1024*1024;
#else
	distant_mmap_size = (mem_total > 4 * GIGABYTE) ? 2 * GIGABYTE : mem_total / 2;
#endif
	/*
	 * Used as hint for mmap thread, so it doesn't interfere
	 * with other potential (temporary) mappings from libc
	 */
	distant_area = SAFE_MMAP(0, distant_mmap_size, PROT_WRITE | PROT_READ,
			MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	SAFE_MUNMAP(distant_area, distant_mmap_size);
	distant_area += distant_mmap_size / 2;

	sigptr.sa_sigaction = sig_handler;
	sigemptyset(&sigptr.sa_mask);
	sigptr.sa_flags = SA_SIGINFO | SA_NODEFER;
	SAFE_SIGACTION(SIGSEGV, &sigptr, NULL);
}

static void run(void)
{
	pthread_t thid[2];
	int start, last_update;

	start = last_update = tst_remaining_runtime();
	while (tst_remaining_runtime()) {
		int fd = mkfile(file_size);

		tst_atomic_store(0, &mapcnt);
		tst_atomic_store(0, &unmapcnt);

		SAFE_PTHREAD_CREATE(&thid[0], NULL, map_write_unmap, &fd);
		SAFE_PTHREAD_CREATE(&thid[1], NULL, read_mem, &fd);
		threads_spawned += 2;

		SAFE_PTHREAD_JOIN(thid[0], NULL);
		SAFE_PTHREAD_JOIN(thid[1], NULL);

		close(fd);

		if (last_update - tst_remaining_runtime() >= PROGRESS_SEC) {
			last_update = tst_remaining_runtime();
			tst_res(TINFO, "[%03d] mapped: %lu, sigsegv hit: %lu, "
				"threads spawned: %lu",
				start - last_update,
				map_count, mapped_sigsegv_count,
				threads_spawned);
			tst_res(TINFO, "      repeated_reads: %ld, "
				"data_matched: %lu", repeated_reads,
				data_matched);
		}
	}
	tst_res(TPASS, "System survived.");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.max_runtime = 180,
	.needs_tmpdir = 1,
};
