/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
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
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <stdint.h>
#include <inttypes.h>

#include "test.h"

#ifndef BSIZE
#define BSIZE  BBSIZE
#endif

void setup();
void cleanup();

#define MAX_SIZE_LC	1000	/* loop count test will reach max size */

char *TCID = "brk01";
int TST_TOTAL = 1;

uintptr_t Max_brk_byte_size;
uintptr_t Beg_brk_val;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;
	int incr;
	uintptr_t nbrkpt;		/* new brk point value */
	uintptr_t cur_brk_val;	/* current size returned by sbrk */
	uintptr_t aft_brk_val;	/* current size returned by sbrk */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * Attempt to control how fast we get to test max size.
	 * Every MAX_SIZE_LC'th lc will be fastest test will reach max size.
	 */
	incr = (Max_brk_byte_size - Beg_brk_val) / (MAX_SIZE_LC / 2);

	if ((incr * 2) < 4096)	/* make sure that process will grow */
		incr += 4096 / 2;

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Determine new value to give brk
		 * Every even lc value, grow by 2 incr and
		 * every odd lc value, strink by one incr.
		 * If lc is equal to 3, no change, special case.
		 */
		cur_brk_val = (uintptr_t)sbrk(0);
		if (lc == 3) {
			nbrkpt = cur_brk_val;	/* no change, special one time case */
		} else if ((lc % 2) == 0) {
			/*
			 * grow
			 */
			nbrkpt = cur_brk_val + (2 * incr);

			if (nbrkpt > Max_brk_byte_size)
				nbrkpt = Beg_brk_val;	/* start over */

		} else {
			/*
			 * shrink
			 */
			nbrkpt = cur_brk_val - incr;
		}

/****
    printf("cur_brk_val = %d, nbrkpt = %d, incr = %d, lc = %d\n",
	cur_brk_val, nbrkpt, incr, lc);
****/

		TEST(brk((char *)nbrkpt));

		if (TEST_RETURN == -1) {

			aft_brk_val = (uintptr_t)sbrk(0);
			tst_resm(TFAIL | TTERRNO,
				 "brk(%"PRIuPTR") failed (size before %"PRIuPTR", after %"PRIuPTR")",
				 nbrkpt, cur_brk_val, aft_brk_val);

		} else {
			aft_brk_val = (uintptr_t)sbrk(0);
			if (aft_brk_val == nbrkpt) {

				tst_resm(TPASS,
					 "brk(%"PRIuPTR") returned %"PRIuPTR", new size verified by sbrk",
					 nbrkpt, TEST_RETURN);
			} else {
				tst_resm(TFAIL,
					 "brk(%"PRIuPTR") returned %"PRIuPTR", sbrk before %"PRIuPTR", after %"PRIuPTR"",
					 nbrkpt, TEST_RETURN,
					 cur_brk_val, aft_brk_val);
			}
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	unsigned long max_size;
	int ncpus;
	unsigned long ulim_sz;
	unsigned long usr_mem_sz;
	struct rlimit lim;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/*if ((ulim_sz=ulimit(3,0)) == -1)
	   tst_brkm(TBROK|TERRNO, cleanup, "ulimit(3,0) failed"); */

	if (getrlimit(RLIMIT_DATA, &lim) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "getrlimit(RLIMIT_DATA,%p) failed", &lim);
	ulim_sz = lim.rlim_cur;

	/*
	 * On IRIX, which is a demand paged system, memory is managed
	 * different than on Crays systems.  For now, pick some value.
	 */
	usr_mem_sz = 1024 * 1024 * sizeof(long);

	if ((ncpus = sysconf(_SC_NPROCESSORS_ONLN)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "sysconf(_SC_NPROCESSORS_ONLN) failed");

	/*
	 * allow 2*ncpus copies to run.
	 * never attempt to take more than a * 1/4 of memory (by single test)
	 */

	max_size = MIN(ulim_sz, usr_mem_sz);

	max_size = max_size / (2 * ncpus);

	if (max_size > (usr_mem_sz / 4))
		max_size = usr_mem_sz / 4;	/* only fourth mem by single test */

	Beg_brk_val = (uintptr_t)sbrk(0);

	/*
	 * allow at least 4 times a big as current.
	 * This will override above code.
	 */
	if (max_size < Beg_brk_val * 4)	/* running on small mem and/or high # cpus */
		max_size = Beg_brk_val * 4;

	Max_brk_byte_size = max_size;

	TEST_PAUSE;

}

void cleanup(void)
{
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "test is not available on uClinux");
}

#endif /* if !defined(UCLINUX) */
