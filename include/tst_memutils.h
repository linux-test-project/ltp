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

/*
 * Read the value of SwapFree from /proc/meminfo.
 */
long long tst_available_swap(void);

/*
 * Enable OOM protection to prevent process($PID) being killed by OOM Killer.
 *   echo -1000 >/proc/$PID/oom_score_adj
 *
 * If the pid is 0 which means it will set on current(self) process.
 *
 * Unless the process has CAP_SYS_RESOURCE this call will be no-op because
 * setting adj value < 0 requires it.
 *
 * CAP_SYS_RESOURCE:
 *   set /proc/[pid]/oom_score_adj to a value lower than the value last set
 *   by a process with CAP_SYS_RESOURCE.
 *
 * Note:
 *  This exported tst_enable_oom_protection function can be used at anywhere
 *  you want to protect, but please remember that if you do enable protection
 *  on a process($PID) that all the children will inherit its score and be
 *  ignored by OOM Killer as well. So that's why tst_disable_oom_protection()
 *  to be used in combination.
 */
void tst_enable_oom_protection(pid_t pid);

/*
 * Disable the OOM protection for the process($PID).
 *   echo 0 >/proc/$PID/oom_score_adj
 */
void tst_disable_oom_protection(pid_t pid);

#define TST_PRINT_MEMINFO() safe_print_file(__FILE__, __LINE__, "/proc/meminfo")

#endif /* TST_MEMUTILS_H__ */
