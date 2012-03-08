/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
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
		"mov %%ebx, %%edi;" // save ebx (for PIC)
		"cpuid;"
		"mov %%ebx, %%esi;" // pass to caller
		"mov %%edi, %%ebx;" // restore ebx
		: "+a" (_eax), "=S" (_ebx), "=c" (_ecx), "=d" (_edx)
		:       /* inputs: eax is handled above */
		: "edi" /* clobbers: we hit edi directly */
	);
	if (eax) *eax = _eax;
	if (ebx) *ebx = _ebx;
	if (ecx) *ecx = _ecx;
	if (edx) *edx = _edx;
#endif
}

#endif
