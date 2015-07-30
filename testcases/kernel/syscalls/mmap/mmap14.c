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
 *  Verify MAP_LOCKED works fine.
 *  "Lock the pages of the mapped region into memory in the manner of mlock(2)."
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  and this region should be locked into memory.
 */
#include <stdio.h>
#include <sys/mman.h>

#include "test.h"

#define TEMPFILE        "mmapfile"
#define MMAPSIZE        (1UL<<20)
#define LINELEN         256

char *TCID = "mmap14";
int TST_TOTAL = 1;

static char *addr;

static void getvmlck(unsigned int *lock_sz);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	unsigned int sz_before;
	unsigned int sz_after;
	unsigned int sz_ch;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		getvmlck(&sz_before);

		addr = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_LOCKED | MAP_ANONYMOUS,
			    -1, 0);

		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap of %s failed", TEMPFILE);
			continue;
		}

		getvmlck(&sz_after);

		sz_ch = sz_after - sz_before;
		if (sz_ch == MMAPSIZE / 1024) {
			tst_resm(TPASS, "Functionality of mmap() "
					"successful");
		} else {
			tst_resm(TFAIL, "Expected %luK locked, "
					"get %uK locked",
					MMAPSIZE / 1024, sz_ch);
		}

		if (munmap(addr, MMAPSIZE) != 0)
			tst_brkm(TFAIL | TERRNO, NULL, "munmap failed");
	}

	cleanup();
	tst_exit();
}

void getvmlck(unsigned int *lock_sz)
{
	int ret;
	char line[LINELEN];
	FILE *fstatus = NULL;

	fstatus = fopen("/proc/self/status", "r");
	if (fstatus == NULL)
		tst_brkm(TFAIL | TERRNO, NULL, "Open dev status failed");

	while (fgets(line, LINELEN, fstatus) != NULL)
		if (strstr(line, "VmLck") != NULL)
			break;

	ret = sscanf(line, "%*[^0-9]%d%*[^0-9]", lock_sz);
	if (ret != 1)
		tst_brkm(TFAIL | TERRNO, NULL, "Get lock size failed");

	fclose(fstatus);
}

static void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
