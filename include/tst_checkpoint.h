/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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

#ifndef TST_CHECKPOINT__
#define TST_CHECKPOINT__

#include "tst_checkpoint_fn.h"

#define TST_CHECKPOINT_WAIT(id) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, 0);

#define TST_CHECKPOINT_WAIT2(id, msec_timeout) \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, msec_timeout);

#define TST_CHECKPOINT_WAKE(id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, 1);

#define TST_CHECKPOINT_WAKE2(id, nr_wake) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, nr_wake);

#define TST_CHECKPOINT_WAKE_AND_WAIT(id) \
        tst_safe_checkpoint_wake(__FILE__, __LINE__, NULL, id, 1); \
        tst_safe_checkpoint_wait(__FILE__, __LINE__, NULL, id, 0);

extern const char *tst_ipc_path;
extern char *const tst_ipc_envp[];

#endif /* TST_CHECKPOINT__ */
