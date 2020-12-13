// SPDX-License-Identifier: GPL-2.0-or-later

/*\
 * Tests sendmmsg() failures:
 *
 * - EBADF Bad socket file descriptor
 * - EFAULT Bad message vector address
\*/

#define _GNU_SOURCE
#include "sendmmsg.h"

#define VLEN 1

static int send_sockfd;
static struct mmsghdr *snd_msg;

static void *bad_addr;
static int bad_fd = -1;

struct test_case {
	const char *desc;
	int *fd;
	int exp_errno;
	struct mmsghdr **msg_vec;
};

static struct test_case tcase[] = {
	{
		.desc = "bad file descriptor",
		.fd = &bad_fd,
		.msg_vec = &snd_msg,
		.exp_errno = EBADF,
	},
	{
		.desc = "invalid msgvec address",
		.fd = &send_sockfd,
		.msg_vec = (void*)&bad_addr,
		.exp_errno = EFAULT,
	},
};

static void do_test(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];

	TST_EXP_FAIL(tv->sendmmsg(*tc->fd, *tc->msg_vec, VLEN, 0),
	             tc->exp_errno, "sendmmsg() %s", tc->desc);
}

static void setup(void)
{
	send_sockfd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);

	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
}

static void cleanup(void)
{
	if (send_sockfd > 0)
		SAFE_CLOSE(send_sockfd);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcase),
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = ARRAY_SIZE(variants),
	.bufs = (struct tst_buffers []) {
		{&snd_msg, .size = VLEN * sizeof(*snd_msg)},
		{},
	}
};
