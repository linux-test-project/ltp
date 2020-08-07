// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef MQ_TIMED_H
#define MQ_TIMED_H

#include "mq.h"
#include "tst_timer.h"

static struct test_variants {
	int (*send)(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
		    unsigned int msg_prio, void *abs_timeout);
	ssize_t (*receive)(mqd_t mqdes, char *msg_ptr, size_t msg_len,
			   unsigned int *msg_prio, void *abs_timeout);

	int (*gettime)(clockid_t clk_id, void *ts);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
	{ .gettime = libc_clock_gettime, .send = libc_mq_timedsend, .receive = libc_mq_timedreceive, .type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_mq_timedsend != __LTP__NR_INVALID_SYSCALL)
	{ .gettime = sys_clock_gettime, .send = sys_mq_timedsend, .receive = sys_mq_timedreceive, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_mq_timedsend_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .gettime = sys_clock_gettime64, .send = sys_mq_timedsend64, .receive = sys_mq_timedreceive64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
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
