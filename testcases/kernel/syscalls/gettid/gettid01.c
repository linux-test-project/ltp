/*
 * Crackerjack Project
 *
 * Copyright (C) 2007-2008, Hitachi, Ltd.
 * Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *            Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *            Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Id: gettid01.c,v 1.5 2009/10/26 14:55:47 subrata_modak Exp $
 *
 */

/* Porting from Crackerjack to LTP is done
   by Masatake YAMATO <yamato@redhat.com> */

#include <sys/types.h>
#include <linux/unistd.h>
#include <errno.h>

#include "test.h"

void setup();
void cleanup();

char *TCID = "gettid01";

int TST_TOTAL = 1;

pid_t my_gettid(void)
{
	return (pid_t) syscall(__NR_gettid);
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * The following loop checks looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(my_gettid());

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "gettid() Failed, errno=%d: %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "gettid() returned %ld",
				 TEST_RETURN);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
}
