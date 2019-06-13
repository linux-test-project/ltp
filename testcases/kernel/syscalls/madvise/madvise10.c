// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-2.0 or later
/*
 *  Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 *  Email : code@zilogic.com
 */

/*
 * test cases for madvise(2) system call, advise value as "MADV_WIPEONFORK".
 *
 * DESCRIPTION
 * Present the child process with zero-filled memory in this
 * range after a fork(2).
 * The MADV_WIPEONFORK operation can be applied only to
 * private anonymous pages.
 * Within the child created by fork(2), the MADV_WIPEONFORK
 * setting remains in place on the specified map_address range.
 * The MADV_KEEPONFORK operation undo the effect of MADV_WIPEONFORK.
 *
 * Test-Case 1 : madvise with "MADV_WIPEONFORK"
 * flow :	Map memory area as private anonymous page.
 *		Mark memory area as wipe-on-fork.
 *		On fork, child process memory should be zeroed.
 *
 * Test-Case 2 : madvise with "MADV_WIPEONFORK" and "ZERO" length
 * flow :	Map memory area as private anonymous page.
 *		Mark memory area as wipe-on-fork, with length zero.
 *		On fork, child process memory should be accessible.
 *
 * Test-Case 3 : "MADV_WIPEONFORK" on Grand child
 * flow :	Map memory area as private anonymous.
 *		Mark memory areas as wipe-on-fork.
 *		On fork, child process memory should be zeroed.
 *		In child, fork to create grand-child,
 *		memory should be zeroed.
 *
 * Test-Case 4 : Undo "MADV_WIPEONFORK" by "MADV_KEEPONFORK"
 * flow :	Map memory area as private anonymous page.
 *		Mark memory area as wipe-on-fork.
 *		Mark memory area as keep-on-fork.
 *		On fork, child process memory should be retained.
 **/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "lapi/mmap.h"
#include "tst_test.h"
#include "tst_safe_macros.h"

#define MAP_SIZE (16 * 1024)

static char pattern[MAP_SIZE];
static char zero[MAP_SIZE];

static const struct test_case {
	int size;
	int advise1;
	int advise2;
	char *exp;
	int grand_child;
	const char *desc;
} tcases[] = {
	{MAP_SIZE, MADV_NORMAL,	MADV_WIPEONFORK, zero,    0,
	"MADV_WIPEONFORK zeroes memory in child"},
	{0,	   MADV_NORMAL, MADV_WIPEONFORK, pattern, 0,
	"MADV_WIPEONFORK with zero length does nothing"},
	{MAP_SIZE, MADV_NORMAL, MADV_WIPEONFORK, zero,    1,
	"MADV_WIPEONFORK zeroes memory in grand-child"},
	{MAP_SIZE, MADV_WIPEONFORK, MADV_KEEPONFORK, pattern, 0,
	"MADV_KEEPONFORK will undo MADV_WIPEONFORK"},
};

static void cmp_area(char *addr, const struct test_case *tc)
{
	int i;

	for (i = 0; i < tc->size; i++) {
		if (addr[i] != tc->exp[i]) {
			tst_res(TFAIL, "In PID %d, addr[%d] = 0x%02x, "
				"expected[%d] = 0x%02x", getpid(),
				i, addr[i], i, tc->exp[i]);
			break;
		}
	}

	tst_res(TPASS, "In PID %d, Matched expected pattern", getpid());
}

static int set_advice(char *addr, int size, int advise)
{
	TEST(madvise(addr, size, advise));

	if (TST_RET == -1) {
		if (TST_ERR == EINVAL) {
			tst_res(TCONF, "madvise(%p, %d, 0x%x) is not supported",
			addr, size, advise);
		} else {
			tst_res(TFAIL | TTERRNO, "madvise(%p, %d, 0x%x)",
			addr, size, advise);
		}

		return 1;
	}

	tst_res(TPASS, "madvise(%p, %d, 0x%x)",	addr, size, advise);
	return 0;
}

static char *mem_map(void)
{
	char *ptr;

	ptr = SAFE_MMAP(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);

	memcpy(ptr, pattern, MAP_SIZE);

	return ptr;
}

static void test_madvise(unsigned int test_nr)
{
	const struct test_case *tc = &tcases[test_nr];
	char *addr;
	pid_t pid;

	addr = mem_map();

	tst_res(TINFO, "%s", tc->desc);
	if (set_advice(addr, tc->size, tc->advise1))
		goto un_map;

	if (!set_advice(addr, tc->size, tc->advise2)) {
		pid = SAFE_FORK();

		if (!pid) {
			if (tc->grand_child) {
				pid = SAFE_FORK();

				if (!pid) {
					cmp_area(addr, tc);
					exit(0);
				}
			} else {
				cmp_area(addr, tc);
				exit(0);
			}
		}
		tst_reap_children();
	}

un_map:
	SAFE_MUNMAP(addr, MAP_SIZE);
}

static void setup(void)
{
	unsigned int i;

	for (i = 0; i < MAP_SIZE; i++)
		pattern[i] = i % 0x03;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.test = test_madvise,
	.setup = setup,
};
