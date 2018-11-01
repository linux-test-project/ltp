/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Portions Copyright (c) 2000 Ulrich Drepper
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
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
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com$
 *
 * For further information regarding this notice, see:$
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 *
 *    Linux Test Project - Silicon Graphics, Inc.
 *    TEST IDENTIFIER	: fork05
 *    EXECUTED BY	: anyone
 *    TEST TITLE	: Make sure LDT is propagated correctly
 *    TEST CASE TOTAL	: 1
 *    CPU TYPES		: i386
 *    AUTHORS		: Ulrich Drepper
 *			  Nate Straz
 *
 *On Friday, May 2, 2003 at 09:47:00AM MST, Ulrich Drepper wrote:
 *>Robert Williamson wrote:
 *>
 *>>   I'm getting a SIGSEGV with one of our tests, fork05.c, that apparently
 *>> you wrote (attached below).  The test passes on my 2.5.68 machine running
 *>> SuSE 8.0 (glibc 2.2.5 and Linuxthreads), however it segmentation faults on
 *>> RedHat 9 running 2.5.68.  The test seems to "break" when it attempts to run
 *>> the assembly code....could you take a look at it?
 *>
 *>There is no need to look at it, I know it cannot work anymore on recent
 *>systems.  Either change all uses of %gs to %fs or skip the entire patch
 *>if %gs has a nonzero value.
 *>
 *>- --
 *>- --------------.                        ,-.            444 Castro Street
 *>Ulrich Drepper \    ,-----------------'   \ Mountain View, CA 94041 USA
 *>Red Hat         `--' drepper at redhat.com `---------------------------
 *
 *
 *
 *On Sat, Aug 12, 2000 at 12:47:31PM -0700, Ulrich Drepper wrote:
 *> Ever since the %gs handling was fixed in the 2.3.99 series the
 *> appended test program worked.  Now with 2.4.0-test6 it's not working
 *> again.  Looking briefly over the patch from test5 to test6 I haven't
 *> seen an immediate candidate for the breakage.  It could be missing
 *> propagation of the LDT to the new process (and therefore an invalid
 *> segment descriptor) or simply clearing %gs.
 *>
 *> Anyway, this is what you should see and what you get with test5:
 *>
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *> a = 42
 *> %gs = 0x0007
 *> %gs = 0x0007
 *> a = 99
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *>
 *> This is what you get with test6:
 *>
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *> a = 42
 *> %gs = 0x0007
 *> %gs = 0x0000
 *> <SEGFAULT>
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *>
 *> If somebody is actually creating a test suite for the kernel, please
 *> add this program.  It's mostly self-contained.  The correct handling
 *> of %gs is really important since glibc 2.2 will make heavy use of it.
 *>
 *> - --
 *> - ---------------.                          ,-.   1325 Chesapeake Terrace
 *> Ulrich Drepper  \    ,-------------------'   \  Sunnyvale, CA 94089 USA
 *> Red Hat          `--' drepper at redhat.com   `------------------------
 *>
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "lapi/syscalls.h"
#include "test.h"

char *TCID = "fork05";

static char *environ_list[] = { "TERM", "NoTSetzWq", "TESTPROG" };

#define NUMBER_OF_ENVIRON (sizeof(environ_list)/sizeof(char *))
int TST_TOTAL = NUMBER_OF_ENVIRON;

#if defined(linux) && defined(__i386__)

struct modify_ldt_ldt_s {
	unsigned int entry_number;
	unsigned long int base_addr;
	unsigned int limit;
	unsigned int seg_32bit:1;
	unsigned int contents:2;
	unsigned int read_exec_only:1;
	unsigned int limit_in_pages:1;
	unsigned int seg_not_present:1;
	unsigned int useable:1;
	unsigned int empty:25;
};

static int a = 42;

static void modify_ldt(int func, struct modify_ldt_ldt_s *ptr, int bytecount)
{
	ltp_syscall(__NR_modify_ldt, func, ptr, bytecount);
}

int main(void)
{
	struct modify_ldt_ldt_s ldt0;
	int lo;
	pid_t pid;
	int res;

	ldt0.entry_number = 0;
	ldt0.base_addr = (long)&a;
	ldt0.limit = 4;
	ldt0.seg_32bit = 1;
	ldt0.contents = 0;
	ldt0.read_exec_only = 0;
	ldt0.limit_in_pages = 0;
	ldt0.seg_not_present = 0;
	ldt0.useable = 1;
	ldt0.empty = 0;

	modify_ldt(1, &ldt0, sizeof(ldt0));

	asm volatile ("movw %w0, %%fs"::"q" (7));

	asm volatile ("movl %%fs:0, %0":"=r" (lo));
	tst_resm(TINFO, "a = %d", lo);

	asm volatile ("pushl %%fs; popl %0":"=q" (lo));
	tst_resm(TINFO, "%%fs = %#06hx", lo);

	asm volatile ("movl %0, %%fs:0"::"r" (99));

	pid = fork();

	if (pid == 0) {
		asm volatile ("pushl %%fs; popl %0":"=q" (lo));
		tst_resm(TINFO, "%%fs = %#06hx", lo);

		asm volatile ("movl %%fs:0, %0":"=r" (lo));
		tst_resm(TINFO, "a = %d", lo);

		if (lo != 99)
			tst_resm(TFAIL, "Test failed");
		else
			tst_resm(TPASS, "Test passed");
		exit(lo != 99);
	} else {
		waitpid(pid, &res, 0);
	}

	return WIFSIGNALED(res);
}

#else /* if defined(linux) && defined(__i386__) */

int main(void)
{
	tst_resm(TINFO, "%%fs test only for ix86");

	/*
	 * should be successful on all non-ix86 platforms.
	 */
	tst_exit();
}

#endif /* if defined(linux) && defined(__i386__) */
