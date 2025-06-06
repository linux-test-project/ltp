/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2025 Li Wang <liwang@redhat.com>
 */

#ifndef TST_ATOMIC_H__
#define TST_ATOMIC_H__

#include <stdint.h>
#include "config.h"

typedef int32_t tst_atomic_t;

#if HAVE_ATOMIC_MEMORY_MODEL == 1

/* Use __atomic built-ins (GCC >= 4.7, Clang >= 3.1), with sequential consistency. */

static inline int tst_atomic_add_return(int32_t i, tst_atomic_t *v)
{
	return __atomic_add_fetch(v, i, __ATOMIC_SEQ_CST);
}

static inline int32_t tst_atomic_load(tst_atomic_t *v)
{
	return __atomic_load_n(v, __ATOMIC_SEQ_CST);
}

static inline void tst_atomic_store(int32_t i, tst_atomic_t *v)
{
	__atomic_store_n(v, i, __ATOMIC_SEQ_CST);
}

#elif HAVE_SYNC_ADD_AND_FETCH == 1

/* Use __sync built-ins (GCC >= 4.1), with explicit memory barriers. */

static inline int tst_atomic_add_return(int32_t i, tst_atomic_t *v)
{
	return __sync_add_and_fetch(v, i);
}

static inline int32_t tst_atomic_load(tst_atomic_t *v)
{
	tst_atomic_t ret;

	__sync_synchronize();
	ret = *v;
	__sync_synchronize();
	return ret;
}

static inline void tst_atomic_store(int32_t i, tst_atomic_t *v)
{
	__sync_synchronize();
	*v = i;
	__sync_synchronize();
}

#else
# error "Your compiler does not support atomic operations (__atomic or __sync)"
#endif

static inline int tst_atomic_inc(tst_atomic_t *v)
{
	return tst_atomic_add_return(1, v);
}

static inline int tst_atomic_dec(tst_atomic_t *v)
{
	return tst_atomic_add_return(-1, v);
}

#endif /* TST_ATOMIC_H__ */
