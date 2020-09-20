// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * This test is based on source contained in the man pages for sendmmsg and
 * recvmmsg in release 4.15 of the Linux man-pages project.
 */

#define _GNU_SOURCE

#include "sendmmsg.h"

static struct tst_ts ts;

enum test_type {
	NORMAL,
	TIMEOUT,
};

struct test_case {
	int ttype;
	const char *desc;
	long tv_sec;
	long tv_nsec;
	int exp_ret;
};

static struct test_case tcase[] = {
	{
		TYPE_NAME(NORMAL),
		.tv_sec = 1,
		.tv_nsec = 0,
		.exp_ret = 2,
	},
	{
		TYPE_NAME(TIMEOUT),
		.tv_sec = 0,
		.tv_nsec = 0,
		.exp_ret = 1,
	},
};

static void do_test(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];
	struct tst_ts timeout;
	int retval;

	tst_res(TINFO, "case %s", tc->desc);

	TEST(tv->sendmmsg(send_sockfd, snd_msg, VLEN, 0));

	if (TST_RET < 0 || snd_msg[0].msg_len != 6 || snd_msg[1].msg_len != 6) {
		tst_res(TFAIL | TERRNO, "sendmmsg() failed");
		return;
	}

	memset(rcv1->iov_base, 0, rcv1->iov_len);
	memset(rcv2->iov_base, 0, rcv2->iov_len);

	timeout.type = tv->ts_type;
	tst_ts_set_sec(&timeout, tc->tv_sec);
	tst_ts_set_nsec(&timeout, tc->tv_nsec);

	TEST(tv->recvmmsg(receive_sockfd, rcv_msg, VLEN, 0, tst_ts_get(&timeout)));

	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "recvmmsg() failed");
		return;
	}
	if (tc->exp_ret != TST_RET) {
		tst_res(TFAIL, "Received unexpected number of messages (%d)",
			retval);
		return;
	}

	if (memcmp(rcv1->iov_base, "onetwo", 6))
		tst_res(TFAIL, "Error in first received message");
	else
		tst_res(TPASS, "First message received successfully");

	if (tc->ttype == NORMAL) {
		if (memcmp(rcv2->iov_base, "three", 5))
			tst_res(TFAIL, "Error in second received message");
		else
			tst_res(TPASS, "Second message received successfully");
	} else {
		TEST(tv->recvmmsg(receive_sockfd, rcv_msg, 1, 0, NULL));
		if (TST_RET != 1 || memcmp(rcv1->iov_base, "three", 5))
			tst_res(TFAIL, "Error in second message after the timeout");
		else
			tst_res(TPASS, "Timeout successfully reached before second message");
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
