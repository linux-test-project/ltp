// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2019-2023
 */

/*\
 * [Description]
 *
 * Very simple uevent netlink socket test.
 *
 * We fork a child that listens for a kernel events while parents creates and
 * removes a tun network device which should produce two several add and remove
 * events.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "tst_kconfig.h"
#include "tst_test.h"

#include "uevent.h"

#define TUN_PATH "/dev/net/tun"
#define CONFIG_RPS "CONFIG_RPS"
#define MAX_UEVENTS 7

static struct uevent_desc add = {
	.msg = "add@/devices/virtual/net/ltp-tun0",
	.value_cnt = 4,
	.values = (const char*[]) {
		"ACTION=add",
		"DEVPATH=/devices/virtual/net/ltp-tun0",
		"SUBSYSTEM=net",
		"INTERFACE=ltp-tun0",
	}
};

static struct uevent_desc add_rx = {
	.msg = "add@/devices/virtual/net/ltp-tun0/queues/rx-0",
	.value_cnt = 3,
	.values = (const char*[]) {
		"ACTION=add",
		"DEVPATH=/devices/virtual/net/ltp-tun0/queues/rx-0",
		"SUBSYSTEM=queues",
	}
};

static struct uevent_desc add_tx = {
	.msg = "add@/devices/virtual/net/ltp-tun0/queues/tx-0",
	.value_cnt = 3,
	.values = (const char*[]) {
		"ACTION=add",
		"DEVPATH=/devices/virtual/net/ltp-tun0/queues/tx-0",
		"SUBSYSTEM=queues",
	}
};

static struct uevent_desc rem_rx = {
	.msg = "remove@/devices/virtual/net/ltp-tun0/queues/rx-0",
	.value_cnt = 3,
	.values = (const char*[]) {
		"ACTION=remove",
		"DEVPATH=/devices/virtual/net/ltp-tun0/queues/rx-0",
		"SUBSYSTEM=queues",
	}
};

static struct uevent_desc rem_tx = {
	.msg = "remove@/devices/virtual/net/ltp-tun0/queues/tx-0",
	.value_cnt = 3,
	.values = (const char*[]) {
		"ACTION=remove",
		"DEVPATH=/devices/virtual/net/ltp-tun0/queues/tx-0",
		"SUBSYSTEM=queues",
	}
};

static struct uevent_desc rem = {
	.msg = "remove@/devices/virtual/net/ltp-tun0",
	.value_cnt = 4,
	.values = (const char*[]) {
		"ACTION=remove",
		"DEVPATH=/devices/virtual/net/ltp-tun0",
		"SUBSYSTEM=net",
		"INTERFACE=ltp-tun0",
	}
};
static const struct uevent_desc *uevents[MAX_UEVENTS];

static void generate_tun_uevents(void)
{
	int fd = SAFE_OPEN(TUN_PATH, O_RDWR);

	struct ifreq ifr = {
		.ifr_flags = IFF_TUN,
		.ifr_name = "ltp-tun0",
	};

	SAFE_IOCTL(fd, TUNSETIFF, (void*)&ifr);

	SAFE_IOCTL(fd, TUNSETPERSIST, 0);

	SAFE_CLOSE(fd);
}

static void verify_uevent(void)
{
	int pid, fd;

	pid = SAFE_FORK();
	if (!pid) {
		fd = open_uevent_netlink();
		TST_CHECKPOINT_WAKE(0);
		wait_for_uevents(fd, uevents);
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	generate_tun_uevents();

	wait_for_pid(pid);
}

static void setup(void)
{
	struct tst_kconfig_var kconfig = {
		.id = CONFIG_RPS,
		.id_len = sizeof(CONFIG_RPS) - 1,
	};
	int i = 0;

	tst_kconfig_read(&kconfig, 1);

	uevents[i++] = &add;
	if (kconfig.choice == 'y')
		uevents[i++] = &add_rx;
	uevents[i++] = &add_tx;
	if (kconfig.choice == 'y')
		uevents[i++] = &rem_rx;
	uevents[i++] = &rem_tx;
	uevents[i++] = &rem;
	uevents[i++] = NULL;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_uevent,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_drivers = (const char *const []) {
		"tun",
		NULL
	},
	.needs_root = 1
};
