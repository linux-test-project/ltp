/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2025 Li Wang <liwang@redhat.com>
 */

#ifndef TST_ATOMIC_H__
#define TST_ATOMIC_H__

#include "config.h"

#if HAVE_ATOMIC_MEMORY_MODEL == 1

/* Use __atomic built-ins (GCC >= 4.7, Clang >= 3.1), with sequential consistency. */

static inline int tst_atomic_add_return(int i, int *v)
{
	return __atomic_add_fetch(v, i, __ATOMIC_SEQ_CST);
}

static inline int tst_atomic_load(int *v)
{
	return __atomic_load_n(v, __ATOMIC_SEQ_CST);
}

static inline void tst_atomic_store(int i, int *v)
{
	__atomic_store_n(v, i, __ATOMIC_SEQ_CST);
}

#elif HAVE_SYNC_ADD_AND_FETCH == 1

/* Use __sync built-ins (GCC >= 4.1), with explicit memory barriers. */

static inline int tst_atomic_add_return(int i, int *v)
{
	return __sync_add_and_fetch(v, i);
}

static inline int tst_atomic_load(int *v)
{
	int ret;

	__sync_synchronize();
	ret = *v;
	__sync_synchronize();
	return ret;
}

static inline void tst_atomic_store(int i, int *v)
{
	__sync_synchronize();
	*v = i;
	__sync_synchronize();
}

#else
# error "Your compiler does not support atomic operations (__atomic or __sync)"
#endif

static inline int tst_atomic_inc(int *v)
{
	return tst_atomic_add_return(1, v);
}

static inline int tst_atomic_dec(int *v)
{
	return tst_atomic_add_return(-1, v);
}

#endif	/* TST_ATOMIC_H__ */
