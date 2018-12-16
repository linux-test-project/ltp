// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * 1. Block on a bitset futex and wait for timeout, the difference between
 *    normal futex and bitset futex is that that the later have absolute timeout.
 * 2. Check that the futex waited for expected time.
 */

#include <errno.h>

#include "tst_test.h"
#include "tst_timer.h"
#include "futextest.h"

#define USE_CLOCK CLOCK_REALTIME

#include "futex_wait_bitset.h"
