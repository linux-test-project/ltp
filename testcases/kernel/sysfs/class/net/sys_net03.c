// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verifies that bridge port sysfs attributes correctly appear, hold sane
 * values, and disappear again as a network device is enslaved to and
 * released from a bridge.
 *
 * The test creates a bridge and a veth pair, then:
 *
 * - verifies /sys/class/net/<bridge>/brif/ is empty and
 *   /sys/class/net/<port>/brport does not exist before enslaving
 * - enslaves one end of the veth pair to the bridge
 * - verifies /sys/class/net/<bridge>/brif/<port> appears and
 *   /sys/class/net/<port>/brport/{state,priority,path_cost} hold sane values
 * - verifies /sys/class/net/<bridge>/bridge/{stp_state,forward_delay} hold
 *   sane values
 * - releases the port from the bridge
 * - verifies /sys/class/net/<bridge>/brif/ is empty and
 *   /sys/class/net/<port>/brport does not exist again
 *
 * This needs root to create the bridge and veth devices.
 */

#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "tst_test.h"
#include "tst_netdevice.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

#define BRNAME "ltp_br0"
#define IFNAME1 "ltp_vethb1"
#define IFNAME2 "ltp_vethb2"

#define BR_PATH PATH_CLASS_NET "/" BRNAME
#define PORT_PATH PATH_CLASS_NET "/" IFNAME1

static int bridge_created;
static int veth_created;

static int count_dir_entries(const char *path)
{
	DIR *d;
	struct dirent *ent;
	int count = 0;

	d = TST_SYSFS_TRY_OPENDIR(path);

	while ((ent = SAFE_READDIR(d))) {
		if (ent->d_name[0] == '.')
			continue;

		count++;
	}

	SAFE_CLOSEDIR(d);

	return count;
}

static void setup(void)
{
	NETDEV_ADD_DEVICE(BRNAME, "bridge");
	bridge_created = 1;

	CREATE_VETH_PAIR(IFNAME1, IFNAME2);
	veth_created = 1;

	NETDEV_SET_STATE(BRNAME, 1);
	NETDEV_SET_STATE(IFNAME1, 1);
	NETDEV_SET_STATE(IFNAME2, 1);
}

static void check_not_enslaved(const char *desc)
{
	int nports = count_dir_entries(BR_PATH "/brif");

	tst_res(TINFO, "%s", desc);

	TST_EXP_EQ_LI(nports, 0);

	if (access(PORT_PATH "/brport", F_OK))
		tst_res(TPASS, PORT_PATH "/brport does not exist");
	else
		tst_res(TFAIL, PORT_PATH "/brport unexpectedly exists");
}

static void check_enslaved(void)
{
	int nports = count_dir_entries(BR_PATH "/brif");

	tst_res(TINFO, "port enslaved to bridge");

	TST_EXP_EQ_LI(nports, 1);

	if (!access(BR_PATH "/brif/" IFNAME1, F_OK))
		tst_res(TPASS, BR_PATH "/brif/" IFNAME1 " exists");
	else
		tst_res(TFAIL, BR_PATH "/brif/" IFNAME1 " does not exist");

	TST_SYSFS_ASSERT_RANGELL(0, 4, PORT_PATH "/brport/state");
	TST_SYSFS_ASSERT_RANGELL(0, 255, PORT_PATH "/brport/priority");
	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX, PORT_PATH "/brport/path_cost");

	TST_SYSFS_ASSERT_RANGELL(0, 2, BR_PATH "/bridge/stp_state");
	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX, BR_PATH "/bridge/forward_delay");
}

static void run(void)
{
	check_not_enslaved("before enslaving");

	NETDEV_SET_MASTER(IFNAME1, BRNAME);
	check_enslaved();

	NETDEV_SET_MASTER(IFNAME1, NULL);
	check_not_enslaved("after releasing");
}

static void cleanup(void)
{
	if (veth_created)
		NETDEV_REMOVE_DEVICE(IFNAME1);

	if (bridge_created)
		NETDEV_REMOVE_DEVICE(BRNAME);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_VETH",
		"CONFIG_BRIDGE",
		NULL
	},
};
