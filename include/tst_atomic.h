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

#elif defined(__i386__) || defined(__x86_64__)
static inline int tst_atomic_add_return(int i, int *v)
{
	int __ret = i;

	/*
	 * taken from arch/x86/include/asm/cmpxchg.h
	 * Since we always pass int sized parameter, we can simplify it
	 * and cherry-pick only that specific case.
	 *
	switch (sizeof(*v)) {
	case 1:
		asm volatile ("lock; xaddb %b0, %1\n"
			: "+q" (__ret), "+m" (*v) : : "memory", "cc");
		break;
	case 2:
		asm volatile ("lock; xaddw %w0, %1\n"
			: "+r" (__ret), "+m" (*v) : : "memory", "cc");
		break;
	case 4:
		asm volatile ("lock; xaddl %0, %1\n"
			: "+r" (__ret), "+m" (*v) : : "memory", "cc");
		break;
	case 8:
		asm volatile ("lock; xaddq %q0, %1\n"
			: "+r" (__ret), "+m" (*v) : : "memory", "cc");
		break;
	default:
		__xadd_wrong_size();
	}
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

#elif defined(__s390__) || defined(__s390x__)
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
#define smp_mb()
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

#elif defined (__aarch64__)
static inline int tst_atomic_add_return(int i, int *v)
{
	unsigned long tmp;
	int result;

	__asm__ __volatile__(
"       prfm    pstl1strm, %2	\n"
"1:     ldxr 	%w0, %2		\n"
"       add	%w0, %w0, %w3	\n"
"       stlxr	%w1, %w0, %2	\n"
"       cbnz	%w1, 1b		\n"
"       dmb ish			\n"
	: "=&r" (result), "=&r" (tmp), "+Q" (*v)
	: "Ir" (i)
	: "memory");

	return result;
}

#else /* HAVE_SYNC_ADD_AND_FETCH == 1 */
# error Your compiler does not provide __sync_add_and_fetch and LTP\
	implementation is missing for your architecture.
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
