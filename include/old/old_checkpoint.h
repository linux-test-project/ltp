/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
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

   Checkpoint - easy to use parent-child synchronization.

   Checkpoint is based on futexes (man futex). The library allocates a page of
   shared memory for futexes and the id is an offset to it which gives the user
   up to page_size/sizeof(uint32_t) checkpoint pairs. Up to INT_MAX processes
   can sleep on single id and can be woken up by single wake.

  */

#ifndef OLD_CHECKPOINT__
#define OLD_CHECKPOINT__

#include "test.h"
#include "tst_checkpoint_fn.h"

/*
 * Checkpoint initializaton, must be done first.
 *
 * NOTE: tst_tmpdir() must be called beforehand.
 */
#define TST_CHECKPOINT_INIT(cleanup_fn) \
	tst_checkpoint_init(__FILE__, __LINE__, cleanup_fn)

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

#endif /* OLD_CHECKPOINT__ */
