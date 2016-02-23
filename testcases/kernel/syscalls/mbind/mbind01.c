/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008			      */
/* Author(s):	Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,		      */
/*		Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,		      */
/*		Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>		      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by	      */
/* the Free Software Foundation; either version 2 of the License, or	      */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,	      */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		      */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License	      */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*									      */
/******************************************************************************/
/******************************************************************************/
/*									      */
/* File:	mbind01.c						      */
/*									      */
/* Description: This tests the mbind() syscall				      */
/*									      */
/* Usage:	<for command-line>					      */
/* mbind01 [-c n] [-e][-i n] [-I x] [-p x] [-t]				      */
/*	where,	-c n : Run n copies concurrently.			      */
/*		-e   : Turn on errno logging.				      */
/*		-i n : Execute test n times.				      */
/*		-I x : Execute test for x seconds.			      */
/*		-P x : Pause for x seconds between iterations.		      */
/*		-t   : Turn on syscall timing.				      */
/*									      */
/* Total Tests: 1							      */
/*									      */
/* Test Name:	mbind01							      */
/* History:	Porting from Crackerjack to LTP is done by		      */
/*		Manas Kumar Nayak maknayak@in.ibm.com>			      */
/******************************************************************************/

#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "linux_syscall_numbers.h"
#include "include_j_h.h"
#include "numa_helper.h"

char *TCID = "mbind01";
int TST_TOTAL = 2;

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H && \
	HAVE_MPOL_CONSTANTS

#define MEM_LENGTH	      (4 * 1024 * 1024)

static int testno;

enum test_type {
	NORMAL,
	INVALID_POINTER,
};

enum from_node {
	NONE,
	SELF,
};

struct test_case {
	int ttype;
	int policy;
	int from_node;
	unsigned flags;
	int ret;
	int err;
};

/* Test cases
 *
 *   test status of errors on man page
 *
 *   EFAULT	v (detect unmapped hole or invalid pointer)
 *   EINVAL	v (invalid arguments)
 *   ENOMEM	can't check because it's difficult to create no-memory
 *   EIO	can't check because we don't have N-node NUMA system
 *		(only we can do is simulate 1-node NUMA)
 */
static struct test_case tcase[] = {
	{			/* case00 */
	 .policy = MPOL_DEFAULT,
	 .from_node = NONE,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case01 */
	 .policy = MPOL_DEFAULT,
	 .from_node = SELF,	/* target exists */
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			/* case02 */
	 .policy = MPOL_BIND,
	 .from_node = NONE,	/* no target */
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			/* case03 */
	 .policy = MPOL_BIND,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case04 */
	 .policy = MPOL_INTERLEAVE,
	 .from_node = NONE,	/* no target */
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			/* case05 */
	 .policy = MPOL_INTERLEAVE,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case06 */
	 .policy = MPOL_PREFERRED,
	 .from_node = NONE,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case07 */
	 .policy = MPOL_PREFERRED,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case08 */
	 .policy = -1,		/* unknown policy */
	 .from_node = NONE,
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			/* case09 */
	 .policy = MPOL_DEFAULT,
	 .from_node = NONE,
	 .flags = -1,		/* invalid flags */
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			/* case10 */
	 .ttype = INVALID_POINTER,
	 .policy = MPOL_PREFERRED,
	 .from_node = SELF,
	 .ret = -1,
	 .err = EFAULT,
	 },
};

static int do_test(struct test_case *tc);
static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc, i, ret;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
	testno = (int)(sizeof(tcase) / sizeof(tcase[0]));

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (i = 0; i < testno; i++) {
			tst_resm(TINFO, "(case%02d) START", i);
			ret = do_test(&tcase[i]);
			tst_resm((ret == 0 ? TPASS : TFAIL | TERRNO),
				 "(case%02d) END", i);
		}
	}
	cleanup();
	tst_exit();
}

static int do_test(struct test_case *tc)
{
	int ret, err, result, cmp_ok = 1;
	int policy;
	char *p = NULL;
#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
	nodemask_t *nodemask, *getnodemask;
#else
	struct bitmask *nodemask = numa_allocate_nodemask();
	struct bitmask *getnodemask = numa_allocate_nodemask();
#endif
	unsigned long maxnode = NUMA_NUM_NODES;
	unsigned long len = MEM_LENGTH;
	unsigned long *invalid_nodemask;
	int test_node = -1;

	ret = get_allowed_nodes(NH_MEMS, 1, &test_node);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "get_allowed_nodes: %d", ret);

#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
	nodemask = malloc(sizeof(nodemask_t));
	nodemask_zero(nodemask);
	nodemask_set(nodemask, test_node);
	getnodemask = malloc(sizeof(nodemask_t));
	nodemask_zero(getnodemask);
#else
	numa_bitmask_setbit(nodemask, test_node);
#endif
	p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
		 0, 0);
	if (p == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap");

	if (tc->ttype == INVALID_POINTER)
		invalid_nodemask = (unsigned long *)0xc0000000;

	errno = 0;
	if (tc->from_node == NONE)
		TEST(ret = ltp_syscall(__NR_mbind, p, len, tc->policy,
				   NULL, 0, tc->flags));
	else if (tc->ttype == INVALID_POINTER)
		TEST(ret = ltp_syscall(__NR_mbind, p, len, tc->policy,
				   invalid_nodemask, maxnode, tc->flags));
	else
#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
		TEST(ret = ltp_syscall(__NR_mbind, p, len, tc->policy,
				   nodemask, maxnode, tc->flags));
#else
		TEST(ret = ltp_syscall(__NR_mbind, p, len, tc->policy,
				   nodemask->maskp, nodemask->size, tc->flags));
#endif

	err = TEST_ERRNO;
	if (ret < 0)
		goto TEST_END;

	/* Check policy of the allocated memory */
#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
	TEST(ltp_syscall(__NR_get_mempolicy, &policy, getnodemask,
		     maxnode, p, MPOL_F_ADDR));
#else
	TEST(ltp_syscall(__NR_get_mempolicy, &policy, getnodemask->maskp,
		     getnodemask->size, p, MPOL_F_ADDR));
#endif
	if (TEST_RETURN < 0) {
		tst_resm(TFAIL | TERRNO, "get_mempolicy failed");
		return -1;
	}

	/* If policy == MPOL_DEFAULT, get_mempolicy doesn't return nodemask */
	if (tc->policy == MPOL_DEFAULT)
#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
		nodemask_zero(nodemask);
#else
		numa_bitmask_clearall(nodemask);
#endif

	if ((tc->policy == MPOL_PREFERRED) && (tc->from_node == NONE))
		cmp_ok = (tc->policy == policy);
	else
		cmp_ok = ((tc->policy == policy) &&
#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
			  nodemask_equal(nodemask, getnodemask));
#else
			  numa_bitmask_equal(nodemask, getnodemask));
#endif
TEST_END:
	result = ((err != tc->err) || (!cmp_ok));
	PRINT_RESULT_CMP(0, tc->ret, tc->err, ret, err, cmp_ok);
	return result;
}

static void setup(void)
{
	/* check syscall availability */
	ltp_syscall(__NR_mbind, NULL, 0, 0, NULL, 0, 0);

	if (!is_numa(NULL, NH_MEMS, 1))
		tst_brkm(TCONF, NULL, "requires NUMA with at least 1 node");

	TEST_PAUSE;
	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}
#else /* no NUMA */
int main(void)
{
	tst_brkm(TCONF, NULL, "System doesn't have required numa support");
}
#endif
