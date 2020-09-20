// SPDX-License-Identifier: GPL-2.0-or-later

/* test status of errors:
 *
 * EBADF            v ('Bad socket file descriptor')
 * EFAULT           v ('Bad message vector address')
 */

#define _GNU_SOURCE
#include "sendmmsg.h"

enum test_type {
	BAD_FD,
	BAD_MSGVEC,
};

#define TYPE_NAME(x) .ttype = x, .desc = #x

struct test_case {
	int ttype;
	const char *desc;
	int fd;
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
};

static void do_test(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];
	void *snd_msgvec;

	tst_res(TINFO, "case %s", tc->desc);

	if (tc->ttype != BAD_FD)
		tc->fd = send_sockfd;

	if (tc->ttype == BAD_MSGVEC)
		snd_msgvec = bad_addr;
	else
		snd_msgvec = snd_msg;

	TEST(tv->sendmmsg(tc->fd, snd_msgvec, VLEN, 0));

	if (TST_RET < 0)
		if (tc->exp_errno != TST_ERR)
			tst_res(TFAIL | TERRNO, "sendmmsg() failed unexpectedly");
		else
			tst_res(TPASS | TERRNO, "sendmmg() failed successfully");
	else
		tst_res(TFAIL, "sendmmsg() succeded unexpectedly");
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
