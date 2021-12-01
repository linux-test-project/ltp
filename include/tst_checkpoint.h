/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_CHECKPOINT__
#define TST_CHECKPOINT__

#include "tst_checkpoint_fn.h"

#define TST_CHECKPOINT_WAIT(id) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, 0)

#define TST_CHECKPOINT_WAIT2(id, msec_timeout) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, msec_timeout)

#define TST_CHECKPOINT_WAKE(id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, 1)

#define TST_CHECKPOINT_WAKE2(id, nr_wake) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, nr_wake)

#define TST_CHECKPOINT_WAKE_AND_WAIT(id) do { \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, 1); \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, 0); \
} while (0)

extern const char *tst_ipc_path;

#endif /* TST_CHECKPOINT__ */
