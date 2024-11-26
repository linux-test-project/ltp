// SPDX-License-Identifier: GPL-2.0-or-later

/*\
 * Test recvmmsg() errors:
 *
 * - EBADF  Bad socket file descriptor
 * - EFAULT Bad message vector address
 * - EINVAL Bad seconds value for the timeout argument
 * - EINVAL Bad nanoseconds value for the timeout argument
 * - EFAULT Bad timeout address
 */

#define _GNU_SOURCE
#include "../sendmmsg/sendmmsg.h"

static int send_sockfd;
static int receive_sockfd;

#define VLEN 1

static struct mmsghdr *msg;
static struct iovec *iov;

static void *bad_addr;
static int bad_fd = -1;

static struct tst_ts ts;

struct test_case {
	const char *desc;
	int *fd;
	long tv_sec;
	long tv_nsec;
	int exp_errno;
	struct mmsghdr **msg_vec;
	int bad_ts_addr;
};

static struct test_case tcase[] = {
	{
		.desc = "bad socket file descriptor",
		.fd = &bad_fd,
		.exp_errno = EBADF,
		.msg_vec = &msg,
	},
	{
		.desc = "bad message vector address",
		.fd = &receive_sockfd,
		.exp_errno = EFAULT,
		.msg_vec = (void *)&bad_addr,
	},
	{
		.desc = "negative seconds in timeout",
		.fd = &receive_sockfd,
		.tv_sec = -1,
		.tv_nsec = 0,
		.exp_errno = EINVAL,
		.msg_vec = &msg,
	},
	{
		.desc = "overflow in nanoseconds in timeout",
		.fd = &receive_sockfd,
		.tv_sec = 1,
		.tv_nsec = 1000000001,
		.exp_errno = EINVAL,
		.msg_vec = &msg,
	},
	{
		.desc = "bad timeout address",
		.fd = &receive_sockfd,
		.exp_errno = EFAULT,
		.msg_vec = &msg,
		.bad_ts_addr = 1,
	}
};

static void verify_recvmmsg(unsigned int i, void *timeout)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_case *tc = &tcase[i];

	ts.type = tv->ts_type;
	tst_ts_set_sec(&ts, tc->tv_sec);
	tst_ts_set_nsec(&ts, tc->tv_nsec);

	TST_EXP_FAIL2(tv->recvmmsg(*tc->fd, *tc->msg_vec, VLEN, 0, timeout),
		      tc->exp_errno, "recvmmsg() %s", tc->desc);
}

static void test_bad_addr(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	void *timeout = bad_addr;
	pid_t pid;
	int status;

	pid = SAFE_FORK();
	if (!pid) {
		verify_recvmmsg(i, timeout);
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status) && !WEXITSTATUS(status))
		return;

	if (tv->ts_type == TST_LIBC_TIMESPEC &&
		WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "Child killed by expected signal");
		return;
	}

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];

	if (tc->bad_ts_addr)
		test_bad_addr(i);
	else
		verify_recvmmsg(i, tst_ts_get(&ts));
}

static void setup(void)
{
	struct sockaddr_in addr;
	unsigned int port = TST_GET_UNUSED_PORT(AF_INET, SOCK_DGRAM);
	struct time64_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);

	send_sockfd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
	receive_sockfd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = port;

	SAFE_BIND(receive_sockfd, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_CONNECT(send_sockfd, (struct sockaddr *)&addr, sizeof(addr));

	msg[0].msg_hdr.msg_iov = iov;
	msg[0].msg_hdr.msg_iovlen = 1;

	TEST(tv->sendmmsg(send_sockfd, msg, 1, 0));

	if (TST_RET != 1) {
		tst_res(TFAIL | TTERRNO, "sendmmsg() failed");
		return;
	}

	bad_addr = tst_get_bad_addr(NULL);
}

static void cleanup(void)
{
	if (send_sockfd > 0)
		SAFE_CLOSE(send_sockfd);

	if (receive_sockfd > 0)
		SAFE_CLOSE(receive_sockfd);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcase),
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = ARRAY_SIZE(variants),
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&iov, .iov_sizes = (int[]){1, -1}},
		{&msg, .size = VLEN * sizeof(*msg)},
		{},
	}
};
