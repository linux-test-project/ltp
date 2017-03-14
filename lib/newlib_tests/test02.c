/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * The tst_resm() and tst_brkm() should be rerouted to the new lib.
 */

#include "tst_test.h"

void tst_resm_(char *, int, int, char *);
void tst_brkm_(char *, int, int, void (*)(void), char *);

static void cleanup(void)
{
}

static void do_test(unsigned int i)
{
	switch (i) {
	case 0:
		tst_resm_(__FILE__, __LINE__, TINFO, "info message");
		tst_resm_(__FILE__, __LINE__, TPASS, "passed message");
	break;
	case 1:
		tst_brkm_(__FILE__, __LINE__, TCONF, cleanup, "Non-NULL cleanup");
	break;
	}
}

static struct tst_test test = {
	.tcnt = 2,
	.test = do_test,
};
