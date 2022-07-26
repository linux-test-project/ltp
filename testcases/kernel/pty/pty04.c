// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE
 *
 * Test transmitting data over a PTY/TTY line discipline and reading from the
 * virtual netdev created by the line discipline. Also hangup the PTY while
 * data is in flight to try to cause a race between the netdev being deleted
 * and the discipline receive function writing to the netdev.
 *
 * For SLCAN we check stack data is not leaked in the frame padding
 * (CVE-2020-11494).
 *
 * Test flow:
 * 1. Create PTY with ldisc X which creates netdev Y
 * 2. Open raw packet socket and bind to netdev Y
 * 3. Send data on ptmx and read packets from socket
 * 4. Hangup while transmission in progress
 *
 * Note that not all line disciplines call unthrottle when they are ready to
 * read more bytes. So it is possible to fill all the write buffers causing
 * write to block forever (because once write sleeps it needs unthrottle to
 * wake it). So we write with O_NONBLOCK.
 *
 * Also the max buffer size for PTYs is 8192, so even if the protocol MTU is
 * greater everything may still be processed in 8129 byte chunks. At least
 * until we are in the netdev code which can have a bigger buffer. Of course
 * the MTU still decides exactly where the packet delimiter goes, this just
 * concerns choosing the best packet size to cause a race.
 *
 * Note on line discipline encapsulation formats:
 * - For SLIP frames we just write the data followed by a delimiter char
 * - SLCAN we write some ASCII described in drivers/net/can/slcan.c which is
 *   converted to the actual frame by the kernel
 */

#define _GNU_SOURCE
#include "config.h"
#include "tst_test.h"
#include "tst_buffers.h"
#include "lapi/tty.h"

#if defined(HAVE_LINUX_IF_PACKET_H) && defined(HAVE_LINUX_IF_ETHER_H)

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/tty.h>

/*
 * define instead of including <linux/can.h> to support kernel headers
 * before change from v4.2-rc1
 * a2f11835994e ("can.h: make padding given by gcc explicit").
 */

#define CAN_MTU		(sizeof(struct can_frame))
#define CAN_MAX_DLEN 8

typedef uint32_t canid_t;

struct can_frame {
	canid_t can_id;
	uint8_t can_dlc;
	uint8_t __pad;
	uint8_t __res0;
	uint8_t __res1;
	uint8_t data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include "lapi/ioctl.h"

#include "tst_safe_stdio.h"

#define SLCAN_FRAME "t00185f5f5f5f5f5f5f5f\r"

struct ldisc_info {
	int n;
	char *name;
	int mtu;
};

static struct ldisc_info ldiscs[] = {
	{N_SLIP, "N_SLIP", 8192},
	{N_SLCAN, "N_SLCAN", CAN_MTU},
};

static int ptmx = -1, pts = -1, sk = -1, mtu, no_check;

static int set_ldisc(int tty, const struct ldisc_info *ldisc)
{
	TEST(ioctl(tty, TIOCSETD, &ldisc->n));

	if (!TST_RET)
		return 0;

	if (TST_ERR == EINVAL) {
		tst_res(TCONF | TTERRNO,
			"You don't appear to have the %s TTY line discipline",
			ldisc->name);
	} else {
		tst_res(TFAIL | TTERRNO,
			"Failed to set the %s line discipline", ldisc->name);
	}

	return 1;
}

static int open_pty(const struct ldisc_info *ldisc)
{
	char pts_path[PATH_MAX];

	ptmx = SAFE_OPEN("/dev/ptmx", O_RDWR);
	if (grantpt(ptmx))
		tst_brk(TBROK | TERRNO, "grantpt(ptmx)");
	if (unlockpt(ptmx))
		tst_brk(TBROK | TERRNO, "unlockpt(ptmx)");
	if (ptsname_r(ptmx, pts_path, sizeof(pts_path)))
		tst_brk(TBROK | TERRNO, "ptsname_r(ptmx, ...)");

	SAFE_FCNTL(ptmx, F_SETFL, O_NONBLOCK);

	tst_res(TINFO, "PTS path is %s", pts_path);
	pts = SAFE_OPEN(pts_path, O_RDWR);

	return set_ldisc(pts, ldisc);
}

static ssize_t try_async_write(int fd, const char *data, ssize_t size,
			       ssize_t *done)
{
	ssize_t off = done ? *done : 0;
	ssize_t ret = write(fd, data + off, size - off);

	if (ret < 0)
		return -(errno != EAGAIN);

	if (!done)
		return 1;

	*done += ret;
	return *done >= size;
}

static ssize_t try_async_read(int fd, char *data, ssize_t size,
			      ssize_t *done)
{
	ssize_t off = done ? *done : 0;
	ssize_t ret = read(fd, data + off, size - off);

	if (ret < 0)
		return -(errno != EAGAIN);

	if (!done)
		return 1;

	*done += ret;
	return *done >= size;
}

static ssize_t retry_async_write(int fd, const char *data, ssize_t size)
{
	ssize_t done = 0;

	return TST_RETRY_FUNC(try_async_write(fd, data, size, &done),
			      TST_RETVAL_NOTNULL);
}

static ssize_t retry_async_read(int fd, char *data, ssize_t size)
{
	ssize_t done = 0;

	return TST_RETRY_FUNC(try_async_read(fd, data, size, &done),
			      TST_RETVAL_NOTNULL);
}

static void do_pty(const struct ldisc_info *ldisc)
{
	char *data;
	ssize_t ret;
	size_t len = 0;

	switch (ldisc->n) {
	case N_SLIP:
		len = mtu;
		break;
	case N_SLCAN:
		len = sizeof(SLCAN_FRAME) - 1;
		break;
	}

	data = tst_alloc(len);

	switch (ldisc->n) {
	case N_SLIP:
		memset(data, '_', len - 1);
		data[len - 1] = 0300;
		break;
	case N_SLCAN:
		memcpy(data, SLCAN_FRAME, len);
		break;
	}

	ret = retry_async_write(ptmx, data, len);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "Failed 1st write to PTY");
	tst_res(TPASS, "Wrote PTY %s %d (1)", ldisc->name, ptmx);

	ret = retry_async_write(ptmx, data, len);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "Failed 2nd write to PTY");

	if (tcflush(ptmx, TCIFLUSH))
		tst_brk(TBROK | TERRNO, "tcflush(ptmx, TCIFLUSH)");

	tst_res(TPASS, "Wrote PTY %s %d (2)", ldisc->name, ptmx);

	ret = retry_async_read(ptmx, data, len);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "Failed read of PTY");

	tst_res(TPASS, "Read PTY %s %d", ldisc->name, ptmx);
	TST_CHECKPOINT_WAKE(0);

	while (1) {
		if (retry_async_read(ptmx, data, len) < 0)
			break;

		if (retry_async_write(ptmx, data, len) < 0)
			break;
	}

	tst_res(TPASS, "Transmission on PTY interrupted by hangup");

	tst_free_all();
}

static void open_netdev(const struct ldisc_info *ldisc)
{
	struct ifreq ifreq = { 0 };
	struct sockaddr_ll lla = { 0 };

	SAFE_IOCTL(pts, SIOCGIFNAME, ifreq.ifr_name);
	tst_res(TINFO, "Netdev is %s", ifreq.ifr_name);

	sk = SAFE_SOCKET(PF_PACKET, SOCK_RAW, 0);

	ifreq.ifr_mtu = ldisc->mtu;
	if (ioctl(sk, SIOCSIFMTU, &ifreq))
		tst_res(TWARN | TERRNO, "Failed to set netdev MTU to maximum");
	SAFE_IOCTL(sk, SIOCGIFMTU, &ifreq);
	mtu = ifreq.ifr_mtu;
	tst_res(TINFO, "Netdev MTU is %d (we set %d)", mtu, ldisc->mtu);

	SAFE_IOCTL(sk, SIOCGIFFLAGS, &ifreq);
	ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;
	SAFE_IOCTL(sk, SIOCSIFFLAGS, &ifreq);
	SAFE_IOCTL(sk, SIOCGIFFLAGS, &ifreq);

	if (!(ifreq.ifr_flags & IFF_UP))
		tst_brk(TBROK, "Netdev did not come up");

	SAFE_IOCTL(sk, SIOCGIFINDEX, &ifreq);

	lla.sll_family = PF_PACKET;
	lla.sll_protocol = htons(ETH_P_ALL);
	lla.sll_ifindex = ifreq.ifr_ifindex;
	SAFE_BIND(sk, (struct sockaddr *)&lla, sizeof(struct sockaddr_ll));

	tst_res(TINFO, "Bound netdev %d to socket %d", ifreq.ifr_ifindex, sk);
}

static void check_data(const struct ldisc_info *ldisc,
		       const char *data, ssize_t len)
{
	ssize_t i = 0, j;
	struct can_frame frm;

	if (no_check)
		return;

	if (ldisc->n == N_SLCAN) {
		memcpy(&frm, data, len);

		if (frm.can_id != 1) {
			tst_res(TFAIL, "can_id = %d != 1",
				frm.can_id);
			no_check = 1;
		}

		if (frm.can_dlc != CAN_MAX_DLEN) {
			tst_res(TFAIL, "can_dlc = %d != " TST_TO_STR_(CAN_MAX_DLEN),
				frm.can_dlc);
			no_check = 1;
		}

		i = offsetof(struct can_frame, __pad);
		if (frm.__pad != frm.__res0 || frm.__res0 != frm.__res1) {
			tst_res_hexd(TFAIL, data + i,
				     offsetof(struct can_frame, data) - i,
				     "Padding bytes may contain stack data");
			no_check = 1;
		}

		i = offsetof(struct can_frame, data);
	}

	do {
		if (i >= len)
			return;
	} while (data[i++] == '_');

	j = i--;

	while (j < len && j - i < 65 && data[j++] != '_')
		;
	j--;

	tst_res_hexd(TFAIL, data + i, j - i,
		     "Corrupt data (max 64 of %ld bytes shown): data[%ld..%ld] = ",
		     len, i, j);
	no_check = 1;

	if (no_check)
		tst_res(TINFO, "Will continue test without data checking");
}

static ssize_t try_sync_read(int fd, char *data, ssize_t size)
{
	ssize_t ret, n = 0;
	int retry = mtu;

	while (retry--) {
		ret = read(fd, data + n, size - n);

		if (ret < 0)
			return ret;

		if ((n += ret) >= size)
			return ret;
	}

	tst_brk(TBROK | TERRNO, "Only read %zd of %zd bytes", n, size);

	return n;
}

static ssize_t try_sync_write(int fd, const char *data, ssize_t size)
{
	ssize_t ret, n = 0;
	int retry = mtu;

	while (retry--) {
		ret = write(fd, data + n, size - n);

		if (ret < 0)
			return ret;

		if ((n += ret) >= size)
			return ret;
	}

	tst_brk(TBROK | TERRNO, "Only wrote %zd of %zd bytes", n, size);

	return n;
}

static void read_netdev(const struct ldisc_info *ldisc)
{
	int rlen, plen = 0;
	char *data;

	switch (ldisc->n) {
	case N_SLIP:
		plen = mtu - 1;
		break;
	case N_SLCAN:
		plen = CAN_MTU;
		break;
	}
	data = tst_alloc(plen);

	tst_res(TINFO, "Reading from socket %d", sk);

	TEST(try_sync_read(sk, data, plen));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "Read netdev %s %d (1)", ldisc->name, sk);
	check_data(ldisc, data, plen);
	tst_res(TPASS, "Read netdev %s %d (1)", ldisc->name, sk);

	TEST(try_sync_read(sk, data, plen));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "Read netdev %s %d (2)", ldisc->name, sk);
	check_data(ldisc, data, plen);
	tst_res(TPASS, "Read netdev %s %d (2)", ldisc->name, sk);

	TEST(try_sync_write(sk, data, plen));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "Write netdev %s %d", ldisc->name, sk);

	tst_res(TPASS, "Write netdev %s %d", ldisc->name, sk);

	while (1) {
		if (try_sync_write(sk, data, plen) < 0)
			break;

		if ((rlen = try_sync_read(sk, data, plen)) < 0)
			break;
		check_data(ldisc, data, rlen);
	}

	tst_res(TPASS, "Data transmission on netdev interrupted by hangup");

	close(sk);
	tst_free_all();
}

static void do_test(unsigned int n)
{
	struct ldisc_info *ldisc = &ldiscs[n];

	if (open_pty(ldisc))
		return;

	open_netdev(ldisc);

	if (!SAFE_FORK()) {
		read_netdev(ldisc);
		return;
	}

	if (!SAFE_FORK()) {
		do_pty(ldisc);
		return;
	}

	if (!SAFE_FORK()) {
		TST_CHECKPOINT_WAIT2(0, 100000);
		SAFE_IOCTL(pts, TIOCVHANGUP);
		tst_res(TINFO, "Sent hangup ioctl to PTS");
		SAFE_IOCTL(ptmx, TIOCVHANGUP);
		tst_res(TINFO, "Sent hangup ioctl to PTM");
		return;
	}

	tst_reap_children();
}

static void cleanup(void)
{
	if (pts >= 0)
		ioctl(pts, TIOCVHANGUP);

	if (ptmx >= 0)
		ioctl(ptmx, TIOCVHANGUP);

	if (sk >= 0)
		close(sk);

	tst_reap_children();
}

static struct tst_test test = {
	.test = do_test,
	.cleanup = cleanup,
	.tcnt = 2,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_root = 1,
	.tags = (const struct tst_tag[]){
		{"linux-git", "b9258a2cece4ec1f020715fe3554bc2e360f6264"},
		{"CVE", "CVE-2020-11494"},
		{}
	}
};

#else

TST_TEST_TCONF("Need <linux/if_packet.h> and <linux/if_ether.h>");

#endif
