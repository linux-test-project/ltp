// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verifies that the link state attributes exported under
 * /sys/class/net/<iface>/ correctly mirror the administrative state of a
 * veth peer.
 *
 * A veth device's carrier is expected to be up only while its peer is up:
 * veth_open()/veth_close() call netif_carrier_on()/netif_carrier_off() based
 * on whether the peer is running. The test creates a veth pair and checks
 * that bringing the peer down and back up is reflected in:
 *
 * - carrier - 1 while the peer is up, 0 while it is down
 * - operstate - ``up`` while the peer is up, ``lowerlayerdown`` while it is
 *   down
 * - carrier_up_count / carrier_down_count - incremented by exactly one on
 *   each corresponding transition
 * - carrier_changes - always equal to carrier_up_count + carrier_down_count
 *
 * Note that while the NETDEV_SET_STATE() netlink call itself is synchronous,
 * operstate is not: veth_open()/veth_close() call netif_carrier_on()/off(),
 * but the kernel's linkwatch mechanism (net/core/link_watch.c) applies the
 * resulting operstate transition asynchronously via a workqueue, so reading
 * it right after the netlink call can still observe the previous value for
 * a little while, particularly on a loaded/slow system. The test retries
 * (see read_operstate() in sys_net_common.h) rather than using
 * poll(2)/select(2): unlike some other sysfs attributes,
 * /sys/class/net/<iface>/carrier does not support poll() based notification
 * on Linux, link state changes are reported to userspace via rtnetlink
 * instead.
 *
 * The test also checks attributes that are not tied to the peer's state:
 *
 * - mtu - directly writable via sysfs; a few valid values are written and
 *   read back, and one syntactically invalid value is rejected (with the mtu
 *   left unchanged)
 * - address - read-only in sysfs, changed via rtnetlink instead (the
 *   interface is brought down first since most drivers require this); while
 *   doing so, the IFF_UP bit in ``flags`` is also cross-checked against the
 *   interface's own administrative state
 * - netdev_group and ifalias - directly writable via sysfs, a value is
 *   written and read back
 *
 * This needs root to create the veth pair.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>

#include "tst_test.h"
#include "tst_netdevice.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

#define IFNAME1 "ltp_vethp1"
#define IFNAME2 "ltp_vethp2"
#define IFACE_PATH PATH_CLASS_NET "/" IFNAME1

static int veth_created;

static long read_mtu(void)
{
	return TST_SYSFS_READ_LI(IFACE_PATH "/mtu");
}

#include "sys_net_common.h"

static void setup(void)
{
	CREATE_VETH_PAIR(IFNAME1, IFNAME2);
	veth_created = 1;

	NETDEV_SET_STATE(IFNAME1, 1);
	NETDEV_SET_STATE(IFNAME2, 1);
}

static unsigned long read_flags(void)
{
	return TST_SYSFS_READ_LX(IFACE_PATH "/flags");
}

static void check_address(void)
{
	static const unsigned char new_addr[6] = {0x02, 0, 0, 0, 0, 0x01};
	char actual[32];
	long iff_up;

	tst_res(TINFO, "IFNAME1 is up before the address change");
	iff_up = read_flags() & IFF_UP;
	TST_EXP_EQ_LI(iff_up, IFF_UP);

	NETDEV_SET_STATE(IFNAME1, 0);
	tst_res(TINFO, "IFNAME1 is down for the address change");
	iff_up = read_flags() & IFF_UP;
	TST_EXP_EQ_LI(iff_up, 0);

	NETDEV_SET_HWADDR(IFNAME1, new_addr, sizeof(new_addr));
	NETDEV_SET_STATE(IFNAME1, 1);
	tst_res(TINFO, "IFNAME1 is up again after the address change");
	iff_up = read_flags() & IFF_UP;
	TST_EXP_EQ_LI(iff_up, IFF_UP);

	TST_SYSFS_READ_STR(actual, sizeof(actual), IFACE_PATH "/address");

	TST_EXP_EQ_STR(actual, "02:00:00:00:00:01");
}

static void check_netdev_group(void)
{
	long group;

	if (FILE_PRINTF(IFACE_PATH "/netdev_group", "%d", 7)) {
		tst_res(TFAIL, "Failed to set netdev_group");
		return;
	}

	group = TST_SYSFS_READ_LI(IFACE_PATH "/netdev_group");

	TST_EXP_EQ_LI(group, 7);
}

static void check_ifalias(void)
{
	char alias[64];

	if (FILE_PRINTF(IFACE_PATH "/ifalias", "%s", "ltp-test-alias")) {
		tst_res(TFAIL, "Failed to set ifalias");
		return;
	}

	TST_SYSFS_READ_STR(alias, sizeof(alias), IFACE_PATH "/ifalias");
	TST_EXP_EQ_STR(alias, "ltp-test-alias");
}

static void run(void)
{
	struct netdev_state s0, s1, s2;

	TST_RETRY_FUNC(read_operstate(&s0, "up"), TST_RETVAL_EQ0);
	check_state(&s0, 1, "up", "both ends up");

	NETDEV_SET_STATE(IFNAME2, 0);
	TST_RETRY_FUNC(read_operstate(&s1, "lowerlayerdown"), TST_RETVAL_EQ0);
	check_state(&s1, 0, "lowerlayerdown", "peer down");
	check_state_delta(&s0, &s1, 0, 1, "peer down transition");

	NETDEV_SET_STATE(IFNAME2, 1);
	TST_RETRY_FUNC(read_operstate(&s2, "up"), TST_RETVAL_EQ0);
	check_state(&s2, 1, "up", "peer back up");
	check_state_delta(&s1, &s2, 1, 0, "peer up transition");

	check_mtu_valid(68);
	check_mtu_valid(1500);
	check_mtu_valid(9000);
	check_mtu_valid(65535);
	check_mtu_invalid("-1");
	check_mtu_invalid("0");
	check_mtu_invalid("67");
	check_mtu_invalid("70000");

	check_address();

	check_netdev_group();
	check_ifalias();
}

static void cleanup(void)
{
	if (veth_created)
		NETDEV_REMOVE_DEVICE(IFNAME1);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_VETH",
		NULL
	},
};
