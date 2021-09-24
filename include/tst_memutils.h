/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

#ifndef TST_MEMUTILS_H__
#define TST_MEMUTILS_H__

/*
 * Fill up to maxsize physical memory with fillchar, then free it for reuse.
 * If maxsize is zero, fill as much memory as possible. This function is
 * intended for data disclosure vulnerability tests to reduce the probability
 * that a vulnerable kernel will leak a block of memory that was full of
 * zeroes by chance.
 *
 * The function keeps a safety margin to avoid invoking OOM killer and
 * respects the limitations of available address space. (Less than 3GB can be
 * polluted on a 32bit system regardless of available physical RAM.)
 */
void tst_pollute_memory(size_t maxsize, int fillchar);

/*
 * Read the value of MemAvailable from /proc/meminfo, if no support on
 * older kernels, return 'MemFree + Cached' for instead.
 */
long long tst_available_mem(void);

#endif /* TST_MEMUTILS_H__ */
