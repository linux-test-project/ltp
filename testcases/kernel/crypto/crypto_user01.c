// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018 Google LLC
 */

/*
 * Regression test for commit f43f39958beb ("crypto: user - fix leaking
 * uninitialized memory to userspace"), or CVE-2018-19854; it was also a
 * re-introduction of CVE-2013-2547.  This bug caused uninitialized kernel stack
 * memory to be leaked in some string fields in the replies to CRYPTO_MSG_GETALG
 * messages over NETLINK_CRYPTO.  To try to detect the bug, this test dumps all
 * algorithms using NLM_F_DUMP mode and checks all string fields for unexpected
 * nonzero bytes.
 */

#include <stdlib.h>

#include "tst_test.h"
#include "tst_crypto.h"

/*
 * include after <sys/socket.h> (via tst_test.h), to work around dependency bug
 * in old kernel headers (https://www.spinics.net/lists/netdev/msg171764.html)
 */
#include <linux/rtnetlink.h>

static struct tst_netlink_context *ctx;

static void setup(void)
{
	ctx = NETLINK_CREATE_CONTEXT(NETLINK_CRYPTO);
}

static void do_check_for_leaks(const char *name, const char *value, size_t vlen)
{
	size_t i;

	for (i = strnlen(value, vlen); i < vlen; i++) {
		if (value[i] != '\0')
			tst_brk(TFAIL, "information leak in field '%s'", name);
	}
}

#define check_for_leaks(name, field)  \
	do_check_for_leaks(name, field, sizeof(field))

static void validate_attr(const struct rtattr *rta)
{
	switch (rta->rta_type) {
	case CRYPTOCFGA_REPORT_LARVAL: {
		const struct crypto_report_larval *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_larval::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_HASH: {
		const struct crypto_report_hash *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_hash::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_BLKCIPHER: {
		const struct crypto_report_blkcipher *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_blkcipher::type", p->type);
		check_for_leaks("crypto_report_blkcipher::geniv", p->geniv);
		break;
	}
	case CRYPTOCFGA_REPORT_AEAD: {
		const struct crypto_report_aead *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_aead::type", p->type);
		check_for_leaks("crypto_report_aead::geniv", p->geniv);
		break;
	}
	case CRYPTOCFGA_REPORT_COMPRESS: {
		const struct crypto_report_comp *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_comp::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_RNG: {
		const struct crypto_report_rng *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_rng::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_CIPHER: {
		const struct crypto_report_cipher *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_cipher::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_AKCIPHER: {
		const struct crypto_report_akcipher *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_akcipher::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_KPP: {
		const struct crypto_report_kpp *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_kpp::type", p->type);
		break;
	}
	case CRYPTOCFGA_REPORT_ACOMP: {
		const struct crypto_report_acomp *p = RTA_DATA(rta);

		check_for_leaks("crypto_report_acomp::type", p->type);
		break;
	}
	} /* end switch */
}

static void validate_one_alg(const struct nlmsghdr *nh)
{
	const struct crypto_user_alg *alg = NLMSG_DATA(nh);
	const struct rtattr *rta = (void *)alg + NLMSG_ALIGN(sizeof(*alg));
	size_t remaining = NLMSG_PAYLOAD(nh, sizeof(*alg));

	check_for_leaks("crypto_user_alg::cru_name", alg->cru_name);
	check_for_leaks("crypto_user_alg::cru_driver_name",
			alg->cru_driver_name);
	check_for_leaks("crypto_user_alg::cru_module_name",
			alg->cru_module_name);

	while (RTA_OK(rta, remaining)) {
		validate_attr(rta);
		rta = RTA_NEXT(rta, remaining);
	}
}

static void validate_alg_list(const struct tst_netlink_message *msg)
{
	for (; msg->header; msg++) {
		if (msg->header->nlmsg_type == NLMSG_DONE)
			return;

		if (msg->header->nlmsg_type != CRYPTO_MSG_GETALG) {
			tst_brk(TBROK,
				"Unexpected message type; type=0x%hx, seq_num=%u",
				msg->header->nlmsg_type,
				msg->header->nlmsg_seq);
		}

		validate_one_alg(msg->header);
	}
}

static void run(void)
{
	struct crypto_user_alg payload = { 0 };
	struct nlmsghdr nh = {
		.nlmsg_type = CRYPTO_MSG_GETALG,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
	};
	struct tst_netlink_message *msg;

	NETLINK_ADD_MESSAGE(ctx, &nh, &payload, sizeof(payload));
	NETLINK_SEND(ctx);
	NETLINK_WAIT(ctx);
	msg = NETLINK_RECV(ctx);
	validate_alg_list(msg);
	NETLINK_FREE_MESSAGE(msg);
	tst_res(TPASS, "No information leaks found");
}

static void cleanup(void)
{
	NETLINK_DESTROY_CONTEXT(ctx);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f43f39958beb"},
		{"CVE", "2013-2547"},
		{"CVE", "2018-19854"},
		{}
	}
};
