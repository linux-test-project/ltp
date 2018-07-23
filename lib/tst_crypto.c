/*
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 *                    Nicolai Stange <nstange@suse.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdio.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_crypto.h"
#include "tst_netlink.h"

void tst_crypto_open(struct tst_crypto_session *ses)
{
	TEST(socket(AF_NETLINK, SOCK_DGRAM, NETLINK_CRYPTO));
	if (TST_RET < 0 && TST_ERR == EPROTONOSUPPORT)
		tst_brk(TCONF | TTERRNO, "NETLINK_CRYPTO is probably disabled");

	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"socket(AF_NETLINK, SOCK_DGRAM, NETLINK_CRYPTO)");
	}

	ses->fd = TST_RET;
	ses->seq_num = 0;
}

void tst_crypto_close(struct tst_crypto_session *ses)
{
	SAFE_CLOSE(ses->fd);
}

static int tst_crypto_recv_ack(struct tst_crypto_session *ses)
{
	uint32_t len;
	char buf[BUFSIZ];
	struct nlmsghdr *nh;

	len = SAFE_NETLINK_RECV(ses->fd, buf, sizeof(buf));

	for (nh = (struct nlmsghdr *) buf;
	     NLMSG_OK(nh, len);
	     nh = NLMSG_NEXT(nh, len)) {
		if (nh->nlmsg_seq != ses->seq_num) {
			tst_brk(TBROK,
				"Message out of sequence; type=0%hx, seq_num=%u (not %u)",
				nh->nlmsg_type, nh->nlmsg_seq, ses->seq_num);
		}

		/* Acks use the error message type with error number set to
		 * zero. Ofcourse we could also receive an actual error.
		 */
		if (nh->nlmsg_type == NLMSG_ERROR)
			return ((struct nlmsgerr *)NLMSG_DATA(nh))->error;

		tst_brk(TBROK, "Unexpected message type; type=0x%hx, seq_num=%u",
			nh->nlmsg_type, nh->nlmsg_seq);
	}

	tst_brk(TBROK, "Empty message from netlink socket?");

	return ENODATA;
}

int tst_crypto_add_alg(struct tst_crypto_session *ses,
		       const struct crypto_user_alg *alg)
{
	struct nlmsghdr nh = {
		.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(*alg),
		.nlmsg_type = CRYPTO_MSG_NEWALG,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.nlmsg_seq = ++(ses->seq_num),
		.nlmsg_pid = 0,
	};

	SAFE_NETLINK_SEND(ses->fd, &nh, alg);

	return tst_crypto_recv_ack(ses);
}

int tst_crypto_del_alg(struct tst_crypto_session *ses,
		       const struct crypto_user_alg *alg)
{
	unsigned int i = 0;
	struct nlmsghdr nh = {
		.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(*alg),
		.nlmsg_type = CRYPTO_MSG_DELALG,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.nlmsg_pid = 0,
	};

	while (1) {
		nh.nlmsg_seq = ++(ses->seq_num),

		SAFE_NETLINK_SEND(ses->fd, &nh, alg);

		TEST(tst_crypto_recv_ack(ses));
		if (TST_RET != -EBUSY || i >= ses->retries)
			break;

		if (usleep(1) && errno != EINTR)
			tst_brk(TBROK | TERRNO, "usleep(1)");

		++i;
	}

	return TST_RET;
}
