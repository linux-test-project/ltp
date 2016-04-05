/*
 * Copyright (c) 2012-2013 The Chromium OS Authors. All rights reserved.
 *
 * Licensed under the BSD 3-clause.
 */

#ifndef __LTP_CPUID_H__
#define __LTP_CPUID_H__

static inline void cpuid(unsigned int info, unsigned int *eax, unsigned int *ebx,
                         unsigned int *ecx, unsigned int *edx)
{
#if defined(__i386__) || defined(__x86_64__)
	unsigned int _eax = info, _ebx, _ecx, _edx;
	asm volatile(
# ifdef __i386__
		"xchg %%ebx, %%esi;" /* save ebx (for PIC) */
		"cpuid;"
		"xchg %%esi, %%ebx;" /* restore ebx & pass to caller */
		: "=S" (_ebx),
# else
		"cpuid;"
		: "=b" (_ebx),
# endif
		  "+a" (_eax), "=c" (_ecx), "=d" (_edx)
		: /* inputs: eax is handled above */
	);
	if (eax) *eax = _eax;
	if (ebx) *ebx = _ebx;
	if (ecx) *ecx = _ecx;
	if (edx) *edx = _edx;
#endif
}

#endif
