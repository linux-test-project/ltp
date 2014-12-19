/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In this header file keep all flags and other
 * structures that will be needed in both kernel
 * and user space. Specifically the ioctl flags
 * will go in here so that in user space a program
 * can specify flags for the ioctl call.
 *
 *  HISTORY     :
 *      11/19/2003 Kai Zhao (ltcd3@cn.ibm.com)
 *
 *  CODE COVERAGE: 74.9% - fs/bio.c (Total Coverage)
 *
 */

#define TMOD_DRIVER_NAME	"ltp test block I/O layer module"
#define TBIO_DEVICE_NAME	"ltp_tbio"
#define DEVICE_NAME		"/dev/tbio"
#define MAG_NUM			'k'
int TBIO_MAJOR;

#define TBIO_TO_DEV              1
#define TBIO_FROM_DEV            2

/* put ioctl flags here, use the _IO macro which is
 found in linux/ioctl.h, takes a letter, and an
 integer */

#define LTP_TBIO_CLONE		_IO(MAG_NUM,1)
#define LTP_TBIO_ALLOC		_IO(MAG_NUM,2)
#define LTP_TBIO_GET_NR_VECS	_IO(MAG_NUM,3)
#define LTP_TBIO_PUT		_IO(MAG_NUM,4)
#define LTP_TBIO_SPLIT		_IO(MAG_NUM,5)
#define LTP_TBIO_DO_IO		_IO(MAG_NUM,6)
#define LTP_TBIO_ADD_PAGE	_IO(MAG_NUM,7)

/* memory between the kernel and user space is
 seperated, so that if a structure is needed
 to be passed between kernel and user space
 a call must be made to copy_to_user or copy
 from user. Use this structure to streamline
 that process. For example: A function that
 writes to a disc takes in a ki_write_t
 pointer from userspace. In the user space
 program specify the length of the pointer as
 in_len, and in_data as the actual structure. */

struct tbio_interface {
	void *data;	/* input data */
	int data_len;	/* input data length */
	int direction;	/* read or write form DEV */
	char *cmd;	/* read or write */
	unsigned short cmd_len;	/* length of cmd */
};
typedef struct tbio_interface tbio_interface_t;
