// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 */
/* Test for CVE-2017-7308 on a raw socket's ring buffer
 *
 * Try to set tpacket_req3.tp_sizeof_priv to a value with the high bit set. So
 * that tp_block_size < tp_sizeof_priv. If the vulnerability is present then
 * this will cause an integer arithmetic overflow and the absurd
 * tp_sizeof_priv value will be allowed. If it has been fixed then setsockopt
 * will fail with EINVAL.
 *
 * We also try a good configuration to make sure it is not failing with EINVAL
 * for some other reason.
 *
 * For a better and more interesting discussion of this CVE see:
 * https://googleprojectzero.blogspot.com/2017/05/exploiting-linux-kernel-via-packet.html
 */

#include <errno.h>
#include "tst_test.h"
#include "tst_safe_net.h"
#include "lapi/if_packet.h"
#include "lapi/if_ether.h"

static int sk;
static long pgsz;

static void setup(void)
{
	pgsz = SAFE_SYSCONF(_SC_PAGESIZE);
}

static void cleanup(void)
{
	if (sk > 0)
		SAFE_CLOSE(sk);
}

static int create_skbuf(unsigned int sizeof_priv)
{
	int ver = TPACKET_V3;
	struct tpacket_req3 req = {};

	req.tp_block_size = pgsz;
	req.tp_block_nr = 2;
	req.tp_frame_size = req.tp_block_size;
	req.tp_frame_nr = req.tp_block_nr;
	req.tp_retire_blk_tov = 100;

	req.tp_sizeof_priv = sizeof_priv;

	sk = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	TEST(setsockopt(sk, SOL_PACKET, PACKET_VERSION, &ver, sizeof(ver)));
	if (TST_RET && TST_ERR == EINVAL)
		tst_brk(TCONF | TTERRNO, "TPACKET_V3 not supported");
	if (TST_RET)
		tst_brk(TBROK | TTERRNO, "setsockopt(sk, SOL_PACKET, PACKET_VERSION, TPACKET_V3)");

	return setsockopt(sk, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req));
}

static void good_size(void)
{
	TEST(create_skbuf(512));
	if (TST_RET)
		tst_brk(TBROK | TTERRNO, "Can't create ring buffer with good settings");

	tst_res(TPASS, "Can create ring buffer with good settinegs");
}

static void bad_size(void)
{
	TEST(create_skbuf(3U << 30));
	if (TST_RET && TST_ERR != EINVAL)
		tst_brk(TBROK | TTERRNO, "Unexpected setsockopt() error");
	if (TST_RET)
		tst_res(TPASS | TTERRNO, "Refused bad tp_sizeof_priv value");
	else
		tst_res(TFAIL, "Allowed bad tp_sizeof_priv value");
}

static void run(unsigned int i)
{
	if (i == 0)
		good_size();
	else
		bad_size();

	SAFE_CLOSE(sk);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
};
