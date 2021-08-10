// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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

#ifndef LAPI_CPUSET_H__
#define LAPI_CPUSET_H__

#ifndef CPU_ALLOC
#define CPU_ALLOC(ncpus) malloc(sizeof(cpu_set_t)); \
if (ncpus > CPU_SETSIZE) { \
	tst_brk(TCONF, \
		"Your libc does not support masks with %ld cpus", (long)ncpus); \
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

#ifndef CPU_ISSET_S
#define CPU_ISSET_S(cpu, size, mask) CPU_ISSET(cpu, mask)
#endif

#endif /* LAPI_CPUSET_H__ */
