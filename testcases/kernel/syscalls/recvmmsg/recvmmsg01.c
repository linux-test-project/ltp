// SPDX-License-Identifier: GPL-2.0-or-later

/* test status of errors:
 *
 * EBADF            v ('Bad socket file descriptor')
 * EFAULT           v ('Bad message vector address')
 * EINVAL           v ('Bad seconds value for the timeout argument')
 * EINVAL           v ('Bad nanoseconds value for the timeout argument')
 * EFAULT           v ('Bad timeout address')
 */

#define _GNU_SOURCE
#include "../sendmmsg/sendmmsg.h"

static struct tst_ts ts;

enum test_type {
	BAD_FD,
	BAD_MSGVEC,
	BAD_TS_VALUE_1,
	BAD_TS_VALUE_2,
	BAD_TS_ADDR,
};

#define TYPE_NAME(x) .ttype = x, .desc = #x

struct test_case {
	int ttype;
	const char *desc;
	int fd;
	long tv_sec;
	long tv_nsec;
	int exp_errno;
};

static struct test_case tcase[] = {
	{
		TYPE_NAME(BAD_FD),
		.fd = -1,
		.exp_errno = EBADF,
	},
	{
		TYPE_NAME(BAD_MSGVEC),
		.exp_errno = EFAULT,
	},
	{
		TYPE_NAME(BAD_TS_VALUE_1),
		.tv_sec = -1,
		.tv_nsec = 0,
		.exp_errno = EINVAL,
	},
	{
		TYPE_NAME(BAD_TS_VALUE_2),
		.tv_sec = 1,
		.tv_nsec = 1000000001,
		.exp_errno = EINVAL,
	},
	{
		TYPE_NAME(BAD_TS_ADDR),
		.exp_errno = EFAULT,
	}
};

static void do_test(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];
	void *rcv_msgvec, *timeout;

	tst_res(TINFO, "case %s", tc->desc);

	if (tc->ttype != BAD_FD)
		tc->fd = receive_sockfd;

	TEST(tv->sendmmsg(send_sockfd, snd_msg, VLEN, 0));

	if (TST_RET != VLEN || snd_msg[0].msg_len != 6 ||
	    snd_msg[1].msg_len != 6) {
		tst_res(TFAIL | TERRNO, "sendmmsg() failed");
		return;
	}

	memset(rcv1->iov_base, 0, rcv1->iov_len);
	memset(rcv2->iov_base, 0, rcv2->iov_len);

	timeout.type = tv->ts_type;
	tst_ts_set_sec(&ts, tc->tv_sec);
	tst_ts_set_nsec(&ts, tc->tv_nsec);

	if (tc->ttype == BAD_MSGVEC)
		rcv_msgvec = bad_addr;
	else
		rcv_msgvec = rcv_msg;

	if (tc->ttype == BAD_TS_ADDR)
		timeout = bad_addr;
	else
		timeout = tst_ts_get(&ts);

	TEST(tv->recvmmsg(tc->fd, rcv_msgvec, VLEN, 0, timeout));

	if (TST_RET < 0) {
		if (tc->exp_errno == errno)
			tst_res(TPASS | TERRNO, "receivemmsg() failed successfully");
		else
			tst_res(TFAIL | TERRNO, "receivemmsg() failed unexpectedly");
	} else {
		tst_res(TFAIL | TERRNO, "receivemmsg() succeded unexpectedly");
	}
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcase),
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = ARRAY_SIZE(variants),
	.bufs = (struct tst_buffers []) {
		{&snd1, .iov_sizes = (int[]){3, 3, -1}},
		{&snd2, .iov_sizes = (int[]){6, -1}},
		{&rcv1, .iov_sizes = (int[]){6, -1}},
		{&rcv2, .iov_sizes = (int[]){5, -1}},
		{&snd_msg, .size = VLEN * sizeof(*snd_msg)},
		{&rcv_msg, .size = VLEN * sizeof(*rcv_msg)},
		{},
	}
};
