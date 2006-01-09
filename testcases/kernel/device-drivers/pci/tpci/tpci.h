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


#define TPCI_TEST_DRIVER_NAME	"pci/pci-express test module"
#define TPCI_MAJOR      252
#define DEVICE_NAME		"/dev/tpci"
#define MAX_DEVFN		256
#define MAX_BUS			256
#define MAG_NUM 		'k'
#define AER_CAP_ID_VALUE	0x14011

#define PCI_PROBE		_IO(MAG_NUM, 1)
#define PCI_ENABLE		_IO(MAG_NUM, 2)
#define PCI_DISABLE		_IO(MAG_NUM, 3)
#define FIND_BUS	        _IO(MAG_NUM, 4)
#define FIND_DEVICE             _IO(MAG_NUM, 5)
#define FIND_CLASS              _IO(MAG_NUM, 6)
#define FIND_SUBSYS             _IO(MAG_NUM, 7)
#define BUS_SCAN		_IO(MAG_NUM, 8)
#define	SLOT_SCAN		_IO(MAG_NUM, 9)
#define ENABLE_BRIDGES		_IO(MAG_NUM, 10)
#define BUS_ADD_DEVICES		_IO(MAG_NUM, 11)
#define MATCH_DEVICE		_IO(MAG_NUM, 12)
#define REG_DRIVER		_IO(MAG_NUM, 13)
#define UNREG_DRIVER		_IO(MAG_NUM, 14)
#define BUS_RESOURCES		_IO(MAG_NUM, 15)
#define PCI_RESOURCES		_IO(MAG_NUM, 16)
#define SAVE_STATE		_IO(MAG_NUM, 19)
#define RESTORE_STATE		_IO(MAG_NUM, 20)
#define TEST_MAX_BUS		_IO(MAG_NUM, 21)
#define FIND_CAP		_IO(MAG_NUM, 22)
#define FIND_PCI_EXP_CAP	_IO(MAG_NUM, 23)
#define READ_PCI_EXP_CONFIG	_IO(MAG_NUM, 24)
#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev) ((dev)->owner = THIS_MODULE)
#endif

/*
 * structures for PCI test driver
 */
struct tpci_interface {
	int 	in_len;		// input data length
	caddr_t	in_data;	// input data
	int 	out_rc;		// return code from the test 
	int 	out_len;	// output data length 
	caddr_t	out_data;	// output data 
};
typedef struct tpci_interface tpci_interface_t;


