/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

#ifndef TST_MEMUTILS_H__
#define TST_MEMUTILS_H__

/**
 * tst_pollute_memory() - Fills physical memory with a byte pattern.
 *
 * @maxsize: Maximum memory size in bytes to fill (0 for maximum possible).
 * @fillchar: Byte value to write into allocated memory.
 *
 * Fills up to maxsize physical memory with fillchar, then frees it for reuse.
 * Keeps a safety margin to avoid invoking the OOM killer and respects address
 * space limits.
 */
void tst_pollute_memory(size_t maxsize, int fillchar);

/**
 * tst_available_mem() - Reads available memory from /proc/meminfo.
 *
 * Reads MemAvailable from /proc/meminfo. On older kernels without MemAvailable,
 * falls back to MemFree + Cached.
 *
 * Return: Available memory in KiB.
 */
long long tst_available_mem(void);

/**
 * tst_available_swap() - Reads free swap from /proc/meminfo using SwapFree.
 *
 * Return: Available swap space in KiB.
 */
long long tst_available_swap(void);

/**
 * tst_enable_oom_protection() - Protects process from OOM killer.
 *
 * @pid: Process PID to protect, or 0 for the calling process.
 *
 * Sets /proc/[pid]/oom_score_adj to -1000. Requires CAP_SYS_RESOURCE; no-op
 * without this capability. Child processes inherit the OOM score.
 */
void tst_enable_oom_protection(pid_t pid);

/**
 * tst_disable_oom_protection() - Disables OOM protection for process.
 *
 * @pid: Process PID, or 0 for the calling process.
 *
 * Sets /proc/[pid]/oom_score_adj to 0.
 */
void tst_disable_oom_protection(pid_t pid);

#define TST_PRINT_MEMINFO() safe_print_file(__FILE__, __LINE__, "/proc/meminfo")

/**
 * tst_mapping_in_range() - Returns true if there is a mapping provided range.
 *
 * @low: A lower address inside of the process address space.
 * @high: A higher address inside of the process address space.
 *
 * Return: Returns true if there is a mapping between low and high addresses in
 * the process address space.
 */
int tst_mapping_in_range(unsigned long low, unsigned long high);

#endif /* TST_MEMUTILS_H__ */
