/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Description:
 *   Verify that,
 *   1) mprotect() succeeds to set a region of memory with no access,
 *      when 'prot' is set to PROT_NONE. An attempt to access the contents
 *      of the region gives rise to the signal SIGSEGV.
 *   2) mprotect() succeeds to set a region of memory to be executed, when
 *      'prot' is set to PROT_EXEC.
 */

#include "config.h"
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "test.h"
#include "safe_macros.h"

static void sighandler(int sig);

static void setup(void);
static void cleanup(void);

static void testfunc_protnone(void);

static void testfunc_protexec(void);

static void (*testfunc[])(void) = { testfunc_protnone, testfunc_protexec };

char *TCID = "mprotect04";
int TST_TOTAL = ARRAY_SIZE(testfunc);

static volatile int sig_caught;
static sigjmp_buf env;
static unsigned int page_sz;
typedef void (*func_ptr_t)(void);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void sighandler(int sig)
{
	sig_caught = sig;
	siglongjmp(env, 1);
}

static void setup(void)
{
	tst_tmpdir();
	tst_sig(NOFORK, sighandler, cleanup);
	page_sz = getpagesize();

	TEST_PAUSE;
}

static void testfunc_protnone(void)
{
	char *addr;

	sig_caught = 0;

	addr = SAFE_MMAP(cleanup, 0, page_sz, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	/* Change the protection to PROT_NONE. */
	TEST(mprotect(addr, page_sz, PROT_NONE));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "mprotect failed");
	} else {
		if (sigsetjmp(env, 1) == 0)
			addr[0] = 1;

		switch (sig_caught) {
		case SIGSEGV:
			tst_resm(TPASS, "test PROT_NONE for mprotect success");
		break;
		case 0:
			tst_resm(TFAIL, "test PROT_NONE for mprotect failed");
		break;
		default:
			tst_brkm(TBROK, cleanup,
			         "received an unexpected signal: %d",
			         sig_caught);
		}
	}

	SAFE_MUNMAP(cleanup, addr, page_sz);
}

static void exec_func(void)
{
	return;
}

static int page_present(void *p)
{
	int fd;

	fd = SAFE_OPEN(cleanup, "page_present", O_WRONLY|O_CREAT, 0644);
	TEST(write(fd, p, 1));
	SAFE_CLOSE(cleanup, fd);

	if (TEST_RETURN >= 0)
		return 1;

	if (TEST_ERRNO != EFAULT)
		tst_brkm(TBROK | TTERRNO, cleanup, "page_present write");

	return 0;
}

static void clear_cache(void *start, int len)
{
#if HAVE_BUILTIN_CLEAR_CACHE == 1
	__builtin___clear_cache(start, start + len);
#else
	tst_brkm(TCONF, cleanup,
		"compiler doesn't have __builtin___clear_cache()");
#endif
}

/*
 * To check for the ABI version, because ppc64le can technically use
 * function descriptors.
 */
#if defined(__powerpc64__) && (!defined(_CALL_ELF) || _CALL_ELF < 2)
#define USE_FUNCTION_DESCRIPTORS
#endif

#ifdef USE_FUNCTION_DESCRIPTORS
typedef struct {
	uintptr_t entry;
	uintptr_t toc;
	uintptr_t env;
} func_descr_t;
#endif

/*
 * Copy page where &exec_func resides. Also try to copy subsequent page
 * in case exec_func is close to page boundary.
 */
static void *get_func(void *mem, uintptr_t *func_page_offset)
{
	uintptr_t page_sz = getpagesize();
	uintptr_t page_mask = ~(page_sz - 1);
	void *func_copy_start, *page_to_copy;
	void *mem_start = mem;

#ifdef USE_FUNCTION_DESCRIPTORS
	func_descr_t *opd =  (func_descr_t *)&exec_func;
	*func_page_offset = (uintptr_t)opd->entry & (page_sz - 1);
	func_copy_start = mem + *func_page_offset;
	page_to_copy = (void *)((uintptr_t)opd->entry & page_mask);
#else
	*func_page_offset = (uintptr_t)&exec_func & (page_sz - 1);
	func_copy_start = mem + *func_page_offset;
	page_to_copy = (void *)((uintptr_t)&exec_func & page_mask);
#endif
	tst_resm(TINFO, "exec_func: %p, page_to_copy: %p",
		&exec_func, page_to_copy);

	/* Copy 1st page. If it's not accessible, we might be running on a
	 * platform that supports execute-only page access permissions, in which
	 * case we have to explicitly change access protections to allow the
	 * memory to be read. */
	if (!page_present(page_to_copy)) {
		TEST(mprotect(page_to_copy, page_sz, PROT_READ | PROT_EXEC));
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO,
				 "mprotect(PROT_READ|PROT_EXEC) failed");
			return NULL;
		}
		/* If the memory is still not accessible, then something must be
		 * wrong. */
		if (!page_present(page_to_copy))
			tst_brkm(TBROK, cleanup, "page_to_copy not present\n");
	}
	memcpy(mem, page_to_copy, page_sz);

	clear_cache(mem_start, page_sz);

	/* return pointer to area where copy of exec_func resides */
	return func_copy_start;
}

static void testfunc_protexec(void)
{
	func_ptr_t func;
	uintptr_t func_page_offset;
	void *p;

	sig_caught = 0;

	p = SAFE_MMAP(cleanup, 0, page_sz, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

#ifdef USE_FUNCTION_DESCRIPTORS
	func_descr_t opd;
	opd.entry = (uintptr_t)get_func(p, &func_page_offset);
	func = (func_ptr_t)&opd;
#else
	func = get_func(p, &func_page_offset);
#endif

	if (!func)
		goto out;

	if (func_page_offset + 64 > page_sz) {
		SAFE_MUNMAP(cleanup, p, page_sz);
		tst_brkm(TCONF, cleanup, "func too close to page boundary, "
			"maybe your compiler ignores -falign-functions?");
	}

	/* Change the protection to PROT_EXEC. */
	TEST(mprotect(p, page_sz, PROT_EXEC));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "mprotect failed");
	} else {
		if (sigsetjmp(env, 1) == 0)
			(*func)();

		switch (sig_caught) {
		case SIGSEGV:
			tst_resm(TFAIL, "test PROT_EXEC for mprotect failed");
		break;
		case 0:
			tst_resm(TPASS, "test PROT_EXEC for mprotect success");
		break;
		default:
			tst_brkm(TBROK, cleanup,
			         "received an unexpected signal: %d",
			         sig_caught);
		}
	}

out:
	SAFE_MUNMAP(cleanup, p, page_sz);
}

static void cleanup(void)
{
	tst_rmdir();
}
