/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define PCI_DEVICE_NAME		"ltp_tpci"
#define MAX_DEVFN		256
#define MAX_BUS			256

enum PCI_TCASES {
	PCI_DISABLE = 0,
	PCI_ENABLE,
	FIND_BUS,
	FIND_DEVICE,
	FIND_CLASS,
	FIND_SUBSYS,
	BUS_SCAN,
	SLOT_SCAN,
	ENABLE_BRIDGES,
	BUS_ADD_DEVICES,
	MATCH_DEVICE,
	REG_DRIVER,
	UNREG_DRIVER,
	PCI_RESOURCES,
	SAVE_STATE,
	RESTORE_STATE,
	FIND_CAP,
	PCI_EXP_CAP_CONFIG,
	PCI_TCASES_NUM,
};
