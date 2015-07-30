/*
 * Copyright (C) 2010  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program
 * with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
/*
 * munmap() don't check sysctl_max_mapcount
 *
 * From http://lkml.org/lkml/2009/10/2/85:
 *
 * On ia64, the following test program exit abnormally, because glibc
 * thread library called abort().
 *
 *  ========================================================
 *  (gdb) bt
 *  #0  0xa000000000010620 in __kernel_syscall_via_break ()
 *  #1  0x20000000003208e0 in raise () from /lib/libc.so.6.1
 *  #2  0x2000000000324090 in abort () from /lib/libc.so.6.1
 *  #3  0x200000000027c3e0 in __deallocate_stack () from
 *      /lib/libpthread.so.0
 *  #4  0x200000000027f7c0 in start_thread () from /lib/libpthread.so.0
 *  #5  0x200000000047ef60 in __clone2 () from /lib/libc.so.6.1
 *  ========================================================
 * The fact is, glibc call munmap() when thread exitng time for freeing
 * stack, and it assume munlock() never fail. However, munmap() often
 * make vma splitting and it with many mapcount make -ENOMEM.
 *
 * Oh well, stack unfreeing is not reasonable option. Also munlock() via
 * free() shouldn't failed.
 *
 * Thus, munmap() shoudn't check max-mapcount.
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<errno.h>
#include<unistd.h>
#include "test.h"

char *TCID = "mmap11";
int TST_TOTAL = 1;

#define MAL_SIZE (100*1024)

static void *wait_thread(void *args);
static void *wait_thread2(void *args);
static void setup(void);
static void cleanup(void);
static void check(void);

int main(int argc, char *argv[])
{

	tst_parse_opts(argc, argv, NULL, NULL);
	setup();
	check();
	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

void cleanup(void)
{
}

void check(void)
{
	int lc;
	pthread_t *thread, th;
	int ret, count = 0;
	pthread_attr_t attr;

	ret = pthread_attr_init(&attr);
	if (ret)
		tst_brkm(TBROK | TERRNO, cleanup, "pthread_attr_init");
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "pthread_attr_setdetachstate");
	thread = malloc(STD_LOOP_COUNT * sizeof(pthread_t));
	if (thread == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		ret = pthread_create(&th, &attr, wait_thread, NULL);
		if (ret) {
			tst_resm(TINFO, "[%d] ", count);
			tst_brkm(TBROK | TERRNO, cleanup, "pthread_create");
		}
		count++;
		ret = pthread_create(&thread[lc], &attr, wait_thread2, NULL);
		if (ret) {
			tst_resm(TINFO, "[%d] ", count);
			tst_brkm(TBROK | TERRNO, cleanup, "pthread_create");
		}
		count++;
	}
	tst_resm(TPASS, "test completed.");
	free(thread);
}

void *wait_thread(void *args)
{
	void *addr;

	addr = malloc(MAL_SIZE);
	if (addr)
		memset(addr, 1, MAL_SIZE);
	sleep(1);
	return NULL;
}

void *wait_thread2(void *args)
{
	return NULL;
}
