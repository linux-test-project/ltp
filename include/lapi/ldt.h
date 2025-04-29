/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

#ifndef LAPI_LDT_H__
#define LAPI_LDT_H__

#include "config.h"
#include "lapi/syscalls.h"

#ifdef HAVE_ASM_LDT_H
#include <asm/ldt.h>
#else
struct user_desc {
	unsigned int entry_number;
	unsigned int base_addr;
	unsigned int limit;
	unsigned int seg_32bit : 1;
	unsigned int contents : 2;
	unsigned int read_exec_only : 1;
	unsigned int limit_in_pages : 1;
	unsigned int seg_not_present : 1;
	unsigned int useable : 1;
#ifdef __x86_64__
	unsigned int lm : 1;
#endif /* __x86_64__ */
};
#endif /* HAVE_ASM_LDT_H */

static inline int modify_ldt(int func, const struct user_desc *ptr,
			     unsigned long bytecount)
{
	return tst_syscall(__NR_modify_ldt, func, ptr, bytecount);
}

static inline int safe_modify_ldt(const char *file, const int lineno, int func,
				  const struct user_desc *ptr,
				  unsigned long bytecount)
{
	int rval;

	rval = modify_ldt(func, ptr, bytecount);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "modify_ldt(%d, %p, %lu)", func, ptr, bytecount);
	} else if (rval) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "modify_ltd(%d, %p, %lu) invalid retval %i", func, ptr,
			 bytecount, rval);
	}

	return rval;
}

#define SAFE_MODIFY_LDT(func, ptr, bytecount) \
	safe_modify_ldt(__FILE__, __LINE__, (func), (ptr), (bytecount))

#endif /* LAPI_LDT_H__ */
