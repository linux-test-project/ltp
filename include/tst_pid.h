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

#ifndef TST_PID_H__
#define TST_PID_H__

#include <sys/types.h>

/*
 * Get a pid value not used by the OS
 */
pid_t tst_get_unused_pid_(void (*cleanup_fn)(void));

/*
 * Returns number of free pids by substarction of the number of pids
 * currently used ('ps -eT') from max_pids
 */
int tst_get_free_pids_(void (*cleanup_fn)(void));

#ifdef TST_TEST_H__
static inline pid_t tst_get_unused_pid(void)
{
	return tst_get_unused_pid_(NULL);
}

static inline int tst_get_free_pids(void)
{
	return tst_get_free_pids_(NULL);
}
#else
static inline pid_t tst_get_unused_pid(void (*cleanup_fn)(void))
{
	return tst_get_unused_pid_(cleanup_fn);
}

static inline int tst_get_free_pids(void (*cleanup_fn)(void))
{
	return tst_get_free_pids_(cleanup_fn);
}
#endif

#endif /* TST_PID_H__ */
