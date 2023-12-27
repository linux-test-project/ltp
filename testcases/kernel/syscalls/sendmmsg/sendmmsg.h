/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef SENDMMSG_H__
#define SENDMMSG_H__

#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "time64_variants.h"
#include "tst_test.h"
#include "lapi/socket.h"
#include "tst_safe_macros.h"
#include "sendmmsg_var.h"

#define BUFSIZE 16

static struct time64_variants variants[] = {
	{ .recvmmsg = libc_recvmmsg, .sendmmsg = libc_sendmmsg, .ts_type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_recvmmsg != __LTP__NR_INVALID_SYSCALL)
	{ .recvmmsg = sys_recvmmsg, .sendmmsg = sys_sendmmsg, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_recvmmsg_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .recvmmsg = sys_recvmmsg64, .sendmmsg = sys_sendmmsg, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

#endif /* SENDMMSG_H__ */
