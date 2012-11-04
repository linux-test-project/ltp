// tbase.h
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


#define TMOD_DRIVER_NAME	"ltp test drivers/base"
#define DEVICE_NAME		"/dev/tbase"
#define MAG_NUM			'k'

/* put ioctl flags here, use the _IO macro which is
 found in linux/ioctl.h, takes a letter, and an
 integer */

#define TBASEMAJOR      253

#define DEV_PROBE		_IO(MAG_NUM, 1)
#define REG_DEVICE		_IO(MAG_NUM, 2)
#define UNREG_DEVICE		_IO(MAG_NUM, 3)
#define BUS_ADD			_IO(MAG_NUM, 4)
#define FIND_BUS		_IO(MAG_NUM, 5)
#define BUS_REMOVE		_IO(MAG_NUM, 6)
#define GET_DRV			_IO(MAG_NUM, 7)
#define PUT_DRV			_IO(MAG_NUM, 8)
#define DRV_REG			_IO(MAG_NUM, 9)
#define DRV_UNREG		_IO(MAG_NUM, 10)
#define REG_FIRM		_IO(MAG_NUM, 11)
#define CREATE_FILE		_IO(MAG_NUM, 12)
#define DEV_SUSPEND		_IO(MAG_NUM, 13)
#define DEV_FILE		_IO(MAG_NUM, 14)
#define BUS_RESCAN		_IO(TBASEMAJOR, 15)
#define BUS_FILE		_IO(MAG_NUM, 16)
#define CLASS_REG		_IO(MAG_NUM, 17)
#define CLASS_UNREG		_IO(MAG_NUM, 18)
#define CLASS_FILE		_IO(MAG_NUM, 19)
#define CLASS_GET		_IO(MAG_NUM, 20)
#define CLASSDEV_REG		_IO(MAG_NUM, 21)
#define CLASSINT_REG		_IO(MAG_NUM, 22)
#define SYSDEV_REG		_IO(MAG_NUM, 23)
#define SYSDEV_UNREG		_IO(MAG_NUM, 24)
#define SYSDEV_CLS_REG		_IO(MAG_NUM, 25)
#define SYSDEV_CLS_UNREG	_IO(MAG_NUM, 26)

/* interface for passing structures between user
 space and kernel space easily */

struct tmod_interface {
	int     in_len;         // input data length
        caddr_t in_data;        // input data
        int     out_rc;         // return code from the test
        int     out_len;        // output data length
        caddr_t out_data;       // output data
};
typedef struct tmod_interface tmod_interface_t;





