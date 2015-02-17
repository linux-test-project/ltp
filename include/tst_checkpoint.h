/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
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
 */

 /*

   Checkpoint - easy to use parent-child synchronization.

   Checkpoint is based on futexes (man futex). The library allocates a page of
   shared memory for futexes and the id is an offset to it which gives the user
   up to page_size/sizeof(uint32_t) checkpoint pairs. Up to INT_MAX processes
   can sleep on single id and can be woken up by single wake.

  */

#ifndef TST_CHECKPOINT
#define TST_CHECKPOINT

#include "test.h"

/*
 * Checkpoint initializaton, must be done first.
 *
 * NOTE: tst_tmpdir() must be called beforehand.
 */
#define TST_CHECKPOINT_INIT(cleanup_fn) \
	tst_checkpoint_init(__FILE__, __LINE__, cleanup_fn)

void tst_checkpoint_init(const char *file, const int lineno,
			 void (*cleanup_fn)(void));



/*
 * Waits for wakeup.
 *
 * @id: Checkpoint id, possitive number
 * @msec_timeout: Timeout in miliseconds, 0 == no timeout
 */
int tst_checkpoint_wait(unsigned int id, unsigned int msec_timeout);

/*
 * Wakes up sleeping process(es)/thread(s).
 *
 * @id: Checkpoint id, possitive number
 * @nr_wake: Number of processes/threads to wake up
 * @msec_timeout: Timeout in miliseconds, 0 == no timeout
 */
int tst_checkpoint_wake(unsigned int id, unsigned int nr_wake,
                        unsigned int msec_timeout);

void tst_safe_checkpoint_wait(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id);

void tst_safe_checkpoint_wake(const char *file, const int lineno,
                              void (*cleanup_fn)(void), unsigned int id,
                              unsigned int nr_wake);

#define TST_SAFE_CHECKPOINT_WAIT(cleanup_fn, id) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, cleanup_fn, id);

#define TST_SAFE_CHECKPOINT_WAKE(cleanup_fn, id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, cleanup_fn, id, 1);

#define TST_SAFE_CHECKPOINT_WAKE2(cleanup_fn, id, nr_wake) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, cleanup_fn, id, nr_wake);

#define TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(cleanup_fn, id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, cleanup_fn, id, 1); \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, cleanup_fn, id);

#endif /* TST_CHECKPOINT */
