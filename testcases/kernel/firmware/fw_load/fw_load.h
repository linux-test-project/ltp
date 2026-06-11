// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef FW_LOAD_H
#define FW_LOAD_H

#define MNAME_KO "ltp_fw_load.ko"
#define FW_NAME	"load_tst.fw"
#define FW_SIZE	0x1000
#define FW_NUM 5
#define FW_PATH "/sys/module/firmware_class/parameters/path"
#define DEV_FWNUM "/sys/devices/ltp_fw_load/fwnum"
#define DEV_RESULT "/sys/devices/ltp_fw_load/result"
#define LIB_PATH "/lib/firmware"

#endif
