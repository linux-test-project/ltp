/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Some old libcs (like glibc < 2.7) do not provide interfaces for
 * dynamically sized cpu sets, but provide only static cpu_set_t type
 * with no more than CPU_SETSIZE cpus in it.
 *
 * This file is a wrapper of the dynamic interfaces using the static ones.
 *
 * If the number of cpus available on the system is greater than
 * CPU_SETSIZE, this interface will not work. Update libc in this case :)
 */

#define _GNU_SOURCE
#include <sched.h>

#ifndef LTP_SCHED_SETAFFINITY_H
#define LTP_SCHED_SETAFFINITY_H

#ifndef CPU_ALLOC
#define CPU_ALLOC(ncpus) malloc(sizeof(cpu_set_t)); \
if (ncpus > CPU_SETSIZE) { \
	tst_brkm(TCONF, cleanup, \
		"Your libc does not support masks with %ld cpus", ncpus); \
}
#endif

#ifndef CPU_FREE
#define CPU_FREE(ptr) free(ptr)
#endif

#ifndef CPU_ALLOC_SIZE
#define CPU_ALLOC_SIZE(size) sizeof(cpu_set_t)
#endif

#ifndef CPU_ZERO_S
#define CPU_ZERO_S(size, mask) CPU_ZERO(mask)
#endif

#ifndef CPU_SET_S
#define CPU_SET_S(cpu, size, mask) CPU_SET(cpu, mask)
#endif

#endif /* LTP_SCHED_SETAFFINITY_H */
