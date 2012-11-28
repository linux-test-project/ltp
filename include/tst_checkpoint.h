/*
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
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

   Note that there are two differnt usages and two different wait and signal
   functions. The choice depends on whether we want parent wait for child or
   child for parent.

  */

#ifndef TST_CHECKPOINT
#define TST_CHECKPOINT

#include "test.h"

#define TST_CHECKPOINT_FIFO "tst_checkpoint_fifo"

struct tst_checkpoint {
	/* child return value in case of failure */
	int retval;
	/* timeout in msecs */
	unsigned int timeout;
};

/*
 * Checkpoint initializaton, must be done first.
 */
#define TST_CHECKPOINT_INIT(self) \
        tst_checkpoint_init(__FILE__, __LINE__, self)

void tst_checkpoint_init(const char *file, const int lineno,
                         struct tst_checkpoint *self);

/*
 * Wait called from parent. In case parent waits for child.
 */
#define TST_CHECKPOINT_PARENT_WAIT(cleanup_fn, self) \
        tst_checkpoint_parent_wait(__FILE__, __LINE__, (cleanup_fn), self)

void tst_checkpoint_parent_wait(const char *file, const int lineno,
                                void (*cleanup_fn)(void),
				struct tst_checkpoint *self);

/*
 * Wait called from child. In case child waits for parent.
 */
#define TST_CHECKPOINT_CHILD_WAIT(self) \
        tst_checkpoint_child_wait(__FILE__, __LINE__, self)

void tst_checkpoint_child_wait(const char *file, const int lineno,
                               struct tst_checkpoint *self);

/*
 * Signals parent that child has reached the checkpoint. Called from child.
 */
#define TST_CHECKPOINT_SIGNAL_PARENT(self) \
        tst_checkpoint_signal_parent(__FILE__, __LINE__, self)

void tst_checkpoint_signal_parent(const char *file, const int lineno,
                                  struct tst_checkpoint *self);

/*
 * Signals child that parent has reached the checkpoint. Called from parent.
 */
#define TST_CHECKPOINT_SIGNAL_CHILD(cleanup_fn, self) \
        tst_checkpoint_signal_child(__FILE__, __LINE__, (cleanup_fn), self)

void tst_checkpoint_signal_child(const char *file, const int lineno,
                                 void (*cleanup_fn)(void),
				 struct tst_checkpoint *self);

#endif /* TST_CHECKPOINT */
