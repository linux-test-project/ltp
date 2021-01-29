// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "tst_cmd.h"
#include "tst_safe_stdio.h"
#include "tst_safe_file_ops.h"

#include <linux/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

static char *can_dev_name;
static int can_created_dev;

static void can_cmd(const char *const argv[])
{
	tst_cmd(argv, NULL, NULL, TST_CMD_TCONF_ON_MISSING);
}

#define CAN_CMD(...) can_cmd((const char *const[]){ __VA_ARGS__, NULL })

static void can_setup_vcan(void)
{
	unsigned int flags;
	char *path;

	if (can_dev_name)
		goto check_echo;

	can_dev_name = "vcan0";

	tst_res(TINFO, "Creating vcan0 device; use -D option to avoid this");

	CAN_CMD("modprobe", "-r", "vcan");
	CAN_CMD("modprobe", "vcan", "echo=1");

	can_created_dev = 1;

	CAN_CMD("ip", "link", "add", "dev", "vcan0", "type", "vcan");
	CAN_CMD("ip", "link", "set", "dev", "vcan0", "up");

check_echo:
	SAFE_ASPRINTF(&path, "/sys/class/net/%s/flags", can_dev_name);
	if (FILE_SCANF(path, "%x", &flags) || !(flags & IFF_ECHO)) {
		tst_res(TWARN,
			"Could not determine if ECHO is set on %s. This may effect code coverage.",
			can_dev_name);
	}
}

static void can_cleanup_vcan(void)
{
	if (!can_created_dev)
		return;

	CAN_CMD("ip", "link", "set", "dev", "vcan0", "down");
	CAN_CMD("ip", "link", "del", "dev", "vcan0");
	CAN_CMD("modprobe", "-r", "vcan");
}
