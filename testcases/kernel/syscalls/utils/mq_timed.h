// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef MQ_TIMED_H
#define MQ_TIMED_H

#include "mq.h"
#include "time64_variants.h"
#include "tst_timer.h"

static struct time64_variants variants[] = {
	{ .clock_gettime = libc_clock_gettime, .mqt_send = libc_mq_timedsend, .mqt_receive = libc_mq_timedreceive, .ts_type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_mq_timedsend != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime, .mqt_send = sys_mq_timedsend, .mqt_receive = sys_mq_timedreceive, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_mq_timedsend_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime64, .mqt_send = sys_mq_timedsend64, .mqt_receive = sys_mq_timedreceive64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

struct test_case {
	int *fd;
	unsigned int len;
	unsigned int prio;
	struct tst_ts *rq;
	long tv_sec;
	long tv_nsec;
	int invalid_msg;
	int send;
	int signal;
	int timeout;
	int bad_msg_addr;
	int bad_ts_addr;
	int ret;
	int err;
};

static pid_t set_sig(struct tst_ts *ts,
		     int (*gettime)(clockid_t clk_id, void *ts))
{
	gettime(CLOCK_REALTIME, tst_ts_get(ts));
	*ts = tst_ts_add_us(*ts, 3000000);

	return create_sig_proc(SIGINT, 40, 50000);
}

static void set_timeout(struct tst_ts *ts,
			int (*gettime)(clockid_t clk_id, void *ts))
{
	gettime(CLOCK_REALTIME, tst_ts_get(ts));
	*ts = tst_ts_add_us(*ts, 10000);
}

static void kill_pid(pid_t pid)
{
	SAFE_KILL(pid, SIGTERM);
	SAFE_WAIT(NULL);
}

#endif /* MQ_TIMED_H */
