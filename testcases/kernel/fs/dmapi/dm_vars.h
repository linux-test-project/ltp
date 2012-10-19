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
 * FILE NAME	: dm_vars.h
 *
 * PURPOSE	: Define variation number bases for all DMAPI test cases
 */

/* session.c */
#define CREATE_SESSION_BASE		1000
#define DESTROY_SESSION_BASE		1050
#define GETALL_SESSIONS_BASE		1100
#define QUERY_SESSION_BASE		1150

/* handle.c */
#define PATH_TO_HANDLE_BASE		1200
#define FD_TO_HANDLE_BASE		1250
#define PATH_TO_FSHANDLE_BASE		1300
#define HANDLE_TO_FSHANDLE_BASE		1350
#define HANDLE_CMP_BASE			1400
#define HANDLE_FREE_BASE		1450
#define HANDLE_IS_VALID_BASE		1500
#define HANDLE_HASH_BASE		1550
#define HANDLE_TO_FSID_BASE		1600
#define HANDLE_TO_IGEN_BASE		1650
#define HANDLE_TO_INO_BASE		1700
#define MAKE_HANDLE_BASE		1750
#define MAKE_FSHANDLE_BASE		1800
#define HANDLE_TO_PATH_BASE		1850
#define SYNC_BY_HANDLE_BASE		1900

/* event_sn.c */
#define DIR_SYNC_NAMESP_EVENT_BASE	1950
#define FILE_SYNC_NAMESP_EVENT_BASE	2000

/* event_sd.c */
#define FILE_READ_DATA_EVENT_BASE	2050
#define FILE_WRITE_DATA_EVENT_BASE	2100
#define FILE_TRUNC_DATA_EVENT_BASE	2150

/* event_an.c */
#define DIR_ASYNC_NAMESP_EVENT_BASE	2200
#define FILE_ASYNC_NAMESP_EVENT_BASE	2250

/* pmr_pre.c */
#define SET_REGION_BASE			2300

/* pmr_post.c */
#define GET_REGION_BASE			2350

/* event_am.c */
#define DIR_ASYNC_META_EVENT_BASE	2400
#define FILE_ASYNC_META_EVENT_BASE	2450
#define SET_RETURN_ON_DESTROY_BASE	2500

/* hole.c */
#define GET_ALLOCINFO_BASE		2550
#define PROBE_HOLE_BASE			2600
#define PUNCH_HOLE_BASE			2650

/* invis.c */
#define READ_INVIS_BASE			2700
#define WRITE_INVIS_BASE		2750

/* attr.c */
#define SET_DMATTR_BASE			2800
#define GET_DMATTR_BASE			2850
#define REMOVE_DMATTR_BASE		2900
#define GETALL_DMATTR_BASE		2950
#define SET_FILEATTR_BASE		3000
#define GET_FILEATTR_BASE		3050
#define INIT_ATTRLOC_BASE		3100
#define GET_DIRATTRS_BASE		3150
#define SET_INHERIT_BASE		3200
#define CLEAR_INHERIT_BASE		3250
#define GETALL_INHERIT_BASE		3300

/* event_us.c */
#define CREATE_USEREVENT_BASE		3350
#define SEND_MSG_BASE			3400
#define FIND_EVENTMSG_BASE		3450

/* disp.c */
#define GET_CONFIG_EVENTS_BASE		3500
#define SET_DISP_BASE			3550
#define GETALL_DISP_BASE		3600
#define SET_EVENTLIST_BASE		3650
#define GET_EVENTLIST_BASE		3800

/* config.c */
#define GET_CONFIG_BASE			3850

/* objref.c */
#define OBJ_REF_HOLD_BASE		3900
#define OBJ_REF_RELE_BASE		3950
#define OBJ_REF_QUERY_BASE		4000

/* mount.c */
#define GET_MOUNTINFO_BASE		4050

/* token.c */
#define GETALL_TOKENS_BASE		4100

/* right.c */
#define REQUEST_RIGHT_BASE		4150
#define RELEASE_RIGHT_BASE		4200
#define QUERY_RIGHT_BASE		4250
#define UPGRADE_RIGHT_BASE		4300
#define DOWNGRADE_RIGHT_BASE		4350

/* mmap.c */
#define MMAP_READ_BASE			4400
#define MMAP_WRITE_BASE			4450

/* event.c */
#define GET_EVENTS_BASE			4500
#define RESPOND_EVENT_BASE		4550
#define MOVE_EVENT_BASE			4600
#define PENDING_BASE			4650
