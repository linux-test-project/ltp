/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd		      */
/*	Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,	      */
/*		   Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,	      */
/*		   Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>		      */
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
/* File:	get_mempolicy01.c					      */
/*									      */
/* Description: This tests the get_mempolicy() syscall			      */
/*									      */
/* Usage:  <for command-line>						      */
/* get_mempolicy01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			      */
/*	where,	-c n : Run n copies concurrently.			      */
/*		-e   : Turn on errno logging.				      */
/*		-i n : Execute test n times.				      */
/*		-I x : Execute test for x seconds.			      */
/*		-P x : Pause for x seconds between iterations.		      */
/*		-t   : Turn on syscall timing.				      */
/*									      */
/* Total Tests: 1							      */
/*									      */
/* Test Name:	get_mempolicy01						      */
/* History:	Porting from Crackerjack to LTP is done by		      */
/*		Manas Kumar Nayak maknayak@in.ibm.com>			      */
/******************************************************************************/

#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif

#include "test.h"
#include "lapi/syscalls.h"
#include "include_j_h.h"
#include "common_j_h.c"
#include "numa_helper.h"

char *TCID = "get_mempolicy01";
int TST_TOTAL = 1;

#ifdef HAVE_NUMA_V2

#define MEM_LENGTH	(4 * 1024 * 1024)

static int testno;

enum test_type {
	DEFAULT,		/* get default policy */
	ADDR,			/* get policy of memory which include mapped address */
	INVALID_POINTER,
	INVALID_FLAGS,
};

enum from_node {
	NONE,
	SELF,
};

struct test_case {
	int ttype;
	int policy;
	int from_node;
	int ret;
	int err;
};

/* Test cases
 *
 *   test status of errors on man page
 *
 *   (NONE)	     man page hadn't been completed.
 *
 *   test status of errors NOT on man page
 *
 *   EFAULT	     v (invalid address)
 *   EINVAL	     v (invalid parameters)
 */
static struct test_case tcase[] = {
	{			/* case00 */
	 .ttype = DEFAULT,
	 .policy = MPOL_DEFAULT,
	 .from_node = NONE,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case01 */
	 .ttype = DEFAULT,
	 .policy = MPOL_BIND,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case02 */
	 .ttype = DEFAULT,
	 .policy = MPOL_INTERLEAVE,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case03 */
	 .ttype = DEFAULT,
	 .policy = MPOL_PREFERRED,
	 .from_node = NONE,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case04 */
	 .ttype = DEFAULT,
	 .policy = MPOL_PREFERRED,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case05 */
	 .ttype = ADDR,
	 .policy = MPOL_DEFAULT,
	 .from_node = NONE,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case06 */
	 .ttype = ADDR,
	 .policy = MPOL_BIND,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case07 */
	 .ttype = ADDR,
	 .policy = MPOL_INTERLEAVE,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case08 */
	 .ttype = ADDR,
	 .policy = MPOL_PREFERRED,
	 .from_node = NONE,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case09 */
	 .ttype = ADDR,
	 .policy = MPOL_PREFERRED,
	 .from_node = SELF,
	 .ret = 0,
	 .err = 0,
	 },
	{			/* case10 */
	 .ttype = INVALID_POINTER,
	 .policy = MPOL_DEFAULT,
	 .from_node = NONE,
	 .ret = -1,
	 .err = EFAULT,
	 },
	{			/* case11 */
	 .ttype = INVALID_FLAGS,
	 .policy = MPOL_DEFAULT,
	 .from_node = NONE,
	 .ret = -1,
	 .err = EINVAL,
	 },
};

static int do_test(struct test_case *tc);
static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int i, ret, lc;

	setup();

	ret = 0;
	testno = (int)ARRAY_SIZE(tcase);
	for (lc = 0; TEST_LOOPING(lc); lc++) {
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
	int ret, err, result, cmp_ok;
	int policy, flags;
	struct bitmask *nodemask = numa_allocate_nodemask();
	struct bitmask *getnodemask = numa_allocate_nodemask();
	char *p = NULL;
	unsigned long len = MEM_LENGTH;
	int test_node = -1;

	ret = get_allowed_nodes(NH_MEMS, 1, &test_node);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "get_allowed_nodes: %d", ret);
	numa_bitmask_setbit(nodemask, test_node);
	switch (tc->ttype) {
	case DEFAULT:
		flags = 0;
		p = NULL;
		if (tc->from_node == NONE)
			TEST(ltp_syscall(__NR_set_mempolicy, tc->policy,
				NULL, 0));
		else
			TEST(ltp_syscall(__NR_set_mempolicy, tc->policy,
				nodemask->maskp, nodemask->size));
		if (TEST_RETURN < 0) {
			tst_resm(TBROK | TERRNO, "set_mempolicy");
			return -1;
		}

		break;
	default:
		flags = MPOL_F_ADDR;
		p = mmap(NULL, len, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (p == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, cleanup, "mmap");
		if (tc->from_node == NONE)
			TEST(ltp_syscall(__NR_mbind, p, len, tc->policy,
				NULL, 0, 0));
		else
			TEST(ltp_syscall(__NR_mbind, p, len, tc->policy,
				nodemask->maskp, nodemask->size, 0));
		if (TEST_RETURN < 0) {
			tst_resm(TBROK | TERRNO, "mbind");
			return -1;
		}

		if (tc->ttype == INVALID_POINTER)
#ifdef __ia64__
			p = (char *)0x8000000000000000UL;
#else
			p = 0;
#endif

		if (tc->ttype == INVALID_FLAGS)
			flags = -1;
	}
	errno = 0;
	cmp_ok = 1;
	TEST(ret = ltp_syscall(__NR_get_mempolicy, &policy, getnodemask->maskp,
			   getnodemask->size, p, flags));
	err = TEST_ERRNO;
	if (ret < 0)
		goto TEST_END;

	/* if policy == MPOL_DEFAULT, get_mempolicy doesn't return nodemask */
	if (tc->policy == MPOL_DEFAULT)
		numa_bitmask_clearall(nodemask);
	cmp_ok = (tc->policy == policy && (tc->from_node == NONE ||
					   numa_bitmask_equal(nodemask,
							      getnodemask)));
TEST_END:
	result = (err != tc->err) || !cmp_ok;
	PRINT_RESULT_CMP(0, tc->ret, tc->err, ret, err, cmp_ok);
	return result;
}

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	/* check syscall availability */
	ltp_syscall(__NR_get_mempolicy, NULL, NULL, 0, NULL, 0);

	if (!is_numa(NULL, NH_MEMS, 1))
		tst_brkm(TCONF, NULL, "requires NUMA with at least 1 node");

	TEST_PAUSE;
	tst_tmpdir();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, NUMA_ERROR_MSG);
}
#endif
