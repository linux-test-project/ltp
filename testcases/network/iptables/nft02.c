// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC
 * Author: Marcos Paulo de Souza <mpdesouza@suse.com>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * CVE-2023-31248
 *
 * Test for use-after-free when adding a new rule to a chain deleted
 * in the same netlink message batch.
 *
 * Kernel bug fixed in:
 *
 *  commit 515ad530795c118f012539ed76d02bacfd426d89
 *  Author: Thadeu Lima de Souza Cascardo <cascardo@canonical.com>
 *  Date:   Wed Jul 5 09:12:55 2023 -0300
 *
 *  netfilter: nf_tables: do not ignore genmask when looking up chain by id
 */

#include <linux/netlink.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>
#include "lapi/nf_tables.h"
#include <linux/netfilter/nfnetlink.h>
#include "tst_test.h"
#include "tst_netlink.h"

#define TABNAME "ltp_table1"
#define SRCCHAIN "ltp_chain_src"
#define DESTCHAIN "ltp_chain_dest"
#define DESTCHAIN_ID 77

static uint32_t chain_id;
static uint32_t imm_dreg, imm_verdict;
static struct tst_netlink_context *ctx;

/* Table creation config */
static const struct tst_netlink_attr_list table_config[] = {
	{NFTA_TABLE_NAME, TABNAME, strlen(TABNAME) + 1, NULL},
	{0, NULL, -1, NULL}
};

/* Chain creation and deletion config */
static const struct tst_netlink_attr_list destchain_config[] = {
	{NFTA_TABLE_NAME, TABNAME, strlen(TABNAME) + 1, NULL},
	{NFTA_CHAIN_NAME, DESTCHAIN, strlen(DESTCHAIN) + 1, NULL},
	{NFTA_CHAIN_ID, &chain_id, sizeof(chain_id), NULL},
	{0, NULL, -1, NULL}
};

static const struct tst_netlink_attr_list delchain_config[] = {
	{NFTA_TABLE_NAME, TABNAME, strlen(TABNAME) + 1, NULL},
	{NFTA_CHAIN_NAME, DESTCHAIN, strlen(DESTCHAIN) + 1, NULL},
	{0, NULL, -1, NULL}
};

static const struct tst_netlink_attr_list srcchain_config[] = {
	{NFTA_TABLE_NAME, TABNAME, strlen(TABNAME) + 1, NULL},
	{NFTA_CHAIN_NAME, SRCCHAIN, strlen(SRCCHAIN) + 1, NULL},
	{0, NULL, -1, NULL}
};

/* Rule creation config */
static const struct tst_netlink_attr_list rule_verdict_config[] = {
	{NFTA_VERDICT_CODE, &imm_verdict, sizeof(imm_verdict), NULL},
	{NFTA_VERDICT_CHAIN_ID, &chain_id, sizeof(chain_id), NULL},
	{0, NULL, -1, NULL}
};

static const struct tst_netlink_attr_list rule_data_config[] = {
	{NFTA_IMMEDIATE_DREG, &imm_dreg, sizeof(imm_dreg), NULL},
	{NFTA_IMMEDIATE_DATA, NULL, 0, (const struct tst_netlink_attr_list[]) {
		{NFTA_DATA_VERDICT, NULL, 0, rule_verdict_config},
		{0, NULL, -1, NULL}
	}},
	{0, NULL, -1, NULL}
};

static const struct tst_netlink_attr_list rule_expr_config[] = {
	{NFTA_LIST_ELEM, NULL, 0, (const struct tst_netlink_attr_list[]) {
		{NFTA_EXPR_NAME, "immediate", 10, NULL},
		{NFTA_EXPR_DATA, NULL, 0, rule_data_config},
		{0, NULL, -1, NULL}
	}},
	{0, NULL, -1, NULL}
};

static const struct tst_netlink_attr_list rule_config[] = {
	{NFTA_RULE_EXPRESSIONS, NULL, 0, rule_expr_config},
	{NFTA_RULE_TABLE, TABNAME, strlen(TABNAME) + 1, NULL},
	{NFTA_RULE_CHAIN, SRCCHAIN, strlen(SRCCHAIN) + 1, NULL},
	{0, NULL, -1, NULL}
};

static void setup(void)
{
	tst_setup_netns();

	chain_id = htonl(DESTCHAIN_ID);
	imm_dreg = htonl(NFT_REG_VERDICT);
	imm_verdict = htonl(NFT_GOTO);
}

static void run(void)
{
	int ret;
	struct nlmsghdr header;
	struct nfgenmsg nfpayload;

	memset(&header, 0, sizeof(header));
	memset(&nfpayload, 0, sizeof(nfpayload));
	nfpayload.version = NFNETLINK_V0;

	ctx = NETLINK_CREATE_CONTEXT(NETLINK_NETFILTER);

	/* Start netfilter batch */
	header.nlmsg_type = NFNL_MSG_BATCH_BEGIN;
	header.nlmsg_flags = NLM_F_REQUEST;
	nfpayload.nfgen_family = AF_UNSPEC;
	nfpayload.res_id = htons(NFNL_SUBSYS_NFTABLES);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));

	/* Add table */
	header.nlmsg_type = (NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_NEWTABLE;
	header.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	nfpayload.nfgen_family = NFPROTO_IPV4;
	nfpayload.res_id = htons(0);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));
	NETLINK_ADD_ATTR_LIST(ctx, table_config);

	/* Add destination chain */
	header.nlmsg_type = (NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_NEWCHAIN;
	header.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	nfpayload.nfgen_family = NFPROTO_IPV4;
	nfpayload.res_id = htons(0);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));
	NETLINK_ADD_ATTR_LIST(ctx, destchain_config);

	/* Delete destination chain */
	header.nlmsg_type = (NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_DELCHAIN;
	header.nlmsg_flags = NLM_F_REQUEST;
	nfpayload.nfgen_family = NFPROTO_IPV4;
	nfpayload.res_id = htons(0);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));
	NETLINK_ADD_ATTR_LIST(ctx, delchain_config);

	/* Add source chain */
	header.nlmsg_type = (NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_NEWCHAIN;
	header.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	nfpayload.nfgen_family = NFPROTO_IPV4;
	nfpayload.res_id = htons(0);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));
	NETLINK_ADD_ATTR_LIST(ctx, srcchain_config);

	/* Add rule to source chain. Require ACK and check for ENOENT error. */
	header.nlmsg_type = (NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_NEWRULE;
	header.nlmsg_flags = NLM_F_REQUEST | NLM_F_APPEND | NLM_F_CREATE |
		NLM_F_ACK;
	nfpayload.nfgen_family = NFPROTO_IPV4;
	nfpayload.res_id = htons(0);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));
	NETLINK_ADD_ATTR_LIST(ctx, rule_config);

	/* End batch */
	header.nlmsg_type = NFNL_MSG_BATCH_END;
	header.nlmsg_flags = NLM_F_REQUEST;
	nfpayload.nfgen_family = AF_UNSPEC;
	nfpayload.res_id = htons(NFNL_SUBSYS_NFTABLES);
	NETLINK_ADD_MESSAGE(ctx, &header, &nfpayload, sizeof(nfpayload));

	ret = NETLINK_SEND_VALIDATE(ctx);
	TST_ERR = tst_netlink_errno;
	NETLINK_DESTROY_CONTEXT(ctx);
	ctx = NULL;

	if (ret)
		tst_res(TFAIL, "Netfilter chain list is corrupted");
	else if (TST_ERR == ENOENT)
		tst_res(TPASS, "Deleted netfilter chain cannot be referenced");
	else if (TST_ERR == EOPNOTSUPP || TST_ERR == EINVAL)
		tst_brk(TCONF, "Test requires unavailable netfilter features");
	else
		tst_brk(TBROK | TTERRNO, "Unknown nfnetlink error");
}

static void cleanup(void)
{
	NETLINK_DESTROY_CONTEXT(ctx);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NF_TABLES",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "515ad530795c"},
		{"CVE", "2023-31248"},
		{}
	}
};
