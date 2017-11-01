/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

 /*

    Timer measuring library.

    The test is supposed to define sampling function and set it in the tst_test
    structure the rest of the work is then done by the library.

    int sample(int clk_id, long long usec)
    {
	// Any setup done here

	tst_timer_start(clk_id);
	// Call that is being measured sleeps for usec
	tst_timer_stop();
	tst_timer_sample();

	// Any cleanup done here

	// Non-zero return exits the test
    }

    struct tst_test test = {
	.scall = "syscall_name()",
	.sample = sample,
    };

  */

#ifndef TST_TIMER_TEST__
#define TST_TIMER_TEST__

#include "tst_test.h"
#include "tst_timer.h"

void tst_timer_sample(void);

# ifdef TST_NO_DEFAULT_MAIN
struct tst_test *tst_timer_test_setup(struct tst_test *test);
# endif /* TST_NO_DEFAULT_MAIN */
#endif /* TST_TIMER_TEST__ */
