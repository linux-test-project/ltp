// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC
 * Author: Marcos Paulo de Souza <mpdesouza@suse.com>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * CVE-2023-1829
 *
 * Test for use-after-free after removing tcindex traffic filter with certain
 * parameters.
 *
 * Tcindex filter removed in:
 *
 *  commit 8c710f75256bb3cf05ac7b1672c82b92c43f3d28
 *  Author: Jamal Hadi Salim <jhs@mojatatu.com>
 *  Date:   Tue Feb 14 08:49:14 2023 -0500
 *
 *  net/sched: Retire tcindex classifier
 */

#include <linux/netlink.h>
#include <linux/pkt_sched.h>
#include <linux/pkt_cls.h>
#include "tst_test.h"
#include "tst_netlink.h"
#include "tst_netdevice.h"
#include "lapi/sched.h"
#include "lapi/if_ether.h"
#include "lapi/rtnetlink.h"

#define DEVNAME "ltp_dummy1"

#ifndef TCA_TCINDEX_MAX
enum {
       TCA_TCINDEX_UNSPEC,
       TCA_TCINDEX_HASH,
       TCA_TCINDEX_MASK,
       TCA_TCINDEX_SHIFT,
       TCA_TCINDEX_FALL_THROUGH,
       TCA_TCINDEX_CLASSID,
       TCA_TCINDEX_POLICE,
       TCA_TCINDEX_ACT,
       __TCA_TCINDEX_MAX
};

#define TCA_TCINDEX_MAX     (__TCA_TCINDEX_MAX - 1)
#endif


static const uint32_t qd_handle = TC_H_MAKE(1 << 16, 0);
static const uint32_t clsid = TC_H_MAKE(1 << 16, 1);
static const uint32_t shift = 10;
static const uint16_t mask = 0xffff;

/* rtnetlink payloads */
static const struct tc_htb_glob qd_opt = {
	.rate2quantum = 10,
	.version = 3,
	.defcls = 30
};
static struct tc_htb_opt cls_opt = {};

/* htb qdisc and class options */
static const struct tst_netlink_attr_list qd_config[] = {
	{TCA_OPTIONS, NULL, 0, (const struct tst_netlink_attr_list[]){
		{TCA_HTB_INIT, &qd_opt, sizeof(qd_opt), NULL},
		{0, NULL, -1, NULL}
	}},
	{0, NULL, -1, NULL}
};
static const struct tst_netlink_attr_list cls_config[] = {
	{TCA_OPTIONS, NULL, 0, (const struct tst_netlink_attr_list[]){
		{TCA_HTB_PARMS, &cls_opt, sizeof(cls_opt), NULL},
		{0, NULL, -1, NULL}
	}},
	{0, NULL, -1, NULL}
};

/* tcindex filter options */
static const struct tst_netlink_attr_list f_config[] = {
	{TCA_OPTIONS, NULL, 0, (const struct tst_netlink_attr_list[]){
		{TCA_TCINDEX_MASK, &mask, sizeof(mask), NULL},
		{TCA_TCINDEX_SHIFT, &shift, sizeof(shift), NULL},
		{TCA_TCINDEX_CLASSID, &clsid, sizeof(clsid), NULL},
		{0, NULL, -1, NULL}
	}},
	{0, NULL, -1, NULL}
};

static void setup(void)
{
	tst_setup_netns();
	NETDEV_ADD_DEVICE(DEVNAME, "dummy");

	cls_opt.rate.rate = cls_opt.ceil.rate = 256000;
	cls_opt.buffer = 1000000 * 1600 / cls_opt.rate.rate;
	cls_opt.cbuffer = 1000000 * 1600 / cls_opt.ceil.rate;
}

static void run(void)
{
	int ret;

	NETDEV_ADD_QDISC(DEVNAME, AF_UNSPEC, TC_H_ROOT, qd_handle, "htb",
		qd_config);
	NETDEV_ADD_TRAFFIC_CLASS(DEVNAME, qd_handle, clsid, "htb", cls_config);
	NETDEV_ADD_TRAFFIC_FILTER(DEVNAME, qd_handle, 10, ETH_P_IP, 1,
		"tcindex", f_config);
	NETDEV_REMOVE_TRAFFIC_FILTER(DEVNAME, qd_handle, 10, ETH_P_IP,
		1, "tcindex");
	ret = tst_netdev_add_traffic_filter(__FILE__, __LINE__, 0, DEVNAME,
		qd_handle, 10, ETH_P_IP, 1, "tcindex", f_config);
	TST_ERR = tst_netlink_errno;
	NETDEV_REMOVE_QDISC(DEVNAME, AF_UNSPEC, TC_H_ROOT, qd_handle, "htb");

	if (ret)
		tst_res(TPASS, "Removing tcindex filter works correctly");
	else if (TST_ERR == EEXIST)
		tst_res(TFAIL, "Kernel traffic filter list is corrupted");
	else
		tst_brk(TBROK | TTERRNO, "Unexpected rtnetlink error");
}

static void cleanup(void)
{
	NETDEV_REMOVE_DEVICE(DEVNAME);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_VETH",
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		"CONFIG_NET_SCH_HTB",
		"CONFIG_NET_CLS_TCINDEX",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.needs_drivers = (const char *const []) {
		"dummy",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "8c710f75256b"},
		{"CVE", "2023-1829"},
		{}
	}
};
