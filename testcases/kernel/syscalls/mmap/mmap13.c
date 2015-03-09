/*
 * Copyright (c) 2013 FNST, DAN LI <li.dan@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify error signal SIGBUS.
 *  "Attempted access to a portion of the buffer that does not correspond
 *   to the file."
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  and an attempt to access the memory which does not correspond to the file
 *  should rise the signal SIGBUS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <setjmp.h>

#include "test.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap13";
int TST_TOTAL = 1;

static size_t page_sz;
static char *addr;
static int fildes;
static volatile sig_atomic_t pass;
static sigjmp_buf env;

static void setup(void);
static void cleanup(void);
static void sig_handler(int sig);

int main(int argc, char *argv[])
{
	int lc;
	char *ch;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		addr = mmap(NULL, page_sz * 2, PROT_READ | PROT_WRITE,
			    MAP_FILE | MAP_SHARED, fildes, 0);

		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap() failed on %s",
				 TEMPFILE);
			continue;
		}

		if (sigsetjmp(env, 1) == 0) {
			ch = addr + page_sz + 1;
			*ch = 0;
		}

		if (pass)
			tst_resm(TPASS, "Got SIGBUS "
					"as expected");
		else
			tst_resm(TFAIL, "Invalid access not "
						"rise SIGBUS");

		if (munmap(addr, page_sz * 2) != 0)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "failed to unmap the mmapped pages");

		pass = 0;
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, sig_handler, cleanup);

	TEST_PAUSE;

	page_sz = getpagesize();

	tst_tmpdir();

	fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0766);
	if (fildes < 0)
		tst_brkm(TFAIL | TERRNO, cleanup, "opening %s failed",
			 TEMPFILE);

	if (ftruncate(fildes, page_sz / 2) == -1)
		tst_brkm(TFAIL | TERRNO, cleanup, "ftruncate %s failed",
			 TEMPFILE);
}

/*
 *   This function gets executed when the test process receives
 *   the signal SIGBUS while trying to access the memory which
 *   does not correspond to the file.
 */
static void sig_handler(int sig)
{
	if (sig == SIGBUS) {
		pass = 1;
		siglongjmp(env, 1);
	} else {
		tst_brkm(TBROK, cleanup, "received an unexpected signal");
	}
}

static void cleanup(void)
{
	close(fildes);
	tst_rmdir();
}
