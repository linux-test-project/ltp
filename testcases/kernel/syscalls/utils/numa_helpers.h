/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Id: numa_helpers.h,v 1.2 2010/01/07 09:32:40 yaberauneya Exp $
 */

#ifndef LTP_NUMA_HELPERS_H
#define LTP_NUMA_HELPERS_H

#include "config.h"
#if HAVE_NUMA_H
#include <numa.h>

/*
 * libnuma v2 tests need to be rewritten because of the following issue:
 *
 * gcooper@orangebox /scratch/ltp-dev2/ltp/testcases/kernel/syscalls/get_mempolicy $ make all
 * make -C "/scratch/ltp-dev2/ltp/lib" -f "/scratch/ltp-dev2/ltp/lib/Makefile" all
 * make[1]: Entering directory `/scratch/ltp-dev2/ltp/lib'
 * make[1]: Nothing to be done for `all'.
 * make[1]: Leaving directory `/scratch/ltp-dev2/ltp/lib'
 * gcc -g -O2 -g -O2 -fno-strict-aliasing -pipe -Wall  -I/scratch/ltp-dev2/ltp/testcases/kernel/include -g -I/scratch/ltp-dev2/ltp/testcases/kernel/syscalls/get_mempolicy/../utils -DNUMA_VERSION1_COMPATIBILITY -I../../../../include -I../../../../include   -L../../../../lib  get_mempolicy01.c   -lltp -lnuma -o get_mempolicy01
 * get_mempolicy01.c: In function 'do_test':
 * get_mempolicy01.c:303: warning: passing argument 1 of 'nodemask_zero' from incompatible pointer type
 * get_mempolicy01.c:305: warning: passing argument 1 of 'nodemask_zero' from incompatible pointer type
 * get_mempolicy01.c:358: warning: passing argument 1 of 'nodemask_zero' from incompatible pointer type
 * get_mempolicy01.c:359: warning: passing argument 1 of 'nodemask_equal' from incompatible pointer type
 * get_mempolicy01.c:359: warning: passing argument 2 of 'nodemask_equal' from incompatible pointer type
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * gcooper@orangebox /scratch/ltp-dev2/ltp/testcases/kernel/syscalls/get_mempolicy $ ./get_mempolicy01 
 * get_mempolicy01    0  TINFO  :  (case00) START
 * Segmentation fault
 * gcooper@orangebox /scratch/ltp-dev2/ltp/testcases/kernel/syscalls/get_mempolicy $ gdb ./get_mempolicy01 
 * GNU gdb 6.8
 * Copyright (C) 2008 Free Software Foundation, Inc.
 * License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
 * and "show warranty" for details.
 * This GDB was configured as "x86_64-pc-linux-gnu"...
 * (gdb) r
 * Starting program: /scratch/ltp-dev2/ltp/testcases/kernel/syscalls/get_mempolicy/get_mempolicy01 
 * get_mempolicy01    0  TINFO  :  (case00) START
 *
 * Program received signal SIGSEGV, Segmentation fault.
 * 0x00007f3c46145a09 in ?? () from /usr/lib/libnuma.so.1
 * (gdb) where
 * #0  0x00007f3c46145a09 in ?? () from /usr/lib/libnuma.so.1
 * #1  0x00007f3c46145aae in numa_bitmask_clearall () from /usr/lib/libnuma.so.1
 * #2  0x0000000000402eeb in main (argc=1, argv=0x7ffffd3378b8) at /usr/include/numa.h:66
 *
 * It should be struct bitmask, which is a completely different structure
 * altogether from nodemask_t (hence that's why numa is segfaulting .. it's
 * accessing out-of-bounds memory).
 *
 * If it wasn't for the fact that this blocks newer systems from compiling,
 * this version wouldn't be checked in yet.
 */

/*
#if LIBNUMA_API_VERSION == 2
static inline void nodemask_dump(const char *header, const struct bitmask *mask)
#else
 */
static inline void nodemask_dump(const char *header, const nodemask_t *mask)
/*
#endif
 */
{
	int i;
	EPRINTF("%s", header);
	for (i = 0; i < NUMA_NUM_NODES/(sizeof(unsigned long)*8); i++) 
		EPRINTF(" 0x%08lx", mask->n[i]);
	EPRINTF("\n");
}

#else

/* 
 * Dummy datatype and stub function -- DO NOT FILL IN OR USE.
 * I don't give a rat's arse if gcc complains about -Wunused.
 */
typedef struct nodemask_t { };

static inline void nodemask_set(nodemask_t *mask, int node)
{
	return;
}

static inline void nodemask_dump(const char *header, const nodemask_t *mask) {
	return;
}

#endif

#endif
