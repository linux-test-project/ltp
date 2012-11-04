/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#define MAG_NUM 'k'
#define MODULE_TEST_COMPONENT   0x00023000
#define OPTION1                 _IO(MAG_NUM, 1)
#define _COMPONENT              MODULE_TEST_COMPONENT
#define TEST_DRIVER_NAME        "nls test module"
#define DEVICE_NAME             "nlsTest"
#define DEV_PATH                "/dev"
#define NLS_DEVICE_PATH         DEV_PATH "/"
#define NLSMAJOR                253
#define MINOR_SHIFT_BITS        3
#define MAX_PARTITIONS          8
#define MAX_NUM_DISKS           3
#define DEV_NAME                "nlsTest"

typedef struct _nlsdev_cmd {
        u_int32_t cmd;
        u_int32_t status;
} nlsdev_cmd_t;

typedef enum nlsdev_ioctl_cmds_s {
        NLS_IOCTL_NUMBER = 0x5500
} nlsdev_ioctl_cmds_t;

#define NLSDEV_CMD              _IOR(NLSMAJOR, NLS_IOCTL_NUMBER, nlsdev_cmd_t**)
