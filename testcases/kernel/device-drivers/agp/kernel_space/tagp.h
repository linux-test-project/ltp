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

/*
 * In this header file keep all flags and other
 * structures that will be needed in both kernel
 * and user space. Specifically the ioctl flags
 * will go in here so that in user space a program
 * can specify flags for the ioctl call.
 *
 * author: Kai Zhao
 * date:   08/25/2003
 *
 */

#define tagp_DRIVER_NAME	"ltp agp module"
#define DEVICE_NAME		"/dev/tagp"
#define TAGP_MAJOR      252
#define MAG_NUM			'k'
#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev) ((dev)->owner = THIS_MODULE)
#endif

/* put ioctl flags here, use the _IO macro which is
 found in linux/ioctl.h, takes a letter, and an
 integer */

#define TEST_PCI_FIND_DEV   			_IO(MAG_NUM, 1)
#define TEST_BACKEND_ACQUIRE			_IO(MAG_NUM, 2)
#define TEST_BACKEND_RELEASE			_IO(MAG_NUM, 3)
#define TEST_ALLOC_BRIDGE			_IO(MAG_NUM, 4)
#define TEST_PUT_BRIDGE				_IO(MAG_NUM, 5)
#define TEST_CREATE_AND_FREE_MEMORY		_IO(MAG_NUM, 6)
//#define TEST_FREE_MEMORY			_IO(MAG_NUM, 7)
#define TEST_COPY_INFO				_IO(MAG_NUM, 8)
//#define TEST_ALLOC_MEMORY_AND_BAND_UNBAND	_IO(MAG_NUM, 9)
#define TEST_GET_VERSION			_IO(MAG_NUM, 10)
#define TEST_GENERIC_ENABLE			_IO(MAG_NUM, 11)
#define TEST_NUM_ENTRIES			_IO(MAG_NUM, 12)
#define TEST_GENERIC_CREATE_GATT_TABLE		_IO(MAG_NUM, 13)
#define TEST_GENERIC_FREE_GATT_TABLE		_IO(MAG_NUM, 14)
#define TEST_GENERIC_INSERT_MEMORY		_IO(MAG_NUM, 15)
#define TEST_GENERIC_ALLOC_BY_TYPE		_IO(MAG_NUM, 16)
#define TEST_GENERIC_ALLOC_PAGE			_IO(MAG_NUM, 17)
#define TEST_ENABLE				_IO(MAG_NUM, 19)
#define TEST_GLOBAL_CACHE_FLUSH			_IO(MAG_NUM, 20)
#define TEST_GENERIC_MASK_MEMORY		_IO(MAG_NUM, 21)

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

struct tagp_interface {
	int     in_len;         // input data length
        caddr_t in_data;        // input data
        int     out_rc;         // return code from the test
        int     out_len;        // output data length
        caddr_t out_data;       // output data
};

typedef struct tagp_interface tagp_interface_t;





