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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *  FILE        : LtpAcpi.h
 *  DESCRIPTION :
 *  HISTORY:
 *    06/09/2003 Initial creation mridge@us.ibm.com
 *      -Ported
 *  updated - 01/09/2005 Updates from Intel to add functionality
 *
 */

#define ACPI_LTP_TEST_COMPONENT	        	0x00020999
#define ACPI_LTP_TEST_CLASS		        	"ltp_test"
#define ACPI_LTP_TEST_HID 		        	"ACPI0999"
#define ACPI_LTP_TEST_DRIVER_NAME		    "ACPI LTP Test Driver"
#define ACPI_LTP_TEST_DEVICE_NAME	    	"LTP Test"
#define ACPI_LTP_TEST_FILE_STATE    		"state"
#define ACPI_LTP_TEST_NOTIFY_STATUS		    0x80
#define ACPI_LTP_TEST_STATUS_OFFLINE		0x00
#define ACPI_LTP_TEST_STATUS_ONLINE	    	0x01
#define ACPI_LTP_TEST_STATUS_UNKNOWN		0xFF
#define _COMPONENT		ACPI_LTP_TEST_COMPONENT
#define ACPI_TLP_TEST_MODULE_NAME		("acpi_ltp_test")
#define ACPI_NS_SYSTEM_BUS          "_SB_"
#define ACPI_BATTERY_FORMAT_BIF	"NNNNNNNNNSSSS"
#define ACPI_BATTERY_FORMAT_BST	"NNNN"


#define ACPI_TYPE_ANY                   0x00
#define ACPI_TYPE_INTEGER               0x01  /* Byte/Word/Dword/Zero/One/Ones */
#define ACPI_TYPE_STRING                0x02
#define ACPI_TYPE_BUFFER                0x03
#define ACPI_TYPE_PACKAGE               0x04  /* byte_const, multiple data_term/Constant/super_name */
#define ACPI_TYPE_FIELD_UNIT            0x05
#define ACPI_TYPE_DEVICE                0x06  /* Name, multiple Node */
#define ACPI_TYPE_EVENT                 0x07
#define ACPI_TYPE_METHOD                0x08  /* Name, byte_const, multiple Code */
#define ACPI_TYPE_MUTEX                 0x09
#define ACPI_TYPE_REGION                0x0A
#define ACPI_TYPE_POWER                 0x0B  /* Name,byte_const,word_const,multi Node */
#define ACPI_TYPE_PROCESSOR             0x0C  /* Name,byte_const,Dword_const,byte_const,multi nm_o */
#define ACPI_TYPE_THERMAL               0x0D  /* Name, multiple Node */
#define ACPI_TYPE_BUFFER_FIELD          0x0E
#define ACPI_TYPE_DDB_HANDLE            0x0F
#define ACPI_TYPE_DEBUG_OBJECT          0x10

#define ACPI_TYPE_EXTERNAL_MAX          0x10
#define LTPMAJOR                        252

/* Use 'k' as magic number */
#define LTPACPI_IOC_MAGIC  'k'
#define TOMINOR(x) ((x & 3) | ((x & 4) << 5))


#define DEV_PATH                            "/dev"
#define LTP_ACPI_DIR_NAME                   ""
#define LTP_ACPI_DEV_NAME                   "LTP"
#define LTP_ACPI_DEV_NODE_PATH              DEV_PATH "/" 
#define LTP_ACPI_DEVICE_NAME                DEV_PATH "/"  LTP_ACPI_DEV_NAME
#define MINOR_SHIFT_BITS 3 
#define MAX_PARTITIONS 8                    /* partition 0 + 7 more possible due to 3 bit partition number field */
#define	MAX_NUM_DISKS 3                   /* number of real devices */

#define MPDEV_FLAG_CLEAR 0
#define MPDEV_FLAG_SET   1
                       
typedef struct _ltpdev_cmd {
    u_int32_t     cmd;           // input - 0==recover, 1==fail
    u_int32_t     status;        // ouput - 0==success
} ltpdev_cmd_t;

typedef enum ltpdev_ioctl_cmds_s {
	/* version commands */
	LTP_IOCTL_NUMBER = 0x5500	
} ltpdev_ioctl_cmds_t;

// define the ioctl cmds
#define LTPDEV_CMD       _IOR( LTPMAJOR, LTP_IOCTL_NUMBER, ltpdev_cmd_t **)

