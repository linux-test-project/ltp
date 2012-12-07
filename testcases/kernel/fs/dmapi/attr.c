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
 * TEST CASE	: attr.c - DMAPI attributes
 *
 * VARIATIONS	: 204
 *
 * API'S TESTED	: dm_set_dmattr
 * 		  dm_get_dmattr
 * 		  dm_remove_dmattr
 * 		  dm_getall_dmattr
 * 		  dm_set_fileattr
 * 		  dm_get_fileattr
 * 		  dm_get_dirattrs
 * 		  dm_set_inherit
 * 		  dm_clear_inherit
 * 		  dm_getall_inherit
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include "dm_test.h"

#define TMP_FILELEN 1000
#define ATTR_NAME2 "DMAttr02"
#define NUM_ATTRS 15
#define ATTR_SMALLLEN (DWALIGN(sizeof(dm_stat_t)+1) + DWALIGN(sizeof(DUMMY_FILE)+1))
#define MIN_ENTRYLEN (DWALIGN(sizeof(dm_stat_t)+1) + DWALIGN(sizeof(CURRENT_DIR)+1))
#define DM_AT_ALL_DIRATTRS (DM_AT_DTIME|DM_AT_HANDLE|DM_AT_EMASK|DM_AT_PMANR|DM_AT_PATTR|DM_AT_STAT|DM_AT_CFLAG)
#define NON_DM_ATTR_NAME "user.non-dm.attr"
#define NON_DM_ATTR_VALUE "This is a non-DM attribute's value"

/*
 * DIRENTS_FILES is very implementation-dependent, and is the number of files
 * that will just fill up the buffer passed to jfs_get_dirents; NUM_FILES
 * should be at least 3 times DIRENTS_FILES; ATTR_LISTLEN needs to be large
 * enough to contain 3 files (., .. and dummy.txt) worth of information while
 * ATTR_BIGLISTLEN needs to be large enough to contain NUM_FILES files worth
 * of information
 */
#define DIRENTS_FILES 5
#define NUM_FILES 15
#define ATTR_LISTLEN 1000
#define ATTR_BIGLISTLEN 10000

char command[4096];
char fullAttrName[32];
dm_sessid_t sid;
dm_size_t maxAttrSize;
dm_size_t persInheritAttr;

void LogDmStat(dm_stat_t * statdm)
{

	DMLOG_PRINT(DMLVL_DEBUG, "    dt_dev %d\n", statdm->dt_dev);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_ino %d\n", statdm->dt_ino);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_mode 0x%x\n", statdm->dt_mode);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_nlink %d\n", statdm->dt_nlink);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_uid %d\n", statdm->dt_uid);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_gid %d\n", statdm->dt_gid);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_rdev %d\n", statdm->dt_rdev);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_size %lld\n", statdm->dt_size);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_blksize %d\n", statdm->dt_blksize);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_blocks %d\n", statdm->dt_blocks);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_atime %d\n", statdm->dt_atime);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_mtime %d\n", statdm->dt_mtime);
	DMLOG_PRINT(DMLVL_DEBUG, "    dt_ctime %d\n", statdm->dt_ctime);

}

void LogDmAttrs(dm_attrlist_t * attrlist)
{

	int i = 0;
	dm_attrlist_t *attr = attrlist;

	while (attr != NULL) {
		DMLOG_PRINT(DMLVL_DEBUG, "  dmattr %d: name %.*s, value %.*s\n",
			    i++, DM_ATTR_NAME_SIZE, attr->al_name.an_chars,
			    DM_GET_LEN(attr, al_data), DM_GET_VALUE(attr,
								    al_data,
								    char *));
		attr = DM_STEP_TO_NEXT(attr, dm_attrlist_t *);
	}
}

void LogDirAttrs(void *attrlist, u_int mask)
{
	int i = 0;
	dm_stat_t *stat = (dm_stat_t *) attrlist;

	while (stat != NULL) {
		DMLOG_PRINT(DMLVL_DEBUG, "  dirattr %d:\n", i++);
		DMLOG_PRINT(DMLVL_DEBUG, "    dt_compname: %s\n",
			    DM_GET_VALUE(stat, dt_compname, char *));
		if (mask & DM_AT_HANDLE)
			dm_LogHandle(DM_GET_VALUE(stat, dt_handle, void *),
				     DM_GET_LEN(stat, dt_handle));
		if (mask & DM_AT_EMASK)
			DMLOG_PRINT(DMLVL_DEBUG, "    dt_emask: %x\n",
				    stat->dt_emask);
		if (mask & DM_AT_PMANR)
			DMLOG_PRINT(DMLVL_DEBUG, "    dt_pmanreg: %s\n",
				    stat->dt_pmanreg ? "DM_TRUE" : "DM_FALSE");
		if (mask & DM_AT_PATTR)
			DMLOG_PRINT(DMLVL_DEBUG, "    dt_pers: %s\n",
				    stat->dt_pers ? "DM_TRUE" : "DM_FALSE");
		if (mask & DM_AT_DTIME)
			DMLOG_PRINT(DMLVL_DEBUG, "    dt_dtime: %d\n",
				    stat->dt_dtime);
		if (mask & DM_AT_CFLAG)
			DMLOG_PRINT(DMLVL_DEBUG, "    dt_change: %d\n",
				    stat->dt_change);
		if (mask & DM_AT_STAT)
			LogDmStat(stat);

		stat = DM_STEP_TO_NEXT(stat, dm_stat_t *);
	}
}

dm_stat_t *GetDirEntry(void *attrlist, char *compname)
{

	dm_stat_t *stat = (dm_stat_t *) attrlist;

	while (stat != NULL) {
		if (strcmp(DM_GET_VALUE(stat, dt_compname, char *), compname) ==
		    0)
			return stat;
		stat = DM_STEP_TO_NEXT(stat, dm_stat_t *);
	}
	return NULL;

}

dm_stat_t *GetLastDirEntry(void *attrlist)
{

	dm_stat_t *stat = (dm_stat_t *) attrlist;
	dm_stat_t *laststat = NULL;

	while (stat != NULL) {
		laststat = stat;
		stat = DM_STEP_TO_NEXT(stat, dm_stat_t *);
	}
	return laststat;

}

int GetNumDirEntry(void *attrlist)
{

	dm_stat_t *stat = (dm_stat_t *) attrlist;
	int i = 0;

	while (stat != NULL) {
		i++;
		stat = DM_STEP_TO_NEXT(stat, dm_stat_t *);
	}
	return i;

}

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int i;
	int rc;
	char *szSessionInfo = "dm_test session info";

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	/* CANNOT DO ANYTHING WITHOUT SUCCESSFUL INITIALIZATION!!! */
	if ((rc = dm_init_service(&varstr)) != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_init_service failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else if ((rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &sid))
		   != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_create_session failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else {
		int fd;
		void *fshanp;
		size_t fshlen;

		fd = open(DUMMY_TMP, O_RDWR | O_CREAT | O_TRUNC,
			  DUMMY_FILE_RW_MODE);
		if (fd != -1) {
			for (i = 0; i < (TMP_FILELEN / DUMMY_STRLEN); i++) {
				if (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
				    DUMMY_STRLEN) {
					rc = -1;
					break;
				}
			}
		} else {
			rc = -1;
		}
		if (rc == 0) {
			rc = close(fd);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "creating dummy file failed! (rc = %d, errno = %d)\n",
				    rc, errno);
			dm_destroy_session(sid);
			DM_EXIT();
		}

		rc = dm_path_to_fshandle(DUMMY_TMP, &fshanp, &fshlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "dm_path_to_fshandle! (rc = %d, errno = %d)\n",
				    rc, errno);
			dm_destroy_session(sid);
			DM_EXIT();
		}

		rc = dm_get_config(fshanp, fshlen, DM_CONFIG_MAX_ATTRIBUTE_SIZE,
				   &maxAttrSize);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "dm_get_config failed! (rc = %d, errno = %d)\n",
				    rc, errno);
			dm_handle_free(fshanp, fshlen);
			dm_destroy_session(sid);
			DM_EXIT();
		}

		rc = dm_get_config(fshanp, fshlen,
				   DM_CONFIG_PERS_INHERIT_ATTRIBS,
				   &persInheritAttr);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "dm_get_config failed! (rc = %d, errno = %d)\n",
				    rc, errno);
			dm_handle_free(fshanp, fshlen);
			dm_destroy_session(sid);
			DM_EXIT();
		}

		dm_handle_free(fshanp, fshlen);

		sprintf(fullAttrName, "%s%s", DMAPI_ATTR_PREFIX, ATTR_NAME);

		/* Clean up any possible leftovers that could get in the way */
		remove(DUMMY_SUBDIR_FILE);
		unlink(DUMMY_SUBDIR_LINK);
		rmdir(DUMMY_SUBDIR_SUBDIR);
		remove(DUMMY_FILE);
		remove(DUMMY_FILE2);
		unlink(DUMMY_LINK);
		rmdir(DUMMY_SUBDIR);
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI attribute tests\n");

	szFuncName = "dm_set_dmattr";

	/*
	 * TEST    : dm_set_dmattr - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_set_dmattr(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, &attrname, 0,
					   sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, &attrname, 0,
					   sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, INVALID_ADDR,
					   &attrname, 0, sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - invalid attrnamep
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #10 (0 return code from strnlen_user
	 * ignored, which indicated fault)
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrnamep)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   (dm_attrname_t *) INVALID_ADDR, 0,
					   sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid buflen)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, INVALID_ADDR, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf),
					   (void *)INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - empty attrname
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(empty attrname)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - zero buflen
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN], value[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(zero buflen)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, 0, NULL);
			if (rc == 0) {
				if ((rc =
				     getxattr(DUMMY_FILE, fullAttrName, value,
					      sizeof(value))) == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected attribute length (%d vs %d)\n",
						    szFuncName, 0, 0, rc);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - maximum buflen
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char *buf, *value;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((buf = malloc(maxAttrSize)) == NULL) {
			/* No clean up */
		} else if ((memset(buf, '1', maxAttrSize) == NULL) ||
			   ((value = malloc(maxAttrSize)) == NULL)) {
			free(buf);
		} else if ((memset(value, 0, maxAttrSize) == NULL) ||
			   ((rc = system(command)) == -1)) {
			free(value);
			free(buf);
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
			free(value);
			free(buf);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			free(value);
			free(buf);
		}
		if (fd == -1 || rc == -1 || buf == NULL || value == NULL) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(max buflen)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, maxAttrSize, buf);
			if (rc == 0) {
				if ((rc =
				     getxattr(DUMMY_FILE, fullAttrName, value,
					      maxAttrSize)) == maxAttrSize) {
					if (memcmp(buf, value, maxAttrSize) ==
					    0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected attribute value (%s vs %s)\n",
							    szFuncName, 0, buf,
							    value);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected attribute length (%d vs %d)\n",
						    szFuncName, 0, maxAttrSize,
						    rc);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			free(buf);
			free(value);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - buflen too big
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char *buf;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((buf = malloc(maxAttrSize + 1)) == NULL) {
			/* No clean up */
		} else if ((memset(buf, '2', maxAttrSize + 1) == NULL) ||
			   ((rc = system(command)) == -1)) {
			free(buf);
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
			free(buf);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			free(buf);
		}
		if (fd == -1 || rc == -1 || buf == NULL) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(buflen too big)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, maxAttrSize + 1, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			free(buf);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - one file attribute, setdtime zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 12)) {
		int fd;
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(one file attr, setdtime zero)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime == statfs2.st_ctime)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and dtime unmodified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime modified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - one file attribute, setdtime non-zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 13)) {
		int fd;
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(one file attr, setdtime non-zero)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 1, sizeof(buf), buf);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime != statfs2.st_ctime)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and dtime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - two file attributes
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		int buf2 = INVALID_ADDR;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(two file attr)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			if (rc == 0) {
				memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
				memcpy(attrname.an_chars, ATTR_NAME2,
				       DM_ATTR_NAME_SIZE);
				rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
						   &attrname, 0, sizeof(buf2),
						   (void *)&buf2);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - multiple file attributes
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 15)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(multiple file attr)\n",
				    szFuncName);
			for (i = 1; (i <= NUM_ATTRS) && (rc == 0); i++) {
				memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
				memcpy(attrname.an_chars, ATTR_NAME,
				       DM_ATTR_NAME_SIZE);
				attrname.an_chars[DM_ATTR_NAME_SIZE - 2] =
				    '0' + (i / 10);
				attrname.an_chars[DM_ATTR_NAME_SIZE - 1] =
				    '0' + (i % 10);
				DMLOG_PRINT(DMLVL_DEBUG, "%s(%.*s)\n",
					    szFuncName, DM_ATTR_NAME_SIZE,
					    attrname.an_chars);
				rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
						   &attrname, 0, sizeof(buf),
						   buf);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - replace file attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 16)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN], value[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(attrname), &attrname)) == -1)
			||
			((rc =
			  (getxattr
			   (DUMMY_FILE, fullAttrName, value,
			    sizeof(value)) == sizeof(attrname)) ? 0 : -1) == -1)
			||
			((rc =
			  (memcmp(&attrname, value, sizeof(attrname)) ==
			   0) ? 0 : -1) == -1)) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(replace file attr)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			if (rc == 0) {
				if ((rc =
				     getxattr(DUMMY_FILE, fullAttrName, value,
					      sizeof(value))) ==
				    ATTR_VALUELEN) {
					if (memcmp(buf, value, sizeof(buf)) ==
					    0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected attribute value (%s vs %s)\n",
							    szFuncName, 0, buf,
							    value);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected attribute length (%d vs %d)\n",
						    szFuncName, 0,
						    ATTR_VALUELEN, rc);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - one directory attribute, setdtime zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 17)) {
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			rmdir(DUMMY_SUBDIR);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(one dir attr, setdtime zero)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime == statfs2.st_ctime)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and dtime unmodified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime modified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - one directory attribute, setdtime non-zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 18)) {
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			rmdir(DUMMY_SUBDIR);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(one dir attr, setdtime non-zero)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 1, sizeof(buf), buf);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime != statfs2.st_ctime)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and dtime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 19)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 20)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_set_dmattr(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, &attrname, 0,
					   sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_dmattr - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 21)) {
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_set_dmattr(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_NO_TOKEN, &attrname, 0, sizeof(buf), buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_set_dmattr - invalidated hanp
	 * EXPECTED: rc = -1, errno = BADF
	 */
	if (DMVAR_EXEC(SET_DMATTR_BASE + 22)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_get_dmattr";

	/*
	 * TEST    : dm_get_dmattr - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_get_dmattr(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, &attrname, sizeof(buf),
					   buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, &attrname, sizeof(buf),
					   buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, INVALID_ADDR,
					   &attrname, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid attrnamep
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrnamep)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   (dm_attrname_t *) INVALID_ADDR,
					   sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid buflen)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 1, buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);
			DMLOG_PRINT(DMLVL_DEBUG, "rlen %d\n");

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #13 (attrname not null-terminated)
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf),
					   (void *)INVALID_ADDR, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG, "rc = %d, %s", rc, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rlenp)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf,
					   (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - zero buflen, zero attribute length
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, 0, NULL)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(zero buflen, zero attr len)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, NULL, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - zero buflen, non-zero attribute length
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(zero buflen, non-zero attr len)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, NULL, &rlen);
			if (rc == -1) {
				if (errno == E2BIG) {
					DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n",
						    rlen);
					if (rlen == ATTR_VALUELEN) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and expected errno = %d\n",
							    szFuncName, -1,
							    E2BIG);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and expected errno = %d but unexpected rlen (%d vs %d)\n",
							    szFuncName, -1,
							    E2BIG, rlen,
							    ATTR_VALUELEN);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected errno = %d\n",
						    szFuncName, -1, E2BIG);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d\n",
					    szFuncName, -1);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - attribute not exist
	 * EXPECTED: rc = -1, errno = ENOENT
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(attr not exist)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - buf too small
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(buflen too small)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf) - 1, buf,
					   &rlen);
			if (rc == -1 && errno == E2BIG) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
			}
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 13)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				if (rlen == ATTR_VALUELEN) {
					if (memcmp
					    (buf, ATTR_VALUE,
					     ATTR_VALUELEN) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and rlen = %d but unexpected buf %s",
							    szFuncName, 0, rlen,
							    buf);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen = %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - maximum buflen
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char *buf;
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((buf = malloc(maxAttrSize)) == NULL) {
			/* No clean up */
		} else if ((memset(buf, '3', maxAttrSize) == NULL) ||
			   ((rc = system(command)) == -1)) {
			free(buf);
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			free(buf);
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			free(buf);
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, maxAttrSize, buf)) == -1) {
			free(buf);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || buf == NULL) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(max buflen)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, maxAttrSize, buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				if (rlen == maxAttrSize) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen (%d vs %d)\n",
						    szFuncName, 0, rlen,
						    maxAttrSize);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			free(buf);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 15)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				if (rlen == ATTR_VALUELEN) {
					if (memcmp
					    (buf, ATTR_VALUE,
					     ATTR_VALUELEN) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and rlen = %d but unexpected buf %s",
							    szFuncName, 0, rlen,
							    buf);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen = %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 16)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 17)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_get_dmattr(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, &attrname, sizeof(buf),
					   buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dmattr - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 18)) {
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_get_dmattr(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_NO_TOKEN, &attrname, sizeof(buf), buf,
				   &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_dmattr - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_DMATTR_BASE + 19)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc == remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_get_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_remove_dmattr";

	/*
	 * TEST    : dm_remove_dmattr - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_remove_dmattr(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, 0, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, (void *)INVALID_ADDR, hlen,
					      DM_NO_TOKEN, 0, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, 0, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, INVALID_ADDR, 0,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - invalid attrnamep
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrnamep)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      (dm_attrname_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - attribute not exist
	 * EXPECTED: rc = -1, errno = ENOENT
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(attr not exist)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - file attribute, setdtime zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 7)) {
		int fd;
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file attr, setdtime zero)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      &attrname);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if (((rc =
				      dm_get_dmattr(sid, hanp, hlen,
						    DM_NO_TOKEN, &attrname,
						    sizeof(buf), buf,
						    &rlen)) == -1)
				    && (errno == ENOENT)) {
					if ((rc2 == 0)
					    && (statfs1.st_ctime ==
						statfs2.st_ctime)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and dtime unmodified\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but dtime modified (%d vs %d)\n",
							    szFuncName, 0,
							    statfs1.st_ctime,
							    statfs2.st_ctime);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but attr still exist (errno = %d)\n",
						    szFuncName, 0, errno);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - file attribute, setdtime non-zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 8)) {
		int fd;
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file attr, setdtime non-zero)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 1,
					      &attrname);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if (((rc =
				      dm_get_dmattr(sid, hanp, hlen,
						    DM_NO_TOKEN, &attrname,
						    sizeof(buf), buf,
						    &rlen)) == -1)
				    && (errno == ENOENT)) {
					if ((rc2 == 0)
					    && (statfs1.st_ctime !=
						statfs2.st_ctime)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and dtime modified\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but dtime unmodified (%d vs %d)\n",
							    szFuncName, 0,
							    statfs1.st_ctime,
							    statfs2.st_ctime);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but attr still exist (errno = %d)\n",
						    szFuncName, 0, errno);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - directory attribute, setdtime zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 9)) {
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir attr, setdtime zero)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      &attrname);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if (((rc =
				      dm_get_dmattr(sid, hanp, hlen,
						    DM_NO_TOKEN, &attrname,
						    sizeof(buf), buf,
						    &rlen)) == -1)
				    && (errno == ENOENT)) {
					if ((rc2 == 0)
					    && (statfs1.st_ctime ==
						statfs2.st_ctime)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and dtime unmodified\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but dtime modified (%d vs %d)\n",
							    szFuncName, 0,
							    statfs1.st_ctime,
							    statfs2.st_ctime);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but attr still exist (errno = %d)\n",
						    szFuncName, 0, errno);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - directory attribute, setdtime non-zero
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 10)) {
		int rc2;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		size_t rlen;
		struct stat statfs1, statfs2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			TIMESTAMP_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir attr, setdtime non-zero)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 1,
					      &attrname);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if (((rc =
				      dm_get_dmattr(sid, hanp, hlen,
						    DM_NO_TOKEN, &attrname,
						    sizeof(buf), buf,
						    &rlen)) == -1)
				    && (errno == ENOENT)) {
					if ((rc2 == 0)
					    && (statfs1.st_ctime !=
						statfs2.st_ctime)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and dtime modified\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but dtime unmodified (%d vs %d)\n",
							    szFuncName, 0,
							    statfs1.st_ctime,
							    statfs2.st_ctime);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but attr still exist (errno = %d)\n",
						    szFuncName, 0, errno);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 11)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_remove_dmattr(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, 0, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_remove_dmattr - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 13)) {
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_remove_dmattr(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      DM_NO_TOKEN, 0, &attrname);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_remove_dmattr - invalidated handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(REMOVE_DMATTR_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated handle)\n",
				    szFuncName);
			rc = dm_remove_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_getall_dmattr";

	/*
	 * TEST    : dm_getall_dmattr - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_getall_dmattr(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, (void *)INVALID_ADDR, hlen,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, INVALID_ADDR,
					      sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(buflen too small)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN, 1,
					      buf, &rlen);
			if (rc == -1 && errno == E2BIG) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
			}
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), (void *)INVALID_ADDR,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rlenp)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf,
					      (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - no file attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(no file attr)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				if (rlen == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - one file attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, ATTR_VALUELEN, buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one file attr)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				LogDmAttrs((dm_attrlist_t *) buf);
				if (rlen ==
				    DWALIGN(sizeof(dm_attrlist_t) +
					    ATTR_VALUELEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d) %d\n",
					    szFuncName, rc, errno, rlen);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - two file attributes
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, ATTR_VALUELEN, buf)) == -1)
			|| (memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE) ==
			    NULL)
			||
			(memcpy
			 (attrname.an_chars, ATTR_NAME2,
			  DM_ATTR_NAME_SIZE) == NULL)
			||
			((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, ATTR_VALUELEN, buf)) == -1)) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(two file attr)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				LogDmAttrs((dm_attrlist_t *) buf);
				if (rlen ==
				    2 * DWALIGN(sizeof(dm_attrlist_t) +
						ATTR_VALUELEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d) %d\n",
					    szFuncName, rc, errno, rlen);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - multiple file attributes
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		size_t len;
		size_t totlen;
		int i;
		void *totbuf;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		for (i = 0, len = 0, totlen = 0; i < NUM_ATTRS && rc == 0; i++) {
			memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
			memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
			attrname.an_chars[DM_ATTR_NAME_SIZE - 2] =
			    '0' + (i / 10);
			attrname.an_chars[DM_ATTR_NAME_SIZE - 1] =
			    '0' + (i % 10);
			memcpy(buf + len, DUMMY_STRING, DUMMY_STRLEN);
			len += DUMMY_STRLEN;
			totlen += DWALIGN(len + sizeof(dm_attrlist_t));
			rc = dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, len, buf);
		}
		if (rc != -1) {
			if ((totbuf = malloc(totlen)) == NULL) {
				close(fd);
				remove(DUMMY_FILE);
			}
		}
		if (fd == -1 || rc == -1 || totbuf == NULL) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(%d file attr)\n",
				    szFuncName, NUM_ATTRS);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      totlen, totbuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				LogDmAttrs((dm_attrlist_t *) totbuf);
				if (rlen == totlen) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d) %d\n",
					    szFuncName, rc, errno, rlen);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			free(totbuf);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - one file attribute with non-DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, ATTR_VALUELEN, buf)) == -1)
			||
			((rc =
			  setxattr(DUMMY_FILE, NON_DM_ATTR_NAME,
				   NON_DM_ATTR_VALUE, sizeof(NON_DM_ATTR_VALUE),
				   0)) == -1)) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(one file attr with non-DM attr)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				LogDmAttrs((dm_attrlist_t *) buf);
				if (rlen ==
				    DWALIGN(sizeof(dm_attrlist_t) +
					    ATTR_VALUELEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d) %d\n",
					    szFuncName, rc, errno, rlen);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - one directory attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 13)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No claen up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, ATTR_VALUELEN, buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one dir attr)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				LogDmAttrs((dm_attrlist_t *) buf);
				if (rlen ==
				    DWALIGN(sizeof(dm_attrlist_t) +
					    ATTR_VALUELEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, 0, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d) %d\n",
					    szFuncName, rc, errno, rlen);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 14)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 15)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_getall_dmattr(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_dmattr - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 16)) {
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_getall_dmattr(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      DM_NO_TOKEN, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_getall_dmattr - invalidated handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GETALL_DMATTR_BASE + 17)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, ATTR_VALUELEN, buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated handle)\n",
				    szFuncName);
			rc = dm_getall_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_set_fileattr";

	/*
	 * TEST    : dm_set_fileattr - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_set_fileattr(INVALID_ADDR, hanp, hlen,
					     DM_NO_TOKEN, DM_AT_UID, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, (void *)INVALID_ADDR, hlen,
					     DM_NO_TOKEN, DM_AT_UID, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, INVALID_ADDR,
					     DM_NO_TOKEN, DM_AT_UID, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, INVALID_ADDR,
					     DM_AT_UID, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - invalid mask
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #20 (0 returned instead of -1 and
	 * EINVAL errno)
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid mask)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_HANDLE, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - invalid attrp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrp)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_UID,
					     (dm_fileattr_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_ATIME on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_atime = DUMMY_TIME;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_ATIME)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_ATIME, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_atime != statfs2.st_atime)
				    && (statfs2.st_atime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and atime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but atime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_atime,
						    statfs2.st_atime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_MTIME on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_mtime = DUMMY_TIME;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_MTIME)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_MTIME, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_mtime != statfs2.st_mtime)
				    && (statfs2.st_mtime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and mtime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but mtime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_mtime,
						    statfs2.st_mtime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_CTIME on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_ctime = DUMMY_TIME;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_CTIME)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_CTIME, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime != statfs2.st_ctime)
				    && (statfs2.st_ctime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and ctime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but ctime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_DTIME on file with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		fileattr.fa_dtime = DUMMY_TIME;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, ATTR_VALUELEN, buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_DTIME with attr)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime != statfs2.st_ctime)
				    && (statfs2.st_ctime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and ctime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but ctime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_DTIME on file without DM attr
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #21 (dtime updated without any DM
	 * attributes)
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_dtime = DUMMY_TIME;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_DTIME without attr)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime == statfs2.st_ctime)
				    && (statfs2.st_ctime != DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and ctime unmodified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but ctime modified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d\n",
					    szFuncName, rc);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_UID on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_UID)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_UID, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_uid != statfs2.st_uid)
				    && (statfs2.st_uid == DUMMY_UID)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and uid modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but uid unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_uid,
						    statfs2.st_uid);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_GID on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 13)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_gid = DUMMY_GID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_GID)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_GID, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_gid != statfs2.st_gid)
				    && (statfs2.st_gid == DUMMY_GID)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and gid modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but gid unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_gid,
						    statfs2.st_gid);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_MODE on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_mode = DUMMY_MODE;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_MODE)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_MODE, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_mode != statfs2.st_mode)
				    && ((statfs2.st_mode & MODE_MASK) ==
					DUMMY_MODE)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and mode modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but mode unmodified (%x vs %x)\n",
						    szFuncName, 0,
						    statfs1.st_mode,
						    statfs2.st_mode);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_SIZE on file, shrink
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 15)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_size = TMP_FILELEN / 2;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_SIZE, shrink)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_SIZE, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_size != statfs2.st_size)
				    && (statfs2.st_size == TMP_FILELEN / 2)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and size modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but size unmodified (%x vs %x)\n",
						    szFuncName, 0,
						    statfs1.st_size,
						    statfs2.st_size);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_SIZE on file, expand
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 16)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_size = TMP_FILELEN * 2;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_SIZE, expand)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_SIZE, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_size != statfs2.st_size)
				    && (statfs2.st_size == TMP_FILELEN * 2)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and size modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but size unmodified (%x vs %x)\n",
						    szFuncName, 0,
						    statfs1.st_size,
						    statfs2.st_size);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 17)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_FILE, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_UID, &fileattr);
			rc2 |= stat(DUMMY_FILE, &statfs2);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_ATIME on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 18)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_atime = DUMMY_TIME;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_ATIME)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_ATIME, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_atime != statfs2.st_atime)
				    && (statfs2.st_atime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and atime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but atime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_atime,
						    statfs2.st_atime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_MTIME on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 19)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_mtime = DUMMY_TIME;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_MTIME)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_MTIME, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_mtime != statfs2.st_mtime)
				    && (statfs2.st_mtime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and mtime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but mtime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_mtime,
						    statfs2.st_mtime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_CTIME on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 20)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_ctime = DUMMY_TIME;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_CTIME)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_CTIME, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime != statfs2.st_ctime)
				    && (statfs2.st_ctime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and ctime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but ctime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_DTIME on directory with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 21)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		dm_attrname_t attrname;
		char buf[ATTR_LISTLEN];
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		fileattr.fa_dtime = DUMMY_TIME;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, ATTR_VALUELEN, buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir DM_AT_DTIME with attr)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime != statfs2.st_ctime)
				    && (statfs2.st_ctime == DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and ctime modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but ctime unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_DTIME on directory without DM attribute
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 22)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_dtime = DUMMY_TIME;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir DM_AT_DTIME without attr)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_ctime == statfs2.st_ctime)
				    && (statfs2.st_ctime != DUMMY_TIME)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and ctime unmodified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but ctime modified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_ctime,
						    statfs2.st_ctime);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d\n",
					    szFuncName, rc);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_UID on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 23)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_UID)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_UID, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_uid != statfs2.st_uid)
				    && (statfs2.st_uid == DUMMY_UID)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and uid modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but uid unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_uid,
						    statfs2.st_uid);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_GID on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 24)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_gid = DUMMY_GID;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_GID)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_GID, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_gid != statfs2.st_gid)
				    && (statfs2.st_gid == DUMMY_GID)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and gid modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but gid unmodified (%d vs %d)\n",
						    szFuncName, 0,
						    statfs1.st_gid,
						    statfs2.st_gid);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_AT_MODE on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 25)) {
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;
		struct stat statfs1, statfs2;
		int rc2;

		/* Variation set up */
		fileattr.fa_mode = DUMMY_MODE;
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			rc2 = stat(DUMMY_SUBDIR, &statfs1);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_MODE)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_MODE, &fileattr);
			rc2 |= stat(DUMMY_SUBDIR, &statfs2);
			if (rc == 0) {
				if ((rc2 == 0)
				    && (statfs1.st_mode != statfs2.st_mode)
				    && ((statfs2.st_mode & MODE_MASK) ==
					DUMMY_MODE)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and mode modified\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but mode unmodified (%x vs %x)\n",
						    szFuncName, 0,
						    statfs1.st_mode,
						    statfs2.st_mode);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 26)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_set_fileattr(DM_NO_SESSION, hanp, hlen,
					     DM_NO_TOKEN, DM_AT_UID, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_fileattr - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 27)) {
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_set_fileattr(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				     DM_NO_TOKEN, DM_AT_UID, &fileattr);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_set_fileattr - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_FILEATTR_BASE + 28)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_fileattr_t fileattr;

		/* Variation set up */
		fileattr.fa_uid = DUMMY_UID;
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_set_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_UID, &fileattr);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_get_fileattr";

	/*
	 * TEST    : dm_get_fileattr - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_get_fileattr(INVALID_ADDR, hanp, hlen,
					     DM_NO_TOKEN, DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, (void *)INVALID_ADDR, hlen,
					     DM_NO_TOKEN, DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, INVALID_ADDR,
					     DM_NO_TOKEN, DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, INVALID_ADDR,
					     DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - invalid mask
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #22 (0 returned instead of -1 and
	 * EINVAL errno)
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid mask)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_HANDLE, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - invalid statp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid statp)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK,
					     (dm_stat_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_EMASK on file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_eventset_t eventset;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, eventset);
		DMEV_SET(DM_EVENT_CLOSE, eventset);
		DMEV_SET(DM_EVENT_DESTROY, eventset);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					  &eventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_EMASK)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &stat);
			if (rc == 0) {
				if (memcmp
				    (&eventset, &stat.dt_emask,
				     sizeof(dm_eventset_t)) == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected emask (%llx vs %llx)\n",
						    szFuncName, 0, eventset,
						    stat.dt_emask);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(eventset);
			rc |=
			    dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					     &eventset, DM_EVENT_MAX);
			rc |= close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_PMANR with region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_region_t region = { 0, 0, DM_REGION_READ };
		dm_boolean_t exactflag;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, 1, &region,
				       &exactflag)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_PMANR with region)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PMANR, &stat);
			if (rc == 0) {
				if (stat.dt_pmanreg == DM_TRUE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pmanreg (%d vs %d)\n",
						    szFuncName, 0, DM_TRUE,
						    stat.dt_pmanreg);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_PMANR without region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_PMANR without region)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PMANR, &stat);
			if (rc == 0) {
				if (stat.dt_pmanreg == DM_FALSE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pmanreg (%d vs %d)\n",
						    szFuncName, 0, DM_FALSE,
						    stat.dt_pmanreg);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_PATTR with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_PATTR with attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PATTR, &stat);
			if (rc == 0) {
				if (stat.dt_pers == DM_TRUE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pers (%d vs %d)\n",
						    szFuncName, 0, DM_TRUE,
						    stat.dt_pers);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_PATTR without DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_PATTR without attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PATTR, &stat);
			if (rc == 0) {
				if (stat.dt_pers == DM_FALSE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pers (%d vs %d)\n",
						    szFuncName, 0, DM_FALSE,
						    stat.dt_pers);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_DTIME with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			stat.dt_dtime = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_DTIME with attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &stat);
			if (rc == 0) {
				if (stat.dt_dtime != 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime not set\n",
						    szFuncName, 0);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_DTIME without DM attribute
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #23 (dtime updated without any DM
	 * attributes)
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 13)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			stat.dt_dtime = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file DM_AT_DTIME without attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &stat);
			if (rc == 0) {
				if (stat.dt_dtime == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime set\n",
						    szFuncName, 0);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - file DM_AT_STAT
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t statdm;
		struct stat statfs;
		int varStatus;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = stat(DUMMY_FILE, &statfs)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file DM_AT_STAT)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_STAT, &statdm);
			if (rc == 0) {
				varStatus = DMSTAT_PASS;
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s returned expected rc = %d\n",
					    szFuncName, rc);
				if (statfs.st_dev != statdm.dt_dev) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching dev (%lld vs %lld)\n",
						    szFuncName, statfs.st_dev,
						    statdm.dt_dev);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_ino != statdm.dt_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching ino (%lld vs %lld)\n",
						    szFuncName, statfs.st_ino,
						    statdm.dt_ino);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_mode != statdm.dt_mode) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching mode (%d vs %d)\n",
						    szFuncName, statfs.st_mode,
						    statdm.dt_mode);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_nlink != statdm.dt_nlink) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching nlink (%d vs %d)\n",
						    szFuncName, statfs.st_nlink,
						    statdm.dt_nlink);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_uid != statdm.dt_uid) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching uid (%d vs %d)\n",
						    szFuncName, statfs.st_uid,
						    statdm.dt_uid);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_gid != statdm.dt_gid) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching gid (%d vs %d)\n",
						    szFuncName, statfs.st_gid,
						    statdm.dt_gid);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_rdev != statdm.dt_rdev) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching rdev (%lld vs %lld)\n",
						    szFuncName, statfs.st_rdev,
						    statdm.dt_rdev);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_size != statdm.dt_size) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching size (%lld vs %lld)\n",
						    szFuncName, statfs.st_size,
						    statdm.dt_size);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_atime != statdm.dt_atime) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching atime (%d vs %d)\n",
						    szFuncName, statfs.st_atime,
						    statdm.dt_atime);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_mtime != statdm.dt_mtime) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching mtime (%d vs %d)\n",
						    szFuncName, statfs.st_mtime,
						    statdm.dt_mtime);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_ctime != statdm.dt_ctime) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching ctime (%d vs %d)\n",
						    szFuncName, statfs.st_ctime,
						    statdm.dt_ctime);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_blksize != statdm.dt_blksize) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching blksize (%d vs %d)\n",
						    szFuncName,
						    statfs.st_blksize,
						    statdm.dt_blksize);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_blocks != statdm.dt_blocks) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching blocks (%lld vs %lld)\n",
						    szFuncName,
						    statfs.st_blocks,
						    statdm.dt_blocks);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_EMASK on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 15)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_eventset_t eventset;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, eventset);
		DMEV_SET(DM_EVENT_CLOSE, eventset);
		DMEV_SET(DM_EVENT_DESTROY, eventset);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					  &eventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_EMASK)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &stat);
			if (rc == 0) {
				if (memcmp
				    (&eventset, &stat.dt_emask,
				     sizeof(dm_eventset_t)) == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected emask (%llx vs %llx)\n",
						    szFuncName, 0, eventset,
						    stat.dt_emask);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(eventset);
			rc |=
			    dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					     &eventset, DM_EVENT_MAX);
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_PMANR on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 16)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_PMANR)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PMANR, &stat);
			if (rc == 0) {
				if (stat.dt_pmanreg == DM_FALSE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pmanreg (%d vs %d)\n",
						    szFuncName, 0, DM_FALSE,
						    stat.dt_pmanreg);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_PATTR on directory with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 17)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir DM_AT_PATTR with attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PATTR, &stat);
			if (rc == 0) {
				if (stat.dt_pers == DM_TRUE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pers (%d vs %d)\n",
						    szFuncName, 0, DM_TRUE,
						    stat.dt_pers);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_PATTR on directory without DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 18)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir DM_AT_PATTR without attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_PATTR, &stat);
			if (rc == 0) {
				if (stat.dt_pers == DM_FALSE) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected pers (%d vs %d)\n",
						    szFuncName, 0, DM_FALSE,
						    stat.dt_pers);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_DTIME on directory with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 19)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			stat.dt_dtime = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir DM_AT_DTIME with attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &stat);
			if (rc == 0) {
				if (stat.dt_dtime != 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime not set\n",
						    szFuncName, 0);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_DTIME on directory without DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 20)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			stat.dt_dtime = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(dir DM_AT_DTIME without attr)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &stat);
			if (rc == 0) {
				if (stat.dt_dtime == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but dtime set\n",
						    szFuncName, 0);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_AT_STAT on directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 21)) {
		void *hanp;
		size_t hlen;
		dm_stat_t statdm;
		struct stat statfs;
		int varStatus;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = stat(DUMMY_SUBDIR, &statfs)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir DM_AT_STAT)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_STAT, &statdm);
			if (rc == 0) {
				varStatus = DMSTAT_PASS;
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s returned expected rc = %d\n",
					    szFuncName, rc);
				if (statfs.st_dev != statdm.dt_dev) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching dev (%lld vs %lld)\n",
						    szFuncName, statfs.st_dev,
						    statdm.dt_dev);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_ino != statdm.dt_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching ino (%lld vs %lld)\n",
						    szFuncName, statfs.st_ino,
						    statdm.dt_ino);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_mode != statdm.dt_mode) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching mode (%d vs %d)\n",
						    szFuncName, statfs.st_mode,
						    statdm.dt_mode);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_nlink != statdm.dt_nlink) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching nlink (%d vs %d)\n",
						    szFuncName, statfs.st_nlink,
						    statdm.dt_nlink);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_uid != statdm.dt_uid) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching uid (%d vs %d)\n",
						    szFuncName, statfs.st_uid,
						    statdm.dt_uid);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_gid != statdm.dt_gid) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching gid (%d vs %d)\n",
						    szFuncName, statfs.st_gid,
						    statdm.dt_gid);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_rdev != statdm.dt_rdev) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching rdev (%lld vs %lld)\n",
						    szFuncName, statfs.st_rdev,
						    statdm.dt_rdev);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_size != statdm.dt_size) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching size (%lld vs %lld)\n",
						    szFuncName, statfs.st_size,
						    statdm.dt_size);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_atime != statdm.dt_atime) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching atime (%d vs %d)\n",
						    szFuncName, statfs.st_atime,
						    statdm.dt_atime);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_mtime != statdm.dt_mtime) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching mtime (%d vs %d)\n",
						    szFuncName, statfs.st_mtime,
						    statdm.dt_mtime);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_ctime != statdm.dt_ctime) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching ctime (%d vs %d)\n",
						    szFuncName, statfs.st_ctime,
						    statdm.dt_ctime);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_blksize != statdm.dt_blksize) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching blksize (%d vs %d)\n",
						    szFuncName,
						    statfs.st_blksize,
						    statdm.dt_blksize);
					varStatus = DMSTAT_FAIL;
				}
				if (statfs.st_blocks != statdm.dt_blocks) {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with nonmatching blocks (%lld vs %lld)\n",
						    szFuncName,
						    statfs.st_blocks,
						    statdm.dt_blocks);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 22)) {
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 23)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_get_fileattr(DM_NO_SESSION, hanp, hlen,
					     DM_NO_TOKEN, DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_fileattr - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 24)) {
		dm_stat_t stat;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_get_fileattr(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				     DM_NO_TOKEN, DM_AT_EMASK, &stat);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_fileattr - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_FILEATTR_BASE + 25)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_stat_t stat;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_get_fileattr(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &stat);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_init_attrloc";

	/*
	 * TEST    : dm_init_attrloc - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 1)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_init_attrloc(INVALID_ADDR, hanp, hlen,
					     DM_NO_TOKEN, &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 2)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, (void *)INVALID_ADDR, hlen,
					     DM_NO_TOKEN, &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, hanp, INVALID_ADDR,
					     DM_NO_TOKEN, &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, hanp, hlen, INVALID_ADDR,
					     &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - invalid locp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 5)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid locp)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					     (dm_attrloc_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 6)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					     &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 7)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					     &loc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "loc = %lld\n", loc);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 8)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					     &loc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "loc = %lld\n", loc);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 9)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_init_attrloc(DM_NO_SESSION, hanp, hlen,
					     DM_NO_TOKEN, &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_init_attrloc - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 10)) {
		dm_attrloc_t loc;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_init_attrloc(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				     DM_NO_TOKEN, &loc);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_init_attrloc - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(INIT_ATTRLOC_BASE + 11)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rmdir(DUMMY_SUBDIR)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					     &loc);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_get_dirattrs";

	/*
	 * TEST    : dm_get_dirattrs - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 1)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_get_dirattrs(INVALID_ADDR, hanp, hlen,
					     DM_NO_TOKEN, DM_AT_EMASK, &loc,
					     sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 2)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, (void *)INVALID_ADDR, hlen,
					     DM_NO_TOKEN, DM_AT_EMASK, &loc,
					     sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid hlen
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, INVALID_ADDR,
					     DM_NO_TOKEN, DM_AT_EMASK, &loc,
					     sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, INVALID_ADDR,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid mask
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #24 (0 returned instead of -1 and
	 * EINVAL errno)
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 5)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid mask)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_SIZE, &loc, sizeof(buf), buf,
					     &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid locp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 6)) {
		void *hanp;
		size_t hlen;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid locp)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK,
					     (dm_attrloc_t *) INVALID_ADDR,
					     sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid loc
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 7)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			loc = INVALID_ADDR;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid loc)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid buflen
	 * EXPECTED: rc = 1
	 *
	 * This variation uncovered XFS BUG #26 (-1 and E2BIG errno returned
	 * instead of 1)
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 8)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid buflen)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, 0, buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			DMVAR_ENDPASSEXP(szFuncName, 1, rc);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 9)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     (void *)INVALID_ADDR, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 10)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rlenp)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 11)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_HANDLE
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 12)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_AT_HANDLE)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_HANDLE, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_HANDLE);
				if (entry != NULL) {
					if (dm_handle_cmp
					    (fhanp, fhlen,
					     DM_GET_VALUE(entry, dt_handle,
							  void *),
					     DM_GET_LEN(entry,
							dt_handle)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but handles NOT same\n",
							    szFuncName, 0);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_EMASK
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 13)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		dm_eventset_t eventset;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, eventset);
		DMEV_SET(DM_EVENT_CLOSE, eventset);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fhanp, fhlen, DM_NO_TOKEN,
					   &eventset, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_AT_EMASK)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_EMASK);
				if (entry != NULL) {
					if (eventset == entry->dt_emask) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but emasks NOT same (%llx vs %llx)\n",
							    szFuncName, 0,
							    eventset,
							    entry->dt_emask);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_PMANR with region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 14)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		dm_region_t region = { 0, 0, DM_REGION_READ };
		dm_boolean_t exactflag;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_set_region(sid, fhanp, fhlen, DM_NO_TOKEN, 1,
					&region, &exactflag)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_PMANR with region)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_PMANR, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_PMANR);
				if (entry != NULL) {
					if (entry->dt_pmanreg == DM_TRUE) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but pmanreg NOT same (%d vs %d)\n",
							    szFuncName, 0,
							    entry->dt_pmanreg,
							    DM_TRUE);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_PMANR without region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 15)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		rc |= dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN, &loc);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_PMANR without region)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_PMANR, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_PMANR);
				if (entry != NULL) {
					if (entry->dt_pmanreg == DM_FALSE) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but pmanreg NOT same (%d vs %d)\n",
							    szFuncName, 0,
							    entry->dt_pmanreg,
							    DM_FALSE);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_PATTR with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 16)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		dm_attrname_t attrname;
		char attrbuf[ATTR_VALUELEN];

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(attrbuf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, fhanp, fhlen, DM_NO_TOKEN,
					&attrname, 0, sizeof(attrbuf),
					attrbuf)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_PATTR with DM attr)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_PATTR, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_PATTR);
				if (entry != NULL) {
					if (entry->dt_pers == DM_TRUE) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but pers NOT same (%d vs %d)\n",
							    szFuncName, 0,
							    entry->dt_pers,
							    DM_TRUE);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_PATTR without DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 17)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_PATTR without DM attr)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_PATTR, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_PATTR);
				if (entry != NULL) {
					if (entry->dt_pers == DM_FALSE) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but pers NOT same (%d vs %d)\n",
							    szFuncName, 0,
							    entry->dt_pers,
							    DM_FALSE);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_DTIME with DM attribute
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 18)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		dm_attrname_t attrname;
		char attrbuf[ATTR_VALUELEN];
		struct stat statfs;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(attrbuf, ATTR_VALUE, ATTR_VALUELEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else if (((rc = stat(DUMMY_SUBDIR_FILE, &statfs)) == -1) ||
			   ((rc =
			     dm_set_dmattr(sid, fhanp, fhlen, DM_NO_TOKEN,
					   &attrname, 0, sizeof(attrbuf),
					   attrbuf)) == -1)
			   ||
			   ((rc =
			     dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					     &loc)) == -1)) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_DTIME with DM attr)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_DTIME);
				if (entry != NULL) {
					if (entry->dt_dtime == statfs.st_ctime) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but dtime NOT same (%d vs %d)\n",
							    szFuncName, 0,
							    entry->dt_dtime,
							    statfs.st_ctime);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_DTIME without DM attribute
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #25 (dtime updated without any DM
	 * attributes)
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 19)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		struct stat statfs;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else if (((rc = stat(DUMMY_SUBDIR_FILE, &statfs)) == -1) ||
			   ((rc =
			     dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					     &loc)) == -1)) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_DTIME without DM attr)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_DTIME, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_DTIME);
				if (entry != NULL) {
					if (entry->dt_dtime != statfs.st_ctime) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but dtime same (%d vs %d)\n",
							    szFuncName, 0,
							    entry->dt_dtime,
							    statfs.st_ctime);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_CFLAG with no change
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 20)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_LISTLEN], buf2[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf1, 0, ATTR_LISTLEN);
		memset(buf2, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					  DM_AT_CFLAG, &loc, sizeof(buf1), buf1,
					  &rlen)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_CFLAG with no change)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_CFLAG, &loc, sizeof(buf2),
					     buf2, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry1 =
				    GetDirEntry(buf1, DUMMY_FILE);
				dm_stat_t *entry2 =
				    GetDirEntry(buf2, DUMMY_FILE);
				LogDirAttrs(buf2, DM_AT_CFLAG);
				if ((entry1 != NULL) && (entry2 != NULL)) {
					if (entry1->dt_change ==
					    entry2->dt_change) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but change not same (%d vs %d)\n",
							    szFuncName, 0,
							    entry1->dt_change,
							    entry2->dt_change);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_CFLAG with data change
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 21)) {
		int fd;
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_LISTLEN], buf2[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf1, 0, ATTR_LISTLEN);
		memset(buf2, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((fd = open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
				      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					  DM_AT_CFLAG, &loc, sizeof(buf1), buf1,
					  &rlen)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN ? -1 : 0) == 1))) {
			close(fd);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_CFLAG with data change)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_CFLAG, &loc, sizeof(buf2),
					     buf2, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry1 =
				    GetDirEntry(buf1, DUMMY_FILE);
				dm_stat_t *entry2 =
				    GetDirEntry(buf2, DUMMY_FILE);
				LogDirAttrs(buf2, DM_AT_CFLAG);
				if ((entry1 != NULL) && (entry2 != NULL)) {
					if (entry1->dt_change !=
					    entry2->dt_change) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but change same (%d vs %d)\n",
							    szFuncName, 0,
							    entry1->dt_change,
							    entry2->dt_change);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_CFLAG with metadata change
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 22)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_LISTLEN], buf2[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf1, 0, ATTR_LISTLEN);
		memset(buf2, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					  DM_AT_CFLAG, &loc, sizeof(buf1), buf1,
					  &rlen)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  chown(DUMMY_SUBDIR_FILE, DUMMY_UID,
				DUMMY_GID)) == -1)) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_CFLAG with metadata change)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_CFLAG, &loc, sizeof(buf2),
					     buf2, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry1 =
				    GetDirEntry(buf1, DUMMY_FILE);
				dm_stat_t *entry2 =
				    GetDirEntry(buf2, DUMMY_FILE);
				LogDirAttrs(buf2, DM_AT_CFLAG);
				if ((entry1 != NULL) && (entry2 != NULL)) {
					if (entry1->dt_change !=
					    entry2->dt_change) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but change same (%d vs %d)\n",
							    szFuncName, 0,
							    entry1->dt_change,
							    entry2->dt_change);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_CFLAG with DM attribute change
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 23)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_LISTLEN], buf2[ATTR_LISTLEN];
		size_t rlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(buf1, 0, ATTR_LISTLEN);
		memset(buf2, 0, ATTR_LISTLEN);
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					  DM_AT_CFLAG, &loc, sizeof(buf1), buf1,
					  &rlen)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  dm_set_dmattr(sid, fhanp, fhlen, DM_NO_TOKEN,
					&attrname, 0, 0, NULL)) == -1)) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_CFLAG with DM attr change)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_CFLAG, &loc, sizeof(buf2),
					     buf2, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry1 =
				    GetDirEntry(buf1, DUMMY_FILE);
				dm_stat_t *entry2 =
				    GetDirEntry(buf2, DUMMY_FILE);
				LogDirAttrs(buf2, DM_AT_CFLAG);
				if ((entry1 != NULL) && (entry2 != NULL)) {
					if (entry1->dt_change !=
					    entry2->dt_change) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but change same (%d vs %d)\n",
							    szFuncName, 0,
							    entry1->dt_change,
							    entry2->dt_change);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_CFLAG with non-DM attribute change
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 23)) {
		void *dhanp, *fhanp;
		size_t dhlen, fhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_LISTLEN], buf2[ATTR_LISTLEN];
		size_t rlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(buf1, 0, ATTR_LISTLEN);
		memset(buf2, 0, ATTR_LISTLEN);
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp,
					   &fhlen)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					  DM_AT_CFLAG, &loc, sizeof(buf1), buf1,
					  &rlen)) == -1)
			||
			((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			||
			((rc =
			  setxattr(DUMMY_SUBDIR_FILE, NON_DM_ATTR_NAME,
				   NON_DM_ATTR_VALUE, sizeof(NON_DM_ATTR_VALUE),
				   0)) == -1)) {
			dm_handle_free(fhanp, fhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_CFLAG with non-DM attr change)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_CFLAG, &loc, sizeof(buf2),
					     buf2, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry1 =
				    GetDirEntry(buf1, DUMMY_FILE);
				dm_stat_t *entry2 =
				    GetDirEntry(buf2, DUMMY_FILE);
				LogDirAttrs(buf2, DM_AT_CFLAG);
				if ((entry1 != NULL) && (entry2 != NULL)) {
					if (entry1->dt_change !=
					    entry2->dt_change) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but change same (%d vs %d)\n",
							    szFuncName, 0,
							    entry1->dt_change,
							    entry2->dt_change);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 24)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;
		struct stat statfs;
		int varStatus;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = stat(DUMMY_SUBDIR_FILE, &statfs)) == -1)) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_AT_STAT)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_STAT);
				if (entry != NULL) {
					varStatus = DMSTAT_PASS;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s returned expected rc = %d\n",
						    szFuncName, rc);
					if (statfs.st_dev != entry->dt_dev) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching dev (%lld vs %lld)\n",
							    szFuncName,
							    statfs.st_dev,
							    entry->dt_dev);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_ino != entry->dt_ino) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching ino (%lld vs %lld)\n",
							    szFuncName,
							    statfs.st_ino,
							    entry->dt_ino);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_mode != entry->dt_mode) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching mode (%d vs %d)\n",
							    szFuncName,
							    statfs.st_mode,
							    entry->dt_mode);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_nlink != entry->dt_nlink) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching nlink (%d vs %d)\n",
							    szFuncName,
							    statfs.st_nlink,
							    entry->dt_nlink);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_uid != entry->dt_uid) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching uid (%d vs %d)\n",
							    szFuncName,
							    statfs.st_uid,
							    entry->dt_uid);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_gid != entry->dt_gid) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching gid (%d vs %d)\n",
							    szFuncName,
							    statfs.st_gid,
							    entry->dt_gid);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_rdev != entry->dt_rdev) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching rdev (%lld vs %lld)\n",
							    szFuncName,
							    statfs.st_rdev,
							    entry->dt_rdev);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_size != entry->dt_size) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching size (%lld vs %lld)\n",
							    szFuncName,
							    statfs.st_size,
							    entry->dt_size);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_atime != entry->dt_atime) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching atime (%d vs %d)\n",
							    szFuncName,
							    statfs.st_atime,
							    entry->dt_atime);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_mtime != entry->dt_mtime) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching mtime (%d vs %d)\n",
							    szFuncName,
							    statfs.st_mtime,
							    entry->dt_mtime);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_ctime != entry->dt_ctime) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching ctime (%d vs %d)\n",
							    szFuncName,
							    statfs.st_ctime,
							    entry->dt_ctime);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_blksize !=
					    entry->dt_blksize) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching blksize (%d vs %d)\n",
							    szFuncName,
							    statfs.st_blksize,
							    entry->dt_blksize);
						varStatus = DMSTAT_FAIL;
					}
					if (statfs.st_blocks !=
					    entry->dt_blocks) {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with nonmatching blocks (%lld vs %lld)\n",
							    szFuncName,
							    statfs.st_blocks,
							    entry->dt_blocks);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT returned over two calls
	 * EXPECTED: rc = 1, 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 25)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf1[2 * ATTR_SMALLLEN];
		char buf2[ATTR_SMALLLEN + 1];
		size_t rlen1, rlen2;
		dm_stat_t *entry;
		int rc1, rc2;
		int varStatus;
		int num;

		/* Variation set up */
		memset(buf1, 0, sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT over two calls)\n",
				    szFuncName);
			rc1 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf1),
					    buf1, &rlen1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "1st call: rc %d, rlen %d, loc %llx\n", rc1,
				    rlen1, loc);
			rc2 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf2),
					    buf2, &rlen2);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "2nd call: rc %d, rlen %d, loc %llx\n", rc2,
				    rlen2, loc);
			varStatus = DMSTAT_PASS;
			if (rc1 == 1) {
				if (((num = GetNumDirEntry(buf1)) == 2)
				    && (rlen1 >= 2 * MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "1st call attrs:\n");
					LogDirAttrs(buf1, DM_AT_STAT);
					if (((entry =
					      GetDirEntry(buf1,
							  CURRENT_DIR)) != NULL)
					    &&
					    ((entry =
					      GetDirEntry(buf1,
							  PARENT_DIR)) !=
					     NULL)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 1st call returned expected rc = %d and %d entries %s and %s in buffer\n",
							    szFuncName, rc1,
							    num, CURRENT_DIR,
							    PARENT_DIR);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 1st call returned expected rc = %d but entries %s and/or %s not in buffer\n",
							    szFuncName, rc1,
							    CURRENT_DIR,
							    PARENT_DIR);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 1st call returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc1, rlen1,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 1st call returned unexpected rc = %d\n",
					    szFuncName, rc1);
				varStatus = DMSTAT_FAIL;
			}
			if (rc2 == 0) {
				if (((num = GetNumDirEntry(buf2)) == 1)
				    && (rlen2 >= MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "2nd call attrs:\n");
					LogDirAttrs(buf2, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf2,
							 DUMMY_FILE)) != NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 2nd call returned expected rc = %d and %d entry %s in buffer\n",
							    szFuncName, rc2,
							    num, DUMMY_FILE);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 2nd call returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc2,
							    DUMMY_FILE);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 2nd call returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc2, rlen2,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 2nd call returned unexpected rc = %d\n",
					    szFuncName, rc2);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT returned over three calls,
	 *              third buffer too small
	 * EXPECTED: rc = 1, 1, 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 26)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_SMALLLEN];
		char buf2[ATTR_SMALLLEN];
		char buf3[ATTR_SMALLLEN / 2];
		size_t rlen1, rlen2, rlen3;
		dm_stat_t *entry;
		int rc1, rc2, rc3;
		int varStatus;
		int num;

		/* Variation set up */
		memset(buf1, 0, sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		memset(buf3, 0, sizeof(buf3));
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT over three calls, third buf too small)\n",
				    szFuncName);
			rc1 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf1),
					    buf1, &rlen1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "1st call: rc %d, rlen %d, loc %llx\n", rc1,
				    rlen1, loc);
			rc2 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf2),
					    buf2, &rlen2);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "2nd call: rc %d, rlen %d, loc %llx\n", rc2,
				    rlen2, loc);
			rc3 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf3),
					    buf3, &rlen3);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "3rd call: rc %d, rlen %d, loc %llx\n", rc3,
				    rlen3, loc);
			varStatus = DMSTAT_PASS;
			if (rc1 == 1) {
				if (((num = GetNumDirEntry(buf1)) == 1)
				    && (rlen1 >= MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "1st call attrs:\n");
					LogDirAttrs(buf1, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf1,
							 CURRENT_DIR)) !=
					    NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 1st call returned expected rc = %d and %d entry %s in buffer\n",
							    szFuncName, rc1,
							    num, CURRENT_DIR);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 1st call returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc1,
							    CURRENT_DIR);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 1st call returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc1, rlen1,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 1st call returned unexpected rc = %d\n",
					    szFuncName, rc1);
				varStatus = DMSTAT_FAIL;
			}
			if (rc2 == 1) {
				if (((num = GetNumDirEntry(buf2)) == 1)
				    && (rlen2 >= MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "2nd call attrs:\n");
					LogDirAttrs(buf2, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf2,
							 PARENT_DIR)) != NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 2nd call returned expected rc = %d and %d entry %s in buffer\n",
							    szFuncName, rc2,
							    num, PARENT_DIR);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 2nd call returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc2,
							    PARENT_DIR);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 2nd call returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc2, rlen2,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 2nd call returned unexpected rc = %d\n",
					    szFuncName, rc2);
				varStatus = DMSTAT_FAIL;
			}
			if (rc3 == 1) {
				if (rlen3 == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s 3rd call returned expected rc = %d and empty buffer\n",
						    szFuncName, rc3);
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 3rd call returned expected rc = %d but unexpected rlen = %d\n",
						    szFuncName, rc3, rlen3);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 3rd call returned unexpected rc = %d\n",
					    szFuncName, rc3);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT returned over three calls
	 * EXPECTED: rc = 1, 1, 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 27)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf1[ATTR_SMALLLEN];
		char buf2[ATTR_SMALLLEN];
		char buf3[ATTR_SMALLLEN];
		size_t rlen1, rlen2, rlen3;
		dm_stat_t *entry;
		int rc1, rc2, rc3;
		int varStatus;
		int num;

		/* Variation set up */
		memset(buf1, 0, sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		memset(buf3, 0, sizeof(buf3));
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT over three calls)\n",
				    szFuncName);
			rc1 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf1),
					    buf1, &rlen1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "1st call: rc %d, rlen %d, loc %llx\n", rc1,
				    rlen1, loc);
			rc2 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf2),
					    buf2, &rlen2);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "2nd call: rc %d, rlen %d, loc %llx\n", rc2,
				    rlen2, loc);
			rc3 =
			    dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					    DM_AT_STAT, &loc, sizeof(buf3),
					    buf3, &rlen3);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "3rd call: rc %d, rlen %d, loc %llx\n", rc3,
				    rlen3, loc);
			varStatus = DMSTAT_PASS;
			if (rc1 == 1) {
				if (((num = GetNumDirEntry(buf1)) == 1)
				    && (rlen1 >= MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "1st call attrs:\n");
					LogDirAttrs(buf1, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf1,
							 CURRENT_DIR)) !=
					    NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 1st call returned expected rc = %d and %d entry %s in buffer\n",
							    szFuncName, rc1,
							    num, CURRENT_DIR);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 1st call returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc1,
							    CURRENT_DIR);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 1st call returned expected rc = %d but unexpected rlen = %d\n and/or number of entries in buffer %d",
						    szFuncName, rc1, rlen1,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 1st call returned unexpected rc = %d\n",
					    szFuncName, rc1);
				varStatus = DMSTAT_FAIL;
			}
			if (rc2 == 1) {
				if (((num = GetNumDirEntry(buf2)) == 1)
				    && (rlen2 >= MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "2nd call attrs:\n");
					LogDirAttrs(buf2, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf2,
							 PARENT_DIR)) != NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 2nd call returned expected rc = %d and %d entry %s in buffer\n",
							    szFuncName, rc2,
							    num, PARENT_DIR);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 2nd call returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc2,
							    PARENT_DIR);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 2nd call returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc2, rlen2,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 2nd call returned unexpected rc = %d\n",
					    szFuncName, rc2);
				varStatus = DMSTAT_FAIL;
			}
			if (rc3 == 0) {
				if (((num = GetNumDirEntry(buf3)) == 1)
				    && (rlen3 >= MIN_ENTRYLEN)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "3rd call attrs:\n");
					LogDirAttrs(buf3, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf3,
							 DUMMY_FILE)) != NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s 3rd call returned expected rc = %d and %d entry %s in buffer\n",
							    szFuncName, rc3,
							    num, DUMMY_FILE);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s 3rd call returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc3,
							    DUMMY_FILE);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s 3rd call returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc3, rlen3,
						    num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 3rd call returned unexpected rc = %d\n",
					    szFuncName, rc3);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir > files fit in buffer)
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 28)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_SMALLLEN * (DIRENTS_FILES - 1)];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, DIRENTS_FILES - 1);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 1) {
				if (((num =
				      GetNumDirEntry(buf)) == DIRENTS_FILES - 1)
				    && (rlen >=
					(DIRENTS_FILES - 1) * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) == NULL) {
						if (((entry =
						      GetLastDirEntry(buf)) !=
						     NULL)
						    && (entry->dt_compname.
							vd_length > 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s returned expected rc = %d, expected number of entries in buffer %d, and neither entry %s nor empty entry in buffer\n",
								    szFuncName,
								    rc, num,
								    filename);
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s returned expected rc = %d but empty entry in buffer\n",
								    szFuncName,
								    rc,
								    filename);
							varStatus = DMSTAT_FAIL;
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir > files fit in buffer)
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 29)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_SMALLLEN * DIRENTS_FILES];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, DIRENTS_FILES);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 1) {
				if (((num =
				      GetNumDirEntry(buf)) == DIRENTS_FILES)
				    && (rlen >= DIRENTS_FILES * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) == NULL) {
						if (((entry =
						      GetLastDirEntry(buf)) !=
						     NULL)
						    && (entry->dt_compname.
							vd_length > 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s returned expected rc = %d, expected number of entries in buffer %d, and neither entry %s nor empty entry in buffer\n",
								    szFuncName,
								    rc, num,
								    filename);
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s returned expected rc = %d but empty entry in buffer\n",
								    szFuncName,
								    rc,
								    filename);
							varStatus = DMSTAT_FAIL;
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir > files fit in buffer)
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 30)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_SMALLLEN * (DIRENTS_FILES + 1)];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, DIRENTS_FILES + 1);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 1) {
				if (((num =
				      GetNumDirEntry(buf)) == DIRENTS_FILES + 1)
				    && (rlen >=
					(DIRENTS_FILES + 1) * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) == NULL) {
						if (((entry =
						      GetLastDirEntry(buf)) !=
						     NULL)
						    && (entry->dt_compname.
							vd_length > 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s returned expected rc = %d, expected number of entries in buffer %d, and neither entry %s nor empty entry in buffer\n",
								    szFuncName,
								    rc, num,
								    filename);
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s returned expected rc = %d but empty entry in buffer\n",
								    szFuncName,
								    rc,
								    filename);
							varStatus = DMSTAT_FAIL;
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir > files fit in buffer)
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 31)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_SMALLLEN * ((2 * DIRENTS_FILES) - 1)];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, (2 * DIRENTS_FILES) - 1);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 1) {
				if (((num =
				      GetNumDirEntry(buf)) ==
				     (2 * DIRENTS_FILES) - 1)
				    && (rlen >=
					((2 * DIRENTS_FILES) -
					 1) * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) == NULL) {
						if (((entry =
						      GetLastDirEntry(buf)) !=
						     NULL)
						    && (entry->dt_compname.
							vd_length > 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s returned expected rc = %d, expected number of entries in buffer %d, and neither entry %s nor empty entry in buffer\n",
								    szFuncName,
								    rc, num,
								    filename);
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s returned expected rc = %d but empty entry in buffer\n",
								    szFuncName,
								    rc,
								    filename);
							varStatus = DMSTAT_FAIL;
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir > files fit in buffer)
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 32)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_SMALLLEN * (2 * DIRENTS_FILES)];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, 2 * DIRENTS_FILES);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 1) {
				if (((num =
				      GetNumDirEntry(buf)) == 2 * DIRENTS_FILES)
				    && (rlen >=
					(2 * DIRENTS_FILES) * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) == NULL) {
						if (((entry =
						      GetLastDirEntry(buf)) !=
						     NULL)
						    && (entry->dt_compname.
							vd_length > 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s returned expected rc = %d, expected number of entries in buffer %d, and neither entry %s nor empty entry in buffer\n",
								    szFuncName,
								    rc, num,
								    filename);
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s returned expected rc = %d but empty entry in buffer\n",
								    szFuncName,
								    rc,
								    filename);
							varStatus = DMSTAT_FAIL;
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir > files fit in buffer)
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 33)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_SMALLLEN * ((2 * DIRENTS_FILES) + 1)];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, (2 * DIRENTS_FILES) + 1);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 1) {
				if (((num =
				      GetNumDirEntry(buf)) ==
				     (2 * DIRENTS_FILES) + 1)
				    && (rlen >=
					((2 * DIRENTS_FILES) +
					 1) * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) == NULL) {
						if (((entry =
						      GetLastDirEntry(buf)) !=
						     NULL)
						    && (entry->dt_compname.
							vd_length > 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s returned expected rc = %d, expected number of entries in buffer %d, and neither entry %s nor empty entry in buffer\n",
								    szFuncName,
								    rc, num,
								    filename);
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s returned expected rc = %d but empty entry in buffer\n",
								    szFuncName,
								    rc,
								    filename);
							varStatus = DMSTAT_FAIL;
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_STAT with one buffer (files
	 *              returned from jfs_readdir < files fit in buffer)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 34)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_BIGLISTLEN];
		size_t rlen;
		dm_stat_t *entry;
		int varStatus;
		int i;
		char *filename;
		int num;

		/* Variation set up */
		memset(buf, 0, sizeof(buf));
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else {
			for (i = 0; i < NUM_FILES && rc == 0; i++) {
				sprintf(command, "cp %s %s.%3.3d", DUMMY_TMP,
					DUMMY_SUBDIR_FILE, i);
				rc = system(command);
			}
			if ((rc == -1) ||
			    ((rc =
			      dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					      &loc)) == -1)) {
				sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
				rc = system(command);
			}
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_AT_STAT with %d files)\n",
				    szFuncName, NUM_FILES + 2);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_STAT, &loc, sizeof(buf), buf,
					     &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			varStatus = DMSTAT_PASS;
			if (rc == 0) {
				if (((num =
				      GetNumDirEntry(buf)) == NUM_FILES + 2)
				    && (rlen >=
					(NUM_FILES + 2) * MIN_ENTRYLEN)) {
					filename = strchr(command, '/') + 1;
					DMLOG_PRINT(DMLVL_DEBUG, "attrs:\n");
					LogDirAttrs(buf, DM_AT_STAT);
					if ((entry =
					     GetDirEntry(buf,
							 filename)) != NULL) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s returned expected rc = %d, expected number of entries in buffer %d, and entry %s in buffer\n",
							    szFuncName, rc, num,
							    filename);
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s returned expected rc = %d but entry %s not in buffer\n",
							    szFuncName, rc,
							    filename);
						varStatus = DMSTAT_FAIL;
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s returned expected rc = %d but unexpected rlen = %d and/or number of entries in buffer %d\n",
						    szFuncName, rc, rlen, num);
					varStatus = DMSTAT_FAIL;
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d\n",
					    szFuncName, rc);
				varStatus = DMSTAT_FAIL;
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			sprintf(command, "rm -rf %s", DUMMY_SUBDIR);
			rc = system(command);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_HANDLE with link
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 35)) {
		void *dhanp, *fhanp1, *fhanp2;
		size_t dhlen, fhlen1, fhlen2;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = system(command)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_FILE, &fhanp1,
					   &fhlen1)) == -1) {
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = link(DUMMY_SUBDIR_FILE, DUMMY_SUBDIR_LINK)) ==
			   -1) {
			dm_handle_free(fhanp1, fhlen1);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_LINK, &fhanp2,
					   &fhlen2)) == -1) {
			unlink(DUMMY_SUBDIR_LINK);
			dm_handle_free(fhanp1, fhlen1);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			dm_handle_free(fhanp2, fhlen2);
			unlink(DUMMY_SUBDIR_LINK);
			dm_handle_free(fhanp1, fhlen1);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_AT_HANDLE with link)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_HANDLE, &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry1 =
				    GetDirEntry(buf, DUMMY_FILE);
				dm_stat_t *entry2 =
				    GetDirEntry(buf, DUMMY_LINK);
				LogDirAttrs(buf, DM_AT_HANDLE);
				if (entry1 != NULL) {
					if (dm_handle_cmp
					    (fhanp1, fhlen1,
					     DM_GET_VALUE(entry1, dt_handle,
							  void *),
					     DM_GET_LEN(entry1,
							dt_handle)) == 0) {
						if (entry2 != NULL) {
							if (dm_handle_cmp
							    (fhanp2, fhlen2,
							     DM_GET_VALUE
							     (entry2, dt_handle,
							      void *),
							     DM_GET_LEN(entry2,
									dt_handle))
							    == 0) {
								DMLOG_PRINT
								    (DMLVL_DEBUG,
								     "%s passed with expected rc = %d\n",
								     szFuncName,
								     0);
								DMVAR_PASS();
							} else {
								DMLOG_PRINT
								    (DMLVL_ERR,
								     "%s failed with expected rc = %d but link handles NOT same\n",
								     szFuncName,
								     0);
								DMVAR_FAIL();
							}
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s failed with expected rc = %d but unable to find entry %s",
								    szFuncName,
								    0,
								    DUMMY_LINK);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but file handles NOT same\n",
							    szFuncName, 0);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= remove(DUMMY_SUBDIR_LINK);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
			dm_handle_free(fhanp1, fhlen1);
			dm_handle_free(fhanp2, fhlen2);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_AT_EMASK (verify no handle)
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #28 (handle returned when
	 * DM_AT_HANDLE not set in mask)
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 36)) {
		void *dhanp;
		size_t dhlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		memset(buf, 0, ATTR_LISTLEN);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if (((rc = system(command)) == -1) ||
			   ((rc =
			     dm_init_attrloc(sid, dhanp, dhlen, DM_NO_TOKEN,
					     &loc)) == -1)) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(~DM_AT_HANDLE)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, dhanp, dhlen, DM_NO_TOKEN,
					     DM_AT_ALL_DIRATTRS &
					     (~DM_AT_HANDLE), &loc, sizeof(buf),
					     buf, &rlen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "call: rc %d, loc %lld, rlen %d\n", rc, loc,
				    rlen);
			if (rc == 0) {
				dm_stat_t *entry = GetDirEntry(buf, DUMMY_FILE);
				LogDirAttrs(buf, DM_AT_ALL_DIRATTRS);
				if (entry != NULL) {
					if ((entry->dt_handle.vd_offset == 0)
					    && (entry->dt_handle.vd_length ==
						0)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but handle non-zero (offset %d, length %d)\n",
							    szFuncName, 0,
							    entry->dt_handle.
							    vd_offset,
							    entry->dt_handle.
							    vd_length);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unable to find entry %s",
						    szFuncName, 0, DUMMY_FILE);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 37)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			remove(DUMMY_FILE);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 38)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_SUBDIR_FILE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if (((rc =
			  dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					  &loc)) == -1)
			|| ((rc = system(command)) == -1)) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_get_dirattrs(DM_NO_SESSION, hanp, hlen,
					     DM_NO_TOKEN, DM_AT_EMASK, &loc,
					     sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 39)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, DM_GLOBAL_HANP,
					     DM_GLOBAL_HLEN, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_dirattrs - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_DIRATTRS_BASE + 40)) {
		void *hanp;
		size_t hlen;
		dm_attrloc_t loc;
		char buf[ATTR_LISTLEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_init_attrloc(sid, hanp, hlen, DM_NO_TOKEN,
					 &loc)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = rmdir(DUMMY_SUBDIR)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_get_dirattrs(sid, hanp, hlen, DM_NO_TOKEN,
					     DM_AT_EMASK, &loc, sizeof(buf),
					     buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_set_inherit";

	/*
	 * TEST    : dm_set_inherit - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_set_inherit(INVALID_ADDR, hanp, hlen,
					    DM_NO_TOKEN, &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_set_inherit(sid, (void *)INVALID_ADDR, hlen,
					    DM_NO_TOKEN, &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_set_inherit(sid, hanp, INVALID_ADDR,
					    DM_NO_TOKEN, &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_set_inherit(sid, hanp, hlen, INVALID_ADDR,
					    &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - invalid attrnamep
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrnamep)\n",
				    szFuncName);
			rc = dm_set_inherit(sid, hanp, hlen, DM_NO_TOKEN,
					    (dm_attrname_t *) INVALID_ADDR, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc,
					 persInheritAttr ? EFAULT : ENOSYS);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_set_inherit(DM_NO_SESSION, hanp, hlen,
					    DM_NO_TOKEN, &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 7)) {
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_set_inherit(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				    DM_NO_TOKEN, &attrname, 0);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_set_inherit - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_set_inherit(sid, hanp, hlen, DM_NO_TOKEN,
					    &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_inherit - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_INHERIT_BASE + 9)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_set_inherit(sid, hanp, hlen, DM_NO_TOKEN,
					    &attrname, 0);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_clear_inherit";

	/*
	 * TEST    : dm_clear_inherit - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_clear_inherit(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_clear_inherit(sid, (void *)INVALID_ADDR, hlen,
					      DM_NO_TOKEN, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_clear_inherit(sid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_clear_inherit(sid, hanp, hlen, INVALID_ADDR,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - invalid attrnamep
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrnamep)\n",
				    szFuncName);
			rc = dm_clear_inherit(sid, hanp, hlen, DM_NO_TOKEN,
					      (dm_attrname_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc,
					 persInheritAttr ? EFAULT : ENOSYS);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_clear_inherit(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 7)) {
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_clear_inherit(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      DM_NO_TOKEN, &attrname);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_clear_inherit - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_clear_inherit(sid, hanp, hlen, DM_NO_TOKEN,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_clear_inherit - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CLEAR_INHERIT_BASE + 9)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_clear_inherit(sid, hanp, hlen, DM_NO_TOKEN,
					      &attrname);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_getall_inherit";

	/*
	 * TEST    : dm_getall_inherit - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_getall_inherit(INVALID_ADDR, hanp, hlen,
					       DM_NO_TOKEN, 1, &inheritbuf,
					       &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_getall_inherit(sid, (void *)INVALID_ADDR, hlen,
					       DM_NO_TOKEN, 1, &inheritbuf,
					       &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_getall_inherit(sid, hanp, INVALID_ADDR,
					       DM_NO_TOKEN, 1, &inheritbuf,
					       &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_getall_inherit(sid, hanp, hlen, INVALID_ADDR, 1,
					       &inheritbuf, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - invalid inheritbufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid inheritbufp)\n",
				    szFuncName);
			rc = dm_getall_inherit(sid, hanp, hlen, DM_NO_TOKEN, 1,
					       (dm_inherit_t *) INVALID_ADDR,
					       &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc,
					 persInheritAttr ? EFAULT : ENOSYS);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_getall_inherit(DM_NO_SESSION, hanp, hlen,
					       DM_NO_TOKEN, 1, &inheritbuf,
					       &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 7)) {
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_getall_inherit(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				       DM_NO_TOKEN, 1, &inheritbuf, &nelem);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_getall_inherit - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_getall_inherit(sid, hanp, hlen, DM_NO_TOKEN, 1,
					       &inheritbuf, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_inherit - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_INHERIT_BASE + 9)) {
		void *hanp;
		size_t hlen;
		dm_inherit_t inheritbuf;
		u_int nelem;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_getall_inherit(sid, hanp, hlen, DM_NO_TOKEN, 1,
					       &inheritbuf, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	rc = dm_destroy_session(sid);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_destroy_session failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	remove(DUMMY_TMP);

	DMLOG_STOP();

	tst_exit();
}
