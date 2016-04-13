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

#ifndef TST_ATOMIC_H__
#define TST_ATOMIC_H__

#include "config.h"

#if HAVE_SYNC_ADD_AND_FETCH == 1
static inline int tst_atomic_add_return(int i, int *v)
{
	return __sync_add_and_fetch(v, i);
}
#else
# error Your compiler does not provide __sync_add_and_fetch and LTP\
	implementation is missing for your architecture.
#endif

static inline int tst_atomic_inc(int *v)
{
	return tst_atomic_add_return(1, v);
}

#endif	/* TST_ATOMIC_H__ */
