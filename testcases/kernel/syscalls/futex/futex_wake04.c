/*
 * Copyright (C) 2015  Yi Zhang <wetpzy@gmail.com>
 *                     Li Wang <liwang@redhat.com>
 *
 * Licensed under the GNU GPLv2 or later.
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
 /* DESCRIPTION:
 *
 *   It is a regression test for commit:
 *   http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/
 *   commit/?id=13d60f4
 *
 *   The implementation of futex doesn't produce unique keys for futexes
 *   in shared huge pages, so threads waiting on different futexes may
 *   end up on the same wait list. This results in incorrect threads being
 *   woken by FUTEX_WAKE.
 *
 *   Needs to be run as root unless there are already enough huge pages available.
 *   In the fail case, which happens in the CentOS-6.6 kernel (2.6.32-504.8.1),
 *   the tests hangs until it times out after a 30-second wait.
 *
 */

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>

#include "test.h"
#include "safe_macros.h"
#include "futextest.h"
#include "futex_utils.h"
#include "lapi/mmap.h"

#define PATH_MEMINFO "/proc/meminfo"
#define PATH_NR_HUGEPAGES "/proc/sys/vm/nr_hugepages"
#define PATH_HUGEPAGES	"/sys/kernel/mm/hugepages/"

const char *TCID = "futex_wake04";
const int TST_TOTAL = 1;

static futex_t *futex1, *futex2;

static struct timespec to = {.tv_sec = 30, .tv_nsec = 0};

static long orig_hugepages;

static void setup(void)
{
	tst_require_root();

	if ((tst_kvercmp(2, 6, 32)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			"that are 2.6.32 or higher");
	}

	if (access(PATH_HUGEPAGES, F_OK))
		tst_brkm(TCONF, NULL, "Huge page is not supported.");

	tst_tmpdir();

	SAFE_FILE_SCANF(NULL, PATH_NR_HUGEPAGES, "%ld", &orig_hugepages);

	if (orig_hugepages <= 0)
		SAFE_FILE_PRINTF(NULL, PATH_NR_HUGEPAGES, "%d", 1);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (orig_hugepages <= 0)
		SAFE_FILE_PRINTF(NULL, PATH_NR_HUGEPAGES, "%ld", orig_hugepages);

	tst_rmdir();
}

static int read_hugepagesize(void)
{
	FILE *fp;
	char line[BUFSIZ], buf[BUFSIZ];
	int val;

	fp = SAFE_FOPEN(cleanup, PATH_MEMINFO, "r");
	while (fgets(line, BUFSIZ, fp) != NULL) {
		if (sscanf(line, "%64s %d", buf, &val) == 2)
			if (strcmp(buf, "Hugepagesize:") == 0) {
				SAFE_FCLOSE(cleanup, fp);
				return 1024 * val;
			}
	}

	SAFE_FCLOSE(cleanup, fp);
	tst_brkm(TBROK, NULL, "can't find \"%s\" in %s",
			"Hugepagesize:", PATH_MEMINFO);
}

static void *wait_thread1(void *arg LTP_ATTRIBUTE_UNUSED)
{
	futex_wait(futex1, *futex1, &to, 0);

	return NULL;
}

static void *wait_thread2(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int res;

	res = futex_wait(futex2, *futex2, &to, 0);
	if (!res)
		tst_resm(TPASS, "Hi hydra, thread2 awake!");
	else
		tst_resm(TFAIL, "Bug: wait_thread2 did not wake after 30 secs.");

	return NULL;
}

static void wakeup_thread2(void)
{
	void *addr;
	int hpsz, pgsz, res;
	pthread_t th1, th2;

	hpsz = read_hugepagesize();
	tst_resm(TINFO, "Hugepagesize %i", hpsz);

	/*allocate some shared memory*/
	addr = mmap(NULL, hpsz, PROT_WRITE | PROT_READ,
	            MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

	if (addr == MAP_FAILED) {
		if (errno == ENOMEM) {
			tst_brkm(TCONF, NULL,
				 "Cannot allocate hugepage, memory too fragmented?");
		}

		tst_brkm(TBROK | TERRNO, NULL, "Cannot allocate hugepage");
	}

	pgsz = getpagesize();

	/*apply the first subpage to futex1*/
	futex1 = addr;
	*futex1 = 0;
	/*apply the second subpage to futex2*/
	futex2 = (futex_t *)((char *)addr + pgsz);
	*futex2 = 0;

	/*thread1 block on futex1 first,then thread2 block on futex2*/
	res = pthread_create(&th1, NULL, wait_thread1, NULL);
	if (res) {
		tst_brkm(TBROK, NULL, "pthread_create(): %s",
				tst_strerrno(res));
	}

	res = pthread_create(&th2, NULL, wait_thread2, NULL);
	if (res) {
		tst_brkm(TBROK, NULL, "pthread_create(): %s",
				tst_strerrno(res));
	}

	while (wait_for_threads(2))
		usleep(1000);

	futex_wake(futex2, 1, 0);

	res = pthread_join(th2, NULL);
	if (res)
		tst_brkm(TBROK, NULL, "pthread_join(): %s", tst_strerrno(res));

	futex_wake(futex1, 1, 0);

	res = pthread_join(th1, NULL);
	if (res)
		tst_brkm(TBROK, NULL, "pthread_join(): %s", tst_strerrno(res));

	SAFE_MUNMAP(NULL, addr, hpsz);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		wakeup_thread2();

	cleanup();
	tst_exit();
}
