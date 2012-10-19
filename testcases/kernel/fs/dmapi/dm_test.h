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
 * FILE NAME	: dm_test.h
 *
 * PURPOSE	: Define macros and constants common to all DMAPI test cases
 */

#include <stdlib.h>
#if HAVE_SYS_JFSDMAPI_H
#include <sys/jfsdmapi.h>
#endif
#include "dm_vars.h"

/* The following constants are implementation-independent */
#define ABORT_ERRNO 12
#define DUMMY_STRING "0123456789"
#define DUMMY_STRING2 "9876543210"
#define DUMMY_STRLEN (sizeof(DUMMY_STRING)-1)
#define ATTR_NAME "DMAttr01"
#define ATTR_VALUE "This is a DM attribute's value"
#define ATTR_VALUELEN (sizeof(ATTR_VALUE)-1)
#define DUMMY_TIME 0xCAFEFEED
#define DUMMY_UID 0xDEAD
#define DUMMY_GID 0xBEEF
#define DUMMY_MODE 0xBAD
#define MODE_MASK 0x0FFF
#define CURRENT_DIR "."
#define PARENT_DIR ".."
#define MSG_DATA "This is the message data for a DM user event"
#define MSG_DATALEN sizeof(MSG_DATA)
#define DWALIGN(n) (((n)+(sizeof(int)-1)) & ~(sizeof(int)-1))

/* The following constants are more than likely implementation-dependent */
#define INVALID_ADDR 0xDEADBEEF
#define FILE_HANDLELEN 24
#define EVENT_DELIVERY_DELAY sleep(1)
#define EVENT_DELIVERY_DELAY_LOOP { \
	do { \
		eventReceived = DM_EVENT_INVALID; \
		EVENT_DELIVERY_DELAY; \
	} while (eventReceived != DM_EVENT_INVALID); \
}
#define TIMESTAMP_DELAY { \
	DMLOG_PRINT(DMLVL_DEBUG, "Sleeping to guarantee timestamp change...\n"); \
	sleep(3); \
}
#define PAGE_SIZE 4096
#define PAGEALIGN(n) (((n)+(PAGE_SIZE-1)) & ~(PAGE_SIZE-1))
#define O_DIRECTORY 0200000

/* The following constants are implementation-dependent */
#define DMAPI_ATTR_PREFIX "user.dmi."
#define PMR_ATTRNAME "system.dmi.persistent.regions"
#define MAXFILESIZE ((__s64)1 << 52)
#define ROOT_INODE 2
#define BLK_SIZE 4096
#define BLKALIGN(n) (((n)+(BLK_SIZE-1)) & ~(BLK_SIZE-1))
#define UNALIGNED_BLK_OFF 1357 /* Odd number less than BLK_SIZE */

/* The following constants are file/directory/link names */
#define DUMMY_FILE "dummy.txt"
#define DUMMY_FILE2 "dummy2.txt"
#define DUMMY_LINK "dummy.lnk"
#define DUMMY_SUBDIR "dummy.dir"
#define DUMMY_SUBDIR2 "dummy2.dir"
#define DUMMY_SUBDIR_FILE "dummy.dir/dummy.txt"
#define DUMMY_SUBDIR_LINK "dummy.dir/dummy.lnk"
#define DUMMY_SUBDIR_SUBDIR "dummy.dir/dummy.dir"
#define DUMMY_SUBDIR2_FILE "dummy2.dir/dummy.txt"
#define DUMMY_SUBDIR2_LINK "dummy2.dir/dummy.lnk"
#define DUMMY_SUBDIR2_SUBDIR "dummy2.dir/dummy.dir"
#define DUMMY_TMP "dummy.tmp"
#define DUMMY_FILE_RO_MODE 	(S_IRUSR | S_IRGRP | S_IROTH)
#define DUMMY_FILE_RW_MODE 	(DUMMY_FILE_RO_MODE | S_IWUSR)
#define DUMMY_DIR_RO_MODE 	(DUMMY_FILE_RO_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
#define DUMMY_DIR_RW_MODE	(DUMMY_DIR_RO_MODE | S_IWUSR)

/* The following constants and macros pertain to DM logging */
#define DMLVL_ERR	1
#define DMLVL_WARN	2
#define DMLVL_DEBUG	3
#define DMLVL_INFO	4

#define DMSTAT_PASS	1
#define DMSTAT_FAIL	2
#define DMSTAT_SKIP	3

#define DMLOG_START()	dm_StartLogging()
#define DMLOG_STOP()	dm_StopLogging()
#define DMLOG_PRINT	dm_LogPrintf
#define DMVAR_EXEC(v)	dm_ExecuteVariation(v)
#define DMVAR_PASS()	dm_PassVariation()
#define DMVAR_FAIL()	dm_FailVariation()
#define DMVAR_SKIP()	dm_SkipVariation()
#define DMOPT_PARSE(c,v)	dm_ParseCommandLineOptions(c, v)
#define DMOPT_GET(o)	dm_GetCommandLineOption(o)
#define DM_ERROR	dm_error
#define DM_EXIT()	exit(-1)
#define DMVAR_ENDPASSEXP(n,e,a)	dm_EndVariation_SuccessExpected(n, e, a)
#define DMVAR_ENDFAILEXP(n,e,a,f) dm_EndVariation_FailureExpected(n, e, a, f);
#define DMVAR_CHKPASSEXP(e1,a1,e2,a2) dm_CheckVariation_SuccessExpected(e1, a1, e2, a2)
#define DMVAR_CHKFAILEXP(e1,a1,e,e2,a2) dm_CheckVariation_FailureExpected(e1, a1, e, e2, a2)
#define DMVAR_END(s)	{ if ((s) == DMSTAT_PASS) DMVAR_PASS(); else DMVAR_FAIL(); }

/* DM logging global functions */
void dm_ParseCommandLineOptions(int argc, char **argv);
char *dm_GetCommandLineOption(char *option);
void dm_StartLogging(void);
void dm_StopLogging(void);
void dm_Error(char *format, ...);
void dm_LogPrintf(u_int level, char *format, ...);
int dm_ExecuteVariation(int var);
void dm_PassVariation(void);
void dm_FailVariation(void);
void dm_SkipVariation(void);
void dm_EndVariation_SuccessExpected(char *funcname, int expectedRC, int actualRC);
void dm_EndVariation_FailureExpected(char *funcname, int expectedRC, int actualRC, int expectedErrno);
#if HAVE_SYS_JFSDMAPI_H
int dm_CheckVariation_SuccessExpected(int expectedRC, int actualRC, dm_eventtype_t expectedEvent, dm_eventtype_t actualEvent);
int dm_CheckVariation_FailureExpected(int expectedRC, int actualRC, int expectedErrno, dm_eventtype_t expectedEvent, dm_eventtype_t actualEvent);
void dm_LogHandle(char *hdl, int len);

/* Persistent managed regions global data */

#ifdef MULTIPLE_REGIONS
#define PMR_NUM_REGIONS 5
#else
#define PMR_NUM_REGIONS 1
#endif
extern dm_region_t dm_PMR_regbuf[];

/* Implementation-dependent data structures */
typedef struct configResult {
	char		*name;
	dm_size_t	result;
} configResult_t;

typedef struct eventValidity {
	dm_boolean_t bGlobalHandle;
	dm_boolean_t bFSHandle;
	dm_boolean_t bDirHandle;
	dm_boolean_t bFileHandle;
	char *       name;
} eventValidity_t;

/* Implementation-dependent global function */
int dmimpl_mount(char **mountPt, char **deviceNm);

/* Implementation-dependent global data */
#define CONFIG_MAX 20
extern configResult_t dmimpl_expectedResults[];
extern eventValidity_t dmimpl_validEvents[];
extern dm_eventset_t dmimpl_eventset;
#endif
