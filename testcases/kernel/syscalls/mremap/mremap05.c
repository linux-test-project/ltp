/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * Test Name: mremap05
 *
 * Test Description:
 *  Verify that MREMAP_FIXED fails without MREMAP_MAYMOVE.
 *  Verify that MREMAP_FIXED|MREMAP_MAYMOVE fails if target address
 *    is not page aligned.
 *  Verify that MREMAP_FIXED|MREMAP_MAYMOVE fails if old range
 *    overlaps with new range.
 *  Verify that MREMAP_FIXED|MREMAP_MAYMOVE can move mapping to new address.
 *  Verify that MREMAP_FIXED|MREMAP_MAYMOVE unmaps previous mapping
 *    at the address range specified by new_address and new_size.
 */

#define _GNU_SOURCE
#include "config.h"
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "tso_safe_macros.h"

char *TCID = "mremap05";

struct test_case_t {
	char *old_address;
	char *new_address;
	size_t old_size;	/* in pages */
	size_t new_size;	/* in pages */
	int flags;
	const char *msg;
	void *exp_ret;
	int exp_errno;
	char *ret;
	void (*setup) (struct test_case_t *);
	void (*cleanup) (struct test_case_t *);
};

static void setup(void);
static void cleanup(void);
static void setup0(struct test_case_t *);
static void setup1(struct test_case_t *);
static void setup2(struct test_case_t *);
static void setup3(struct test_case_t *);
static void setup4(struct test_case_t *);
static void cleanup0(struct test_case_t *);
static void cleanup1(struct test_case_t *);

struct test_case_t tdat[] = {
	{
	 .old_size = 1,
	 .new_size = 1,
	 .flags = MREMAP_FIXED,
	 .msg = "MREMAP_FIXED requires MREMAP_MAYMOVE",
	 .exp_ret = MAP_FAILED,
	 .exp_errno = EINVAL,
	 .setup = setup0,
	 .cleanup = cleanup0},
	{
	 .old_size = 1,
	 .new_size = 1,
	 .flags = MREMAP_FIXED | MREMAP_MAYMOVE,
	 .msg = "new_addr has to be page aligned",
	 .exp_ret = MAP_FAILED,
	 .exp_errno = EINVAL,
	 .setup = setup1,
	 .cleanup = cleanup0},
	{
	 .old_size = 2,
	 .new_size = 1,
	 .flags = MREMAP_FIXED | MREMAP_MAYMOVE,
	 .msg = "old/new area must not overlap",
	 .exp_ret = MAP_FAILED,
	 .exp_errno = EINVAL,
	 .setup = setup2,
	 .cleanup = cleanup0},
	{
	 .old_size = 1,
	 .new_size = 1,
	 .flags = MREMAP_FIXED | MREMAP_MAYMOVE,
	 .msg = "mremap #1",
	 .setup = setup3,
	 .cleanup = cleanup0},
	{
	 .old_size = 1,
	 .new_size = 1,
	 .flags = MREMAP_FIXED | MREMAP_MAYMOVE,
	 .msg = "mremap #2",
	 .setup = setup4,
	 .cleanup = cleanup1},
};

static int pagesize;
static int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);

static void free_test_area(void *p, int size)
{
	SAFE_MUNMAP(cleanup, p, size);
}

static void *get_test_area(int size, int free_area)
{
	void *p;
	p = mmap(NULL, size, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (p == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "get_test_area mmap");
	if (free_area)
		free_test_area(p, size);
	return p;
}

static void test_mremap(struct test_case_t *t)
{
	t->ret = mremap(t->old_address, t->old_size, t->new_size, t->flags,
			t->new_address);

	if (t->ret == t->exp_ret) {
		if (t->ret != MAP_FAILED) {
			tst_resm(TPASS, "%s", t->msg);
			if (*(t->ret) == 0x1)
				tst_resm(TPASS, "%s value OK", t->msg);
			else
				tst_resm(TPASS, "%s value failed", t->msg);
		} else {
			if (errno == t->exp_errno)
				tst_resm(TPASS, "%s", t->msg);
			else
				tst_resm(TFAIL | TERRNO, "%s", t->msg);
		}
	} else {
		tst_resm(TFAIL, "%s ret: %p, expected: %p", t->msg,
			 t->ret, t->exp_ret);
	}
}

static void setup0(struct test_case_t *t)
{
	t->old_address = get_test_area(t->old_size * pagesize, 0);
	t->new_address = get_test_area(t->new_size * pagesize, 1);
}

static void setup1(struct test_case_t *t)
{
	t->old_address = get_test_area(t->old_size * pagesize, 0);
	t->new_address = get_test_area((t->new_size + 1) * pagesize, 1) + 1;
}

static void setup2(struct test_case_t *t)
{
	t->old_address = get_test_area(t->old_size * pagesize, 0);
	t->new_address = t->old_address;
}

static void setup3(struct test_case_t *t)
{
	t->old_address = get_test_area(t->old_size * pagesize, 0);
	t->new_address = get_test_area(t->new_size * pagesize, 1);
	t->exp_ret = t->new_address;
	*(t->old_address) = 0x1;
}

static void setup4(struct test_case_t *t)
{
	t->old_address = get_test_area(t->old_size * pagesize, 0);
	t->new_address = get_test_area(t->new_size * pagesize, 0);
	t->exp_ret = t->new_address;
	*(t->old_address) = 0x1;
	*(t->new_address) = 0x2;
}

static void cleanup0(struct test_case_t *t)
{
	if (t->ret == MAP_FAILED)
		free_test_area(t->old_address, t->old_size * pagesize);
	else
		free_test_area(t->ret, t->new_size * pagesize);
}

static void cleanup1(struct test_case_t *t)
{
	if (t->ret == MAP_FAILED) {
		free_test_area(t->old_address, t->old_size * pagesize);
		free_test_area(t->new_address, t->new_size * pagesize);
	} else {
		free_test_area(t->ret, t->new_size * pagesize);
	}
}

int main(int ac, char **av)
{
	int lc, testno;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; testno++) {
			tdat[testno].setup(&tdat[testno]);
			test_mremap(&tdat[testno]);
			tdat[testno].cleanup(&tdat[testno]);
		}
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	pagesize = getpagesize();
}

static void cleanup(void)
{
}
