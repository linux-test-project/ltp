// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef HAVE_CPUID_H
#  include <cpuid.h>
#endif

#ifndef LAPI_CPUID_H__
#define LAPI_CPUID_H__

/*
 * gcc cpuid.h provides __cpuid_count() since v4.4.
 * Clang/LLVM cpuid.h provides __cpuid_count() since v3.4.0.
 *
 * Provide local define for tests needing __cpuid_count() because
 * ltp needs to work in older environments that do not yet
 * have __cpuid_count().
 */
#ifndef __cpuid_count
#  if defined(__i386__) || defined(__x86_64__)
#define __cpuid_count(level, count, a, b, c, d) ({			\
	__asm__ __volatile__ ("cpuid\n\t"				\
			      : "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
			      : "0" (level), "2" (count));		\
})
#  else
#define __cpuid_count(level, count, a, b, c, d)
#  endif
#endif

#endif /* LAPI_CPUID_H__ */
