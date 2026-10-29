// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verifies that the link state attributes exported under
 * /sys/class/net/<iface>/ correctly mirror the attach/detach state of a TUN
 * device's file descriptor, and that TUN/TAP-specific sysfs attributes hold
 * sane values.
 *
 * Unlike veth, a TUN device has no peer network device: instead, its carrier
 * is controlled by whether a userspace process currently has the device's
 * /dev/net/tun file descriptor attached to it (tun_attach()/tun_detach() in
 * the kernel call netif_carrier_on()/netif_carrier_off() respectively). The
 * device is created persistent (TUNSETPERSIST) so that it survives the file
 * descriptor being closed, which is what allows the attach/detach cycle to
 * be exercised here without the interface disappearing. The test checks
 * that closing and reopening the attachment is reflected in:
 *
 * - carrier - 1 while a file descriptor is attached, 0 while detached
 * - operstate - ``up`` while attached, ``down`` while detached. Unlike veth,
 *   this is plain ``down`` rather than ``lowerlayerdown``: the latter is only
 *   reported when the carrier-down state is attributable to a registered
 *   upper/lower netdevice relationship (a veth peer, a bond slave, a VLAN's
 *   parent, ...), whereas a TUN device's carrier is driven by a userspace
 *   file descriptor attachment, which is not a netdevice dependency at all.
 * - carrier_up_count / carrier_down_count - incremented by exactly one on
 *   each corresponding transition
 * - carrier_changes - always equal to carrier_up_count + carrier_down_count
 *
 * These four attributes are generic net_device core statistics updated by
 * netif_carrier_on()/off() regardless of the underlying driver, so the
 * expected behavior mirrors the veth peer test in sys_net02.c, aside from
 * the ``down`` vs ``lowerlayerdown`` difference above.
 *
 * The very first attach happens as part of the same TUNSETIFF call that
 * registers the netdevice, before which netif_carrier_on() skips updating
 * carrier_up_count/operstate for it. setup() therefore performs a throwaway
 * detach/reattach cycle so that run() starts from a state where carrier
 * tracking already behaves normally.
 *
 * The test also checks attributes that are not tied to the attach state:
 *
 * - mtu - directly writable via sysfs; a few valid values are written and
 *   read back, and one syntactically invalid value is rejected (with the mtu
 *   left unchanged)
 * - tun_flags - reflects (at least) the IFF_TUN, IFF_NO_PI and IFF_PERSIST
 *   bits the device was created/configured with, and never has IFF_TAP set
 * - owner / group - default to -1 (no restriction) since neither
 *   TUNSETOWNER nor TUNSETGROUP was used
 * - a second, TAP-mode device is created solely to cross-check that type and
 *   addr_len correctly differ between the two: a TUN device has no
 *   link-layer header (type == ARPHRD_NONE, addr_len == 0), while a TAP
 *   device behaves like Ethernet (type == ARPHRD_ETHER, addr_len == 6)
 *
 * This needs root to create the TUN/TAP devices.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>

#include "tst_test.h"
#include "tst_netdevice.h"
#include "tst_safe_file_ops.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

#define TUN_PATH "/dev/net/tun"

#define IFNAME_TUN "ltp_tun0"
#define IFNAME_TAP "ltp_tap0"
#define IFACE_PATH PATH_CLASS_NET "/" IFNAME_TUN

/* include/uapi/linux/if_arp.h, avoided pulling in the whole header. */
#define ARPHRD_ETHER 1
#define ARPHRD_NONE 0xFFFE

static int tun_fd = -1;
static int tap_fd = -1;
static int tun_created;
static int tap_created;

static long read_mtu(void)
{
	return TST_SYSFS_READ_LI(IFACE_PATH "/mtu");
}

#include "sys_net_common.h"

static int open_tun(const char *ifname, short flags)
{
	struct ifreq ifr = { .ifr_flags = flags };
	int fd = SAFE_OPEN(TUN_PATH, O_RDWR);

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	SAFE_IOCTL(fd, TUNSETIFF, (void *)&ifr);

	return fd;
}

static void setup(void)
{
	tun_fd = open_tun(IFNAME_TUN, IFF_TUN | IFF_NO_PI);
	SAFE_IOCTL(tun_fd, TUNSETPERSIST, 1);
	tun_created = 1;

	/*
	 * The initial attach above happens as part of the same TUNSETIFF call
	 * that registers the netdevice, before which netif_carrier_on() skips
	 * updating carrier_up_count/operstate for it. Detach and reattach
	 * once here so run() starts from a state where carrier tracking
	 * already behaves normally, see the top comment.
	 */
	SAFE_CLOSE(tun_fd);
	NETDEV_SET_STATE(IFNAME_TUN, 1);
	tun_fd = open_tun(IFNAME_TUN, IFF_TUN | IFF_NO_PI);

	tap_fd = open_tun(IFNAME_TAP, IFF_TAP | IFF_NO_PI);
	SAFE_IOCTL(tap_fd, TUNSETPERSIST, 1);
	tap_created = 1;
}

static void check_tun_flags(void)
{
	unsigned long flags;

	flags = TST_SYSFS_READ_LX(IFACE_PATH "/tun_flags");

	tst_res(TINFO, "%s/tun_flags = 0x%lx", IFACE_PATH, flags);

	TST_EXP_EQ_LI(flags & IFF_TUN, IFF_TUN);
	TST_EXP_EQ_LI(flags & IFF_TAP, 0);
	TST_EXP_EQ_LI(flags & IFF_NO_PI, IFF_NO_PI);
	TST_EXP_EQ_LI(flags & IFF_PERSIST, IFF_PERSIST);
}

static void check_owner_group(void)
{
	TST_SYSFS_EXP_EQ_LI(-1, IFACE_PATH "/owner");
	TST_SYSFS_EXP_EQ_LI(-1, IFACE_PATH "/group");
}

static void check_tun_vs_tap_type(void)
{
	TST_SYSFS_EXP_EQ_LI(ARPHRD_NONE, PATH_CLASS_NET "/" IFNAME_TUN "/type");
	TST_SYSFS_EXP_EQ_LI(0, PATH_CLASS_NET "/" IFNAME_TUN "/addr_len");

	TST_SYSFS_EXP_EQ_LI(ARPHRD_ETHER, PATH_CLASS_NET "/" IFNAME_TAP "/type");
	TST_SYSFS_EXP_EQ_LI(6, PATH_CLASS_NET "/" IFNAME_TAP "/addr_len");
}

static void run(void)
{
	struct netdev_state s0, s1, s2;

	TST_RETRY_FUNC(read_operstate(&s0, "up"), TST_RETVAL_EQ0);
	check_state(&s0, 1, "up", "tun attached and up");

	SAFE_CLOSE(tun_fd);
	TST_RETRY_FUNC(read_operstate(&s1, "down"), TST_RETVAL_EQ0);
	check_state(&s1, 0, "down", "tun detached");
	check_state_delta(&s0, &s1, 0, 1, "detach transition");

	tun_fd = open_tun(IFNAME_TUN, IFF_TUN | IFF_NO_PI);
	TST_RETRY_FUNC(read_operstate(&s2, "up"), TST_RETVAL_EQ0);
	check_state(&s2, 1, "up", "tun reattached");
	check_state_delta(&s1, &s2, 1, 0, "reattach transition");

	check_mtu_valid(68);
	check_mtu_valid(1500);
	check_mtu_valid(9000);
	check_mtu_valid(65535);
	check_mtu_invalid("-1");
	check_mtu_invalid("0");
	check_mtu_invalid("67");
	check_mtu_invalid("70000");

	check_tun_flags();
	check_owner_group();
	check_tun_vs_tap_type();
}

static void cleanup(void)
{
	if (tun_fd != -1)
		SAFE_CLOSE(tun_fd);

	if (tap_fd != -1)
		SAFE_CLOSE(tap_fd);

	if (tun_created)
		NETDEV_REMOVE_DEVICE(IFNAME_TUN);

	if (tap_created)
		NETDEV_REMOVE_DEVICE(IFNAME_TAP);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_TUN",
		NULL
	},
};
