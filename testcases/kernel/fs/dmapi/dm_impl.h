/*
 *   Copyright (c) International Business Machines  Corp., 2004
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
 * TEST CASE	: dm_impl.h
 *
 * PURPOSE	: Define implementation-dependent functions and variables
 * 		  common to all DMAPI test cases
 *
 * NOTES	: The validEvents[] table is derived from the XDSM
 * 		  specification, and then the row(s) for unsupported event(s)
 * 		  are set to DM_FALSE.  The definition from the specification
 * 		  is:
 *
 * eventValidity_t validEvents[DM_EVENT_MAX] = {
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_CANCEL" },
 *	{ DM_TRUE , DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_MOUNT" },
 *	{ DM_FALSE, DM_TRUE , DM_FALSE, DM_FALSE , "DM_EVENT_PREUNMOUNT" },
 *	{ DM_FALSE, DM_TRUE , DM_FALSE, DM_FALSE , "DM_EVENT_UNMOUNT" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_DEBUT" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_CREATE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_CLOSE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTCREATE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_REMOVE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTREMOVE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_RENAME" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTRENAME" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_LINK" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTLINK" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_SYMLINK" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTSYMLINK" },
 *	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_READ" },
 *	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_WRITE" },
 *	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_TRUNCATE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_ATTRIBUTE" },
 *	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_DESTROY" },
 *	{ DM_FALSE, DM_TRUE , DM_FALSE, DM_FALSE , "DM_EVENT_NOSPACE" },
 *	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_USER" }
 * };
 *
 */

#include <sys/mount.h>

#ifdef JFS
configResult_t dmimpl_expectedResults[CONFIG_MAX] = {
	{ "DM_CONFIG_INVALID", 0 },
	{ "DM_CONFIG_BULKALL", DM_FALSE },
	{ "DM_CONFIG_CREATE_BY_HANDLE", DM_FALSE },
	{ "DM_CONFIG_DTIME_OVERLOAD", DM_TRUE },
	{ "DM_CONFIG_LEGACY", DM_TRUE },
	{ "DM_CONFIG_LOCK_UPGRADE", DM_FALSE },
	{ "DM_CONFIG_MAX_ATTR_ON_DESTROY", 256 /* from dmapi_jfs.c */},
	{ "DM_CONFIG_MAX_ATTRIBUTE_SIZE", 0xFFFF /* from jfs_xattr.h */},
	{ "DM_CONFIG_MAX_HANDLE_SIZE", 56 /* from dmapi_kern.h */},
	{ "DM_CONFIG_MAX_MANAGED_REGIONS", 0x7FFFFFFF /* from dmapi_jfs.c */},
	{ "DM_CONFIG_MAX_MESSAGE_DATA", 3960 /* from dmapi_private.h */},
	{ "DM_CONFIG_OBJ_REF", DM_TRUE },
	{ "DM_CONFIG_PENDING", DM_TRUE }, // ?
	{ "DM_CONFIG_PERS_ATTRIBUTES", DM_TRUE },
	{ "DM_CONFIG_PERS_EVENTS", DM_FALSE },
	{ "DM_CONFIG_PERS_INHERIT_ATTRIBS", DM_FALSE },
	{ "DM_CONFIG_PERS_MANAGED_REGIONS", DM_TRUE },
	{ "DM_CONFIG_PUNCH_HOLE", DM_TRUE },
	{ "DM_CONFIG_TOTAL_ATTRIBUTE_SPACE", 0x7FFFFFFF /* from dmapi_jfs.c */},
	{ "DM_CONFIG_WILL_RETRY", DM_TRUE }
};

/* JFS does not support DM_EVENT_CANCEL or DM_EVENT_DEBUT */
eventValidity_t dmimpl_validEvents[DM_EVENT_MAX] = {
	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_CANCEL" },
	{ DM_TRUE , DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_MOUNT" },
	{ DM_FALSE, DM_TRUE , DM_FALSE, DM_FALSE , "DM_EVENT_PREUNMOUNT" },
	{ DM_FALSE, DM_TRUE , DM_FALSE, DM_FALSE , "DM_EVENT_UNMOUNT" },
	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_DEBUT" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_CREATE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_CLOSE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTCREATE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_REMOVE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTREMOVE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_RENAME" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTRENAME" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_LINK" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTLINK" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_SYMLINK" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_FALSE , "DM_EVENT_POSTSYMLINK" },
	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_READ" },
	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_WRITE" },
	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_TRUNCATE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_ATTRIBUTE" },
	{ DM_FALSE, DM_TRUE , DM_TRUE , DM_TRUE  , "DM_EVENT_DESTROY" },
	{ DM_FALSE, DM_TRUE , DM_FALSE, DM_FALSE , "DM_EVENT_NOSPACE" },
	{ DM_FALSE, DM_FALSE, DM_FALSE, DM_FALSE , "DM_EVENT_USER" }
};

dm_eventset_t dmimpl_eventset =
/*	(1 << DM_EVENT_CANCEL)		| */
	(1 << DM_EVENT_MOUNT)		|
	(1 << DM_EVENT_PREUNMOUNT)	|
	(1 << DM_EVENT_UNMOUNT)		|
/*	(1 << DM_EVENT_DEBUT) 		| */
	(1 << DM_EVENT_CREATE)		|
	(1 << DM_EVENT_CLOSE)		|
	(1 << DM_EVENT_POSTCREATE)	|
	(1 << DM_EVENT_REMOVE)		|
	(1 << DM_EVENT_POSTREMOVE)	|
	(1 << DM_EVENT_RENAME)		|
	(1 << DM_EVENT_POSTRENAME)	|
	(1 << DM_EVENT_LINK)		|
	(1 << DM_EVENT_POSTLINK)	|
	(1 << DM_EVENT_SYMLINK)		|
	(1 << DM_EVENT_POSTSYMLINK)	|
	(1 << DM_EVENT_READ)		|
	(1 << DM_EVENT_WRITE)		|
	(1 << DM_EVENT_TRUNCATE)	|
	(1 << DM_EVENT_ATTRIBUTE)	|
	(1 << DM_EVENT_DESTROY)		|
	(1 << DM_EVENT_NOSPACE)		|
	(1 << DM_EVENT_USER);

int dmimpl_mount(char **mountPt, char **deviceNm) {
	char options[FILENAME_MAX];

	if ((*mountPt = DMOPT_GET("mtpt")) == NULL) {
		DMLOG_PRINT(DMLVL_ERR, "Missing mount point, use -mtpt (for example, -mtpt /dmapidir)\n");
		DM_EXIT();
	} else {
		DMLOG_PRINT(DMLVL_DEBUG, "Mount point is %s\n", *mountPt);
	}

	if ((*deviceNm = DMOPT_GET("device")) == NULL) {
		DMLOG_PRINT(DMLVL_ERR, "Missing device name, use -device (for example, -device /dev/hda5)\n");
		DM_EXIT();
	} else {
		DMLOG_PRINT(DMLVL_DEBUG, "Device name is %s\n", *deviceNm);
	}

	sprintf(options, "dmapi,mtpt=%s", *mountPt);
	EVENT_DELIVERY_DELAY;
	DMLOG_PRINT(DMLVL_DEBUG, "Mounting %s on %s now...\n", *deviceNm, *mountPt);
	return mount(*deviceNm, *mountPt, "jfs", 0, options);
}
#endif
