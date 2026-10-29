// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2026
 */

/*
 * Checkpoint - easy to use parent-child synchronization.
 *
 * Checkpoint is based on futexes (man futex). The library allocates a page of
 * shared memory for futexes and the id is an offset to it which gives the user
 * up to page_size/sizeof(uint32_t) checkpoint pairs. Up to INT_MAX processes
 * can sleep on single id and can be woken up by single wake.
 */

#ifndef TSO_CHECKPOINT__
#define TSO_CHECKPOINT__

#include "test.h"
#include "tst_checkpoint_fn.h"

#define TST_SAFE_CHECKPOINT_WAIT(cleanup_fn, id) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, cleanup_fn, id, 0);

#define TST_SAFE_CHECKPOINT_WAIT2(cleanup_fn, id, msec_timeout) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, cleanup_fn, id, msec_timeout);

#define TST_SAFE_CHECKPOINT_WAKE(cleanup_fn, id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, cleanup_fn, id, 1);

#define TST_SAFE_CHECKPOINT_WAKE2(cleanup_fn, id, nr_wake) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, cleanup_fn, id, nr_wake);

#define TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(cleanup_fn, id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, cleanup_fn, id, 1); \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, cleanup_fn, id, 0);

#endif /* TSO_CHECKPOINT__ */
