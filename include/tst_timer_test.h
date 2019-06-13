// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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
