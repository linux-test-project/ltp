/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/* The LTP library has some of its own atomic synchronisation primitives
 * contained in this file. Generally speaking these should not be used
 * directly in tests for synchronisation, instead use tst_checkpoint.h,
 * tst_fuzzy_sync.h or the POSIX library.
 *
 * Notes on compile and runtime memory barriers and atomics.
 *
 * Within the LTP library we have three concerns when accessing variables
 * shared by multiple threads or processes:
 *
 * (1) Removal or reordering of accesses by the compiler.
 * (2) Atomicity of addition.
 * (3) LOAD-STORE ordering between threads.
 *
 * The first (1) is the most likely to cause an error if not properly
 * handled. We avoid it by using volatile variables and statements which will
 * not be removed or reordered by the compiler during optimisation. This includes
 * the __atomic and __sync intrinsics and volatile asm statements marked with
 * "memory" as well as variables marked with volatile.
 *
 * On any platform Linux is likely to run on, a LOAD (fetch) or STORE of a
 * 32-bit integer will be atomic. However fetching and adding to a variable is
 * quite likely not; so for (2) we need to ensure we use atomic addition.
 *
 * Finally, for tst_fuzzy_sync at least, we need to ensure that LOADs and
 * STOREs of any shared variables (including non-atomics) that are made
 * between calls to tst_fzsync_wait are completed (globally visible) before
 * tst_fzsync_wait completes. For this, runtime memory and instruction
 * barriers are required in addition to compile time.
 *
 * We use full sequential ordering (__ATOMIC_SEQ_CST) for the sake of
 * simplicity. LTP tests tend to be syscall heavy so any performance gain from
 * using a weaker memory model is unlikely to result in a relatively large
 * performance improvement while at the same time being a potent source of
 * confusion.
 *
 * Likewise, for the fallback ASM, the simplest "definitely will work, always"
 * approach is preferred over anything more performant.
 *
 * Also see Documentation/memory-barriers.txt in the kernel tree and
 * https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
 * terminology may vary between sources.
 */

#ifndef TST_ATOMIC_H__
#define TST_ATOMIC_H__

#include "config.h"

#if HAVE_ATOMIC_MEMORY_MODEL == 1
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

#elif defined(__i386__) || defined(__x86_64__)
# define LTP_USE_GENERIC_LOAD_STORE_ASM 1

static inline int tst_atomic_add_return(int i, int *v)
{
	int __ret = i;

	/*
	 * taken from arch/x86/include/asm/cmpxchg.h
	 */
	asm volatile ("lock; xaddl %0, %1\n"
		: "+r" (__ret), "+m" (*v) : : "memory", "cc");

	return i + __ret;
}

#elif defined(__powerpc__) || defined(__powerpc64__)
static inline int tst_atomic_add_return(int i, int *v)
{
	int t;

	/* taken from arch/powerpc/include/asm/atomic.h */
	asm volatile(
		"	sync\n"
		"1:	lwarx	%0,0,%2		# atomic_add_return\n"
		"	add %0,%1,%0\n"
		"	stwcx.	%0,0,%2 \n"
		"	bne-	1b\n"
		"	sync\n"
		: "=&r" (t)
		: "r" (i), "r" (v)
		: "cc", "memory");

	return t;
}

static inline int tst_atomic_load(int *v)
{
	int ret;

	asm volatile("sync\n" : : : "memory");
	ret = *v;
	asm volatile("sync\n" : : : "memory");

	return ret;
}

static inline void tst_atomic_store(int i, int *v)
{
	asm volatile("sync\n" : : : "memory");
	*v = i;
	asm volatile("sync\n" : : : "memory");
}

#elif defined(__s390__) || defined(__s390x__)
# define LTP_USE_GENERIC_LOAD_STORE_ASM 1

static inline int tst_atomic_add_return(int i, int *v)
{
	int old_val, new_val;

	/* taken from arch/s390/include/asm/atomic.h */
	asm volatile(
		"	l	%0,%2\n"
		"0:	lr	%1,%0\n"
		"	ar	%1,%3\n"
		"	cs	%0,%1,%2\n"
		"	jl	0b"
		: "=&d" (old_val), "=&d" (new_val), "+Q" (*v)
		: "d" (i)
		: "cc", "memory");

	return old_val + i;
}

#elif defined(__arc__)

/*ARCv2 defines the smp barriers */
#ifdef __ARC700__
#define smp_mb()	asm volatile("" : : : "memory")
#else
#define smp_mb()	asm volatile("dmb 3\n" : : : "memory")
#endif

static inline int tst_atomic_add_return(int i, int *v)
{
	unsigned int val;

	smp_mb();

	asm volatile(
		"1:	llock   %[val], [%[ctr]]	\n"
		"	add     %[val], %[val], %[i]	\n"
		"	scond   %[val], [%[ctr]]	\n"
		"	bnz     1b			\n"
		: [val]	"=&r"	(val)
		: [ctr]	"r"	(v),
		  [i]	"ir"	(i)
		: "cc", "memory");

	smp_mb();

	return val;
}

static inline int tst_atomic_load(int *v)
{
	int ret;

	smp_mb();
	ret = *v;
	smp_mb();

	return ret;
}

static inline void tst_atomic_store(int i, int *v)
{
	smp_mb();
	*v = i;
	smp_mb();
}

#elif defined (__aarch64__)
static inline int tst_atomic_add_return(int i, int *v)
{
	unsigned long tmp;
	int result;

	__asm__ __volatile__(
"       prfm    pstl1strm, %2	\n"
"1:     ldaxr	%w0, %2		\n"
"       add	%w0, %w0, %w3	\n"
"       stlxr	%w1, %w0, %2	\n"
"       cbnz	%w1, 1b		\n"
"       dmb ish			\n"
	: "=&r" (result), "=&r" (tmp), "+Q" (*v)
	: "Ir" (i)
	: "memory");

	return result;
}

/* We are using load and store exclusive (ldaxr & stlxr) instructions to try
 * and help prevent the tst_atomic_load and, more likely, tst_atomic_store
 * functions from interfering with tst_atomic_add_return which takes advantage
 * of exclusivity. It is not clear if this is a good idea or not, but does
 * mean that all three functions are very similar.
 */
static inline int tst_atomic_load(int *v)
{
	int ret;
	unsigned long tmp;

	asm volatile("//atomic_load			\n"
		"	prfm	pstl1strm,  %[v]	\n"
		"1:	ldaxr	%w[ret], %[v]		\n"
		"	stlxr   %w[tmp], %w[ret], %[v]  \n"
		"	cbnz    %w[tmp], 1b		\n"
		"	dmb ish				\n"
		: [tmp] "=&r" (tmp), [ret] "=&r" (ret), [v] "+Q" (*v)
		: : "memory");

	return ret;
}

static inline void tst_atomic_store(int i, int *v)
{
	unsigned long tmp;

	asm volatile("//atomic_store			\n"
		"	prfm	pstl1strm, %[v]		\n"
		"1:	ldaxr	%w[tmp], %[v]		\n"
		"	stlxr   %w[tmp], %w[i], %[v]	\n"
		"	cbnz    %w[tmp], 1b		\n"
		"	dmb ish				\n"
		: [tmp] "=&r" (tmp), [v] "+Q" (*v)
		: [i] "r" (i)
		: "memory");
}

#elif defined(__sparc__) && defined(__arch64__)
# define LTP_USE_GENERIC_LOAD_STORE_ASM 1
static inline int tst_atomic_add_return(int i, int *v)
{
	int ret, tmp;

	/* Based on arch/sparc/lib/atomic_64.S with the exponential backoff
	 * function removed because we are unlikely to have a large (>= 16?)
	 * number of cores continuously trying to update one variable.
	 */
	asm volatile("/*atomic_add_return*/		\n"
		"1:	ldsw	[%[v]], %[ret];		\n"
		"	add	%[ret], %[i], %[tmp];	\n"
		"	cas	[%[v]], %[ret], %[tmp];	\n"
		"	cmp	%[ret], %[tmp];		\n"
		"	bne,pn	%%icc, 1b;		\n"
		"	nop;				\n"
		"	add	%[ret], %[i], %[ret];	\n"
		: [ret] "=r&" (ret), [tmp] "=r&" (tmp)
		: [i] "r" (i), [v] "r" (v)
		: "memory", "cc");

	return ret;
}

#else /* HAVE_SYNC_ADD_AND_FETCH == 1 */
# error Your compiler does not provide __atomic_add_fetch, __sync_add_and_fetch \
        and an LTP implementation is missing for your architecture.
#endif

#ifdef LTP_USE_GENERIC_LOAD_STORE_ASM
static inline int tst_atomic_load(int *v)
{
	int ret;

	asm volatile("" : : : "memory");
	ret = *v;
	asm volatile("" : : : "memory");

	return ret;
}

static inline void tst_atomic_store(int i, int *v)
{
	asm volatile("" : : : "memory");
	*v = i;
	asm volatile("" : : : "memory");
}
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
