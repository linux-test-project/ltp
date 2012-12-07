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
 * TEST CASE	: handle.c
 *
 * VARIATIONS	: 156
 *
 * API'S TESTED	: dm_path_to_handle
 * 		  dm_fd_to_handle
 * 		  dm_path_to_fshandle
 * 		  dm_handle_to_fshandle
 * 		  dm_handle_cmp
 * 		  dm_handle_free
 * 		  dm_handle_is_valid
 * 		  dm_handle_hash
 * 		  dm_handle_to_fsid
 * 		  dm_handle_to_igen
 * 		  dm_handle_to_ino
 * 		  dm_make_handle
 * 		  dm_make_fshandle
 * 		  dm_handle_to_path
 * 		  dm_sync_by_handle
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "dm_test.h"

#define PATHBUF_LEN 256

#define DIR_LEVEL1 "level1"
#define DIR_LEVEL2 "level1/level2"
#define DIR_LEVEL3 "level1/level2/level3"
#define DIR_LEVEL4 "level1/level2/level3/level4"
#define FILE_LEVEL4 "level1/level2/level3/level4/dummy.txt"
#define PATH_NOTDIR "dummy.txt/dummy.txt"
#define FILE_NOTDMAPI "/usr/include/errno.h"
#define PATH_TOOLONG "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456"

char *mountPt;
char DummySubdirFile[FILENAME_MAX];
dm_sessid_t sid;

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int rc;
	dm_boolean_t bRC;
	char *szSessionInfo = "dm_test session info";
	void *mtpthanp, *curdirhanp;
	size_t mtpthlen, curdirhlen;

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	if ((mountPt = DMOPT_GET("mtpt")) == NULL) {
		DMLOG_PRINT(DMLVL_ERR,
			    "Missing mount point, use -mtpt (for example, -mtpt /dmapidir)\n");
		DM_EXIT();
	} else {
		DMLOG_PRINT(DMLVL_DEBUG, "Mount point is %s\n", mountPt);
	}

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
	} else
	    if (((rc = dm_path_to_handle(mountPt, &mtpthanp, &mtpthlen)) != 0)
		||
		((rc =
		  dm_path_to_handle(CURRENT_DIR, &curdirhanp,
				    &curdirhlen)) != 0)) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_path_to_handle failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else if (dm_handle_cmp(mtpthanp, mtpthlen, curdirhanp, curdirhlen) !=
		   0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "This test case must be run from the root directory of the DMAPI FS (%s)\n",
			    mountPt);
		DM_EXIT();
	} else {
		sprintf(DummySubdirFile, "%s/%s", mountPt, DUMMY_SUBDIR_FILE);

		remove(DUMMY_SUBDIR_FILE);
		unlink(DUMMY_SUBDIR_LINK);
		rmdir(DUMMY_SUBDIR_SUBDIR);
		remove(DUMMY_FILE);
		remove(DUMMY_FILE2);
		unlink(DUMMY_LINK);
		rmdir(DUMMY_SUBDIR);
		remove(FILE_LEVEL4);
		rmdir(DIR_LEVEL4);
		rmdir(DIR_LEVEL3);
		rmdir(DIR_LEVEL2);
		rmdir(DIR_LEVEL1);
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI handle tests\n");

	szFuncName = "dm_path_to_handle";

	/*
	 * TEST    : dm_path_to_handle - invalid path
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #3 (0 return code from strnlen_user
	 * ignored, which indicated fault)
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 1)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid path)\n", szFuncName);
		rc = dm_path_to_handle((char *)INVALID_ADDR, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_handle - nonexistent path, current directory
	 * EXPECTED: rc = -1, errno = ENOENT
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 2)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(nonexistent path in curdir)\n",
			    szFuncName);
		rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_handle - file in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in curdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
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
	 * TEST    : dm_path_to_handle - link in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* no clean up */
		} else if ((rc = link(DUMMY_FILE, DUMMY_LINK)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in curdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_LINK, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			rc |= unlink(DUMMY_LINK);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_path_to_handle - directory in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 5)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir in curdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_path_to_handle - nonexistent path in subdirectory
	 * EXPECTED: rc = -1, errno = ENOENT
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 6)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(nonexistent path in subdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_SUBDIR_FILE, &hanp, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_path_to_handle - file in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in subdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_SUBDIR_FILE, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_SUBDIR_FILE);
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
	 * TEST    : dm_path_to_handle - link in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = link(DUMMY_SUBDIR_FILE, DUMMY_SUBDIR_LINK)) ==
			   -1) {
			close(fd);
			remove(DUMMY_SUBDIR_FILE);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in subdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_SUBDIR_LINK, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_SUBDIR_FILE);
			rc |= unlink(DUMMY_SUBDIR_LINK);
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
	 * TEST    : dm_path_to_handle - directory in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 9)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DUMMY_SUBDIR_SUBDIR, DUMMY_DIR_RW_MODE))
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir in subdir)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_SUBDIR_SUBDIR, &hanp,
					       &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR_SUBDIR);
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
	 * TEST    : dm_path_to_handle - path too long
	 * EXPECTED: rc = -1, errno = ENAMETOOLONG
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 10)) {
		void *hanp;
		size_t hlen;
		char *szTooLong = PATH_TOOLONG;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(path too long)\n", szFuncName);
		rc = dm_path_to_handle(szTooLong, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENAMETOOLONG);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_handle - path includes invalid directory
	 * EXPECTED: rc = -1, errno = ENOTDIR
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(path not dir)\n",
				    szFuncName);
			rc = dm_path_to_handle(PATH_NOTDIR, &hanp, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOTDIR);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_path_to_handle - path not DMAPI
	 * EXPECTED: rc = -1, errno = ENXIO
	 *
	 * This variation uncovered XFS BUG #4 (EINVAL errno returned instead
	 * of ENXIO)
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 12)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(path not DMAPI)\n", szFuncName);
		rc = dm_path_to_handle(FILE_NOTDMAPI, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENXIO);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_handle - invalid hanpp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 13)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanpp)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_FILE,
					       (void **)INVALID_ADDR, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_path_to_handle - invalid hlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 14)) {
		int fd;
		void *hanp;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlenp)\n",
				    szFuncName);
			rc = dm_path_to_handle(DUMMY_FILE, &hanp,
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
		}
	}

	/*
	 * TEST    : dm_path_to_handle - different paths to same file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 15)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(%s)\n", szFuncName,
				    DUMMY_SUBDIR_FILE);
			rc = dm_path_to_handle(DUMMY_SUBDIR_FILE, &hanp1,
					       &hlen1);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp1 = %p, hlen1 = %d\n", hanp1,
					    hlen1);
				dm_LogHandle(hanp1, hlen1);
				DMLOG_PRINT(DMLVL_DEBUG, "%s(%s)\n", szFuncName,
					    DummySubdirFile);
				rc = dm_path_to_handle(DummySubdirFile, &hanp2,
						       &hlen2);
				if (rc == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "hanp2 = %p, hlen2 = %d\n",
						    hanp2, hlen2);
					dm_LogHandle(hanp2, hlen2);
				}
			}

			if (rc == 0) {
				if (dm_handle_cmp(hanp1, hlen1, hanp2, hlen2) ==
				    0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and handles same\n",
						    szFuncName, rc);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but handles NOT same\n",
						    szFuncName, rc);
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
			rc |= remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_path_to_handle - empty path
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 16)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(empty path)\n", szFuncName);
		rc = dm_path_to_handle("", &hanp, &hlen);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "hanp = %p, hlen = %d\n", hanp,
				    hlen);
			dm_LogHandle(hanp, hlen);

			if (dm_handle_cmp(mtpthanp, mtpthlen, hanp, hlen) == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, 0);
				DMVAR_PASS();
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with expected rc = %d but unexpected handle\n",
					    szFuncName, 0);
				DMVAR_PASS();
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with unexpected rc = %d (errno = %d)\n",
				    szFuncName, rc, errno);
			DMVAR_FAIL();
		}

		/* Variation clean up */
		dm_handle_free(hanp, hlen);
	}

	/*
	 * TEST    : dm_path_to_handle - current directory path
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_HANDLE_BASE + 17)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(curdir path)\n", szFuncName);
		rc = dm_path_to_handle(CURRENT_DIR, &hanp, &hlen);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "hanp = %p, hlen = %d\n", hanp,
				    hlen);
			dm_LogHandle(hanp, hlen);

			if (dm_handle_cmp(mtpthanp, mtpthlen, hanp, hlen) == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, 0);
				DMVAR_PASS();
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with expected rc = %d but unexpected handle\n",
					    szFuncName, 0);
				DMVAR_PASS();
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with unexpected rc = %d (errno = %d)\n",
				    szFuncName, rc, errno);
			DMVAR_FAIL();
		}

		/* Variation clean up */
		dm_handle_free(hanp, hlen);
	}

	szFuncName = "dm_fd_to_handle";

	/*
	 * TEST    : dm_fd_to_handle - invalid fd
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 1)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid fd)\n", szFuncName);
		rc = dm_fd_to_handle(INVALID_ADDR, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_fd_to_handle - file fd in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in curdir)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
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
	 * TEST    : dm_fd_to_handle - link fd in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 3)) {
		int fd_f, fd_l;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd_f =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = link(DUMMY_FILE, DUMMY_LINK)) == -1) {
			close(fd_f);
			remove(DUMMY_FILE);
		} else if ((fd_l = open(DUMMY_FILE, O_RDWR)) == -1) {
			unlink(DUMMY_LINK);
			close(fd_f);
			remove(DUMMY_FILE);
		}
		if (fd_f == -1 || rc == -1 || fd_l == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in curdir)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd_l, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd_f);
			rc |= close(fd_l);
			rc |= remove(DUMMY_FILE);
			rc |= remove(DUMMY_LINK);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_fd_to_handle - directory fd in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_SUBDIR, O_DIRECTORY)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir in curdir)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_fd_to_handle - file fd in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in subdir)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_SUBDIR_FILE);
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
	 * TEST    : dm_fd_to_handle - link fd in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 6)) {
		int fd_f, fd_l;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd_f =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = link(DUMMY_SUBDIR_FILE, DUMMY_SUBDIR_LINK)) ==
			   -1) {
			remove(DUMMY_SUBDIR_FILE);
			close(fd_f);
			rmdir(DUMMY_SUBDIR);
		} else if ((fd_l = open(DUMMY_SUBDIR_FILE, O_RDWR)) == -1) {
			unlink(DUMMY_SUBDIR_LINK);
			close(fd_f);
			remove(DUMMY_SUBDIR_FILE);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd_f == -1 || fd_l == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in subdir)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd_l, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd_f);
			rc |= close(fd_l);
			rc |= remove(DUMMY_SUBDIR_FILE);
			rc |= unlink(DUMMY_SUBDIR_LINK);
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
	 * TEST    : dm_fd_to_handle - directory fd in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DUMMY_SUBDIR_SUBDIR, DUMMY_DIR_RW_MODE))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((fd = open(DUMMY_SUBDIR_SUBDIR, O_DIRECTORY)) == -1) {
			rmdir(DUMMY_SUBDIR_SUBDIR);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir in subdir)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= rmdir(DUMMY_SUBDIR_SUBDIR);
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
	 * TEST    : dm_fd_to_handle - fd not DMAPI
	 * EXPECTED: rc = -1, errno = ENXIO
	 *
	 * This variation uncovered XFS BUG #27 (EBADF errno returned instead
	 * of ENXIO)
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		fd = open(FILE_NOTDMAPI, O_RDONLY);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fd not DMAPI)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENXIO);

			/* Variation clean up */
			rc = close(fd);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_fd_to_handle - invalid hanpp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 9)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanpp)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, (void **)INVALID_ADDR, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_fd_to_handle - invalid hlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 10)) {
		int fd;
		void *hanp;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlenp)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp,
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
		}
	}

	/*
	 * TEST    : dm_fd_to_handle - stdin fd
	 * EXPECTED: rc = -1, errno = ENXIO
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 11)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(stdin fd)\n", szFuncName);
		rc = dm_fd_to_handle(0, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENXIO);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_fd_to_handle - stdout fd
	 * EXPECTED: rc = -1, errno = ENXIO
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 12)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(stdout fd)\n", szFuncName);
		rc = dm_fd_to_handle(1, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENXIO);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_fd_to_handle - stderr fd
	 * EXPECTED: rc = -1, errno = ENXIO
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 13)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(stderr fd)\n", szFuncName);
		rc = dm_fd_to_handle(2, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENXIO);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_fd_to_handle - invalidated fd
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(FD_TO_HANDLE_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) != -1) {
			rc = close(fd);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated fd)\n",
				    szFuncName);
			rc = dm_fd_to_handle(fd, &hanp, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	szFuncName = "dm_path_to_fshandle";

	/*
	 * TEST    : dm_path_to_fshandle - invalid path
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #5 (0 return code from strnlen_user
	 * ignored, which indicated fault)
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 1)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid path)\n", szFuncName);
		rc = dm_path_to_fshandle((char *)INVALID_ADDR, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_fshandle - nonexistent path in current directory
	 * EXPECTED: rc = -1, errno = ENOENT
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 2)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(nonexistent path in curdir)\n",
			    szFuncName);
		rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_fshandle - file in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in curdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
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
	 * TEST    : dm_path_to_fshandle - link in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = link(DUMMY_FILE, DUMMY_LINK)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in curdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_LINK, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			rc |= unlink(DUMMY_LINK);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_path_to_fshandle - directory in current directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 5)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir in curdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_SUBDIR, &hanp, &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_path_to_fshandle - nonexistent path in subdirectory
	 * EXPECTED: rc = -1, errno = ENOENT
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 6)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(nonexistent path in subdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_SUBDIR_FILE, &hanp,
						 &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_path_to_fshandle - file in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in subdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_SUBDIR_FILE, &hanp,
						 &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_SUBDIR_FILE);
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
	 * TEST    : dm_path_to_fshandle - link in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = link(DUMMY_SUBDIR_FILE, DUMMY_SUBDIR_LINK)) ==
			   -1) {
			close(fd);
			remove(DUMMY_SUBDIR_FILE);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in subdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_SUBDIR_LINK, &hanp,
						 &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_SUBDIR_FILE);
			rc |= unlink(DUMMY_SUBDIR_LINK);
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
	 * TEST    : dm_path_to_fshandle - directory in subdirectory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 9)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DUMMY_SUBDIR_SUBDIR, DUMMY_DIR_RW_MODE))
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir in subdir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_SUBDIR_SUBDIR, &hanp,
						 &hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp,
					    hlen);
				dm_LogHandle(hanp, hlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR_SUBDIR);
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
	 * TEST    : dm_path_to_fshandle - path too long
	 * EXPECTED: rc = -1, errno = ENAMETOOLONG
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 10)) {
		void *hanp;
		size_t hlen;
		char *szTooLong = PATH_TOOLONG;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(path too long)\n", szFuncName);
		rc = dm_path_to_fshandle(szTooLong, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENAMETOOLONG);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_fshandle - path includes invalid directory
	 * EXPECTED: rc = -1, errno = ENOTDIR
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(path not dir)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(PATH_NOTDIR, &hanp, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOTDIR);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_path_to_fshandle - path not DMAPI
	 * EXPECTED: rc = -1, errno = ENXIO
	 *
	 * This variation uncovered XFS BUG #6 (EINVAL errno returned instead
	 * of ENXIO)
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 12)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(path not DMAPI)\n", szFuncName);
		rc = dm_path_to_fshandle(FILE_NOTDMAPI, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENXIO);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_path_to_fshandle - invalid hanpp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 13)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		size_t hlen;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanpp)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_FILE,
						 (void **)INVALID_ADDR, &hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_path_to_fshandle - invalid hlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 14)) {
		int fd;
		void *hanp;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlenp)\n",
				    szFuncName);
			rc = dm_path_to_fshandle(DUMMY_FILE, &hanp,
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
		}
	}

	/*
	 * TEST    : dm_path_to_fshandle - empty path
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 15)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(empty path)\n", szFuncName);
		rc = dm_path_to_fshandle("", &hanp, &hlen);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "hanp = %p, hlen = %d\n", hanp,
				    hlen);
			dm_LogHandle(hanp, hlen);
		}
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		dm_handle_free(hanp, hlen);
	}

	/*
	 * TEST    : dm_path_to_fshandle - current directory  path
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PATH_TO_FSHANDLE_BASE + 16)) {
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(curdir path)\n", szFuncName);
		rc = dm_path_to_fshandle(CURRENT_DIR, &hanp, &hlen);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "hanp = %p, hlen = %d\n", hanp,
				    hlen);
			dm_LogHandle(hanp, hlen);
		}
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		dm_handle_free(hanp, hlen);
	}

	szFuncName = "dm_handle_to_fshandle";

	/*
	 * TEST    : dm_handle_to_fshandle - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_handle_to_fshandle((void *)INVALID_ADDR, hlen,
						   &fshanp, &fshlen);
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_fshandle - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 2)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_handle_to_fshandle(hanp, INVALID_ADDR, &fshanp,
						   &fshlen);
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
	 * TEST    : dm_handle_to_fshandle - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 3)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_handle_to_fshandle(hanp, hlen, &fshanp,
						   &fshlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "fshanp = %p, fshlen = %d\n",
					    fshanp, fshlen);
				dm_LogHandle(fshanp, fshlen);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_fshandle - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 4)) {
		void *hanp, *fshanp;
		size_t hlen, fshlen;

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
			rc = dm_handle_to_fshandle(hanp, hlen, &fshanp,
						   &fshlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "fshanp = %p, fshlen = %d\n",
					    fshanp, fshlen);
				dm_LogHandle(fshanp, fshlen);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_fshandle - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 5)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_handle_to_fshandle(hanp, hlen, &fshanp,
						   &fshlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "fshanp = %p, fshlen = %d\n",
					    fshanp, fshlen);
				dm_LogHandle(fshanp, fshlen);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_fshandle - invalid fshanpp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 6)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp;
		size_t hlen, fshlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid fshanpp)\n",
				    szFuncName);
			rc = dm_handle_to_fshandle(hanp, hlen,
						   (void **)INVALID_ADDR,
						   &fshlen);
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_fshandle - invalid fshlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 7)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp, *fshanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid fshlenp)\n",
				    szFuncName);
			rc = dm_handle_to_fshandle(hanp, hlen, &fshanp,
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_fshandle - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSHANDLE_BASE + 8)) {
		void *fshanp;
		size_t fshlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_handle_to_fshandle(DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					   &fshanp, &fshlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	szFuncName = "dm_handle_cmp";

	/*
	 * TEST    : dm_handle_cmp - invalid hanp1
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp1)\n",
				    szFuncName);
			rc = dm_handle_cmp((char *)INVALID_ADDR, hlen, hanp,
					   hlen);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_cmp - invalid hlen1
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen1)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, INVALID_ADDR, hanp, hlen);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
	 * TEST    : dm_handle_cmp - invalid hanp2
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 3)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp2)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, hlen, (char *)INVALID_ADDR,
					   hlen);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_cmp - invalid hlen2
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen2)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, hlen, hanp, INVALID_ADDR);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
	 * TEST    : dm_handle_cmp - hlen1 < hlen2
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(hlen1 < hlen2)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, hlen, hanp, hlen + 1);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (same file handles)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 == hanp2, same file handle)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, hlen, hanp, hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (same fs handles)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 == hanp2, same fs handle)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, hlen, hanp, hlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
	 * TEST    : dm_handle_cmp - hlen1 > hlen2
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(hlen1 > hlen2)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp, hlen, hanp, hlen - 1);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (different file handles, same path)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 9)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp1, &hlen1))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp2, &hlen2))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 == hanp2, diff file handles from same path)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (different fs handles, same path)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 10)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &hanp1,
					     &hlen1)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &hanp2,
					     &hlen2)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 == hanp2, diff fs handles from same path)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (different file handles, one path, one fd)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 11)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp1, &hlen1))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp2, &hlen2)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 == hanp2, diff file handles from path, fd)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (different file handles, same fd)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 12)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp1, &hlen1)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp2, &hlen2)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 == hanp2, diff file handles from same fd)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 != hanp2 (different path)
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 13)) {
		int fd1, fd2;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd1 =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp1, &hlen1))
			   == -1) {
			close(fd1);
			remove(DUMMY_FILE);
		} else
		    if ((fd2 =
			 open(DUMMY_FILE2, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			close(fd1);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		} else if ((rc = dm_path_to_handle(DUMMY_FILE2, &hanp2, &hlen2))
			   == -1) {
			close(fd1);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
			close(fd2);
			remove(DUMMY_FILE2);
		}
		if (fd1 == -1 || rc == -1 || fd2 == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 != hanp2, different paths)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d\n",
					    szFuncName, rc);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd1);
			rc |= remove(DUMMY_FILE);
			rc |= close(fd2);
			rc |= remove(DUMMY_FILE2);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 != hanp2 (different fd)
	 * EXPECTED: rc != 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 14)) {
		int fd1, fd2;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd1 =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd1, &hanp1, &hlen1)) == -1) {
			close(fd1);
			remove(DUMMY_FILE);
		} else
		    if ((fd2 =
			 open(DUMMY_FILE2, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			close(fd1);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		} else if ((rc = dm_fd_to_handle(fd2, &hanp2, &hlen2)) == -1) {
			close(fd1);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
			close(fd2);
			remove(DUMMY_FILE2);
		}
		if (fd1 == -1 || rc == -1 || fd2 == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 != hanp2, different fd's)\n",
				    szFuncName);
			rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d\n",
					    szFuncName, rc);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = close(fd1);
			rc |= remove(DUMMY_FILE);
			rc |= close(fd2);
			rc |= remove(DUMMY_FILE2);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 == hanp2 (global handle)
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #41 (fault occurred instead of
	 * rc = 0)
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 15)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(hanp1 == hanp2, global handle)\n",
			    szFuncName);
		rc = dm_handle_cmp(DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_GLOBAL_HANP, DM_GLOBAL_HLEN);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s passed with expected rc = %d\n",
				    szFuncName, rc);
			DMVAR_PASS();
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with unexpected rc = %d\n",
				    szFuncName, rc);
			DMVAR_FAIL();
		}

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_handle_cmp - hanp1 != hanp2 (file and global handle)
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_CMP_BASE + 16)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(hanp1 != hanp2, file and global handle)\n",
				    szFuncName);
			rc = dm_handle_cmp(DM_GLOBAL_HANP, DM_GLOBAL_HLEN, hanp,
					   hlen);
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				DMVAR_PASS();
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

	szFuncName = "dm_handle_free";

	/*
	 * TEST    : dm_handle_free - invalid hanp
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		dm_handle_free((char *)INVALID_ADDR, FILE_HANDLELEN);
		DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
		DMVAR_PASS();

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_free - file handle from path
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle from path)\n",
				    szFuncName);
			dm_handle_free(hanp, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
			DMVAR_PASS();

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_handle_free - file handle from fd
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle from fd)\n",
				    szFuncName);
			dm_handle_free(hanp, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
			DMVAR_PASS();

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_handle_free - fs handle from path
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from path)\n",
				    szFuncName);
			dm_handle_free(hanp, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
			DMVAR_PASS();

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_handle_free - fs handle from handle
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 5)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp1, &hlen1)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp1, hlen1, &hanp2,
					       &hlen2)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from handle)\n",
				    szFuncName);
			dm_handle_free(hanp2, hlen2);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
			DMVAR_PASS();

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp1, hlen1);
		}
	}

	/*
	 * TEST    : dm_handle_free - file handle from make
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 6)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;
		dm_fsid_t fsid;
		dm_igen_t igen;
		dm_ino_t ino;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp1, &hlen1))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if (((rc = dm_handle_to_fsid(hanp1, hlen1, &fsid)) == -1)
			   || ((rc = dm_handle_to_igen(hanp1, hlen1, &igen)) ==
			       -1)
			   || ((rc = dm_handle_to_ino(hanp1, hlen1, &ino)) ==
			       -1)
			   ||
			   ((rc =
			     dm_make_handle(&fsid, &ino, &igen, &hanp2,
					    &hlen2)) == -1)) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle from make)\n",
				    szFuncName);
			dm_handle_free(hanp2, hlen2);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
			DMVAR_PASS();

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_handle_free - fs handle from make
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 7)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;
		dm_fsid_t fsid;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &hanp1,
					     &hlen1)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if (((rc = dm_handle_to_fsid(hanp1, hlen1, &fsid)) == -1)
			   || ((rc = dm_make_fshandle(&fsid, &hanp2, &hlen2)) ==
			       -1)) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp1, hlen1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from make)\n",
				    szFuncName);
			dm_handle_free(hanp2, hlen2);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
			DMVAR_PASS();

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp1, hlen1);
		}
	}

	/*
	 * TEST    : dm_handle_free - global handle
	 * EXPECTED: return
	 */
	if (DMVAR_EXEC(HANDLE_FREE_BASE + 8)) {
#ifdef USER_SPACE_FAULTS
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		dm_handle_free(DM_GLOBAL_HANP, DM_GLOBAL_HLEN);
		DMLOG_PRINT(DMLVL_DEBUG, "%s passed\n", szFuncName);
		DMVAR_PASS();

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	szFuncName = "dm_handle_is_valid";

	/*
	 * TEST    : dm_handle_is_valid - invalid hanp
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		bRC = dm_handle_is_valid((char *)INVALID_ADDR, FILE_HANDLELEN);
		DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_is_valid - file handle
	 * EXPECTED: rc = DM_TRUE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			bRC = dm_handle_is_valid(hanp, hlen);
			DMVAR_ENDPASSEXP(szFuncName, DM_TRUE, bRC);

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
	 * TEST    : dm_handle_is_valid - file handle, hlen too small
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file hlen too small)\n",
				    szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen - 1);
			DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

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
	 * TEST    : dm_handle_is_valid - file handle, hlen too big
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file hlen too big)\n",
				    szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen + 1);
			DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

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
	 * TEST    : dm_handle_is_valid - modified file handle
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			memset(hanp, 0, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(modified file handle)\n",
				    szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen);
			DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

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
	 * TEST    : dm_handle_is_valid - fs handle
	 * EXPECTED: rc = DM_TRUE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen);
			DMVAR_ENDPASSEXP(szFuncName, DM_TRUE, bRC);

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
	 * TEST    : dm_handle_is_valid - fs handle, hlen too small
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs hlen too small)\n",
				    szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen - 1);
			DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

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
	 * TEST    : dm_handle_is_valid - fs handle, hlen too big
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs hlen too big)\n",
				    szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen + 1);
			DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

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
	 * TEST    : dm_handle_is_valid - modified fs handle
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			memset(hanp, 0, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s(modified fs handle)\n",
				    szFuncName);
			bRC = dm_handle_is_valid(hanp, hlen);
			DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

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
	 * TEST    : dm_handle_is_valid - global handle
	 * EXPECTED: rc = DM_TRUE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 10)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		bRC = dm_handle_is_valid(DM_GLOBAL_HANP, DM_GLOBAL_HLEN);
		DMVAR_ENDPASSEXP(szFuncName, DM_TRUE, bRC);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_handle_is_valid - invalid handle
	 * EXPECTED: rc = DM_FALSE
	 */
	if (DMVAR_EXEC(HANDLE_IS_VALID_BASE + 11)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid handle)\n", szFuncName);
		bRC = dm_handle_is_valid(DM_INVALID_HANP, DM_INVALID_HLEN);
		DMVAR_ENDPASSEXP(szFuncName, DM_FALSE, bRC);

		/* Variation clean up */
	}

	szFuncName = "dm_handle_hash";

	/*
	 * TEST    : dm_handle_hash - invalid hanp
	 * EXPECTED: rc = ?
	 */
	if (DMVAR_EXEC(HANDLE_HASH_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_handle_hash((char *)INVALID_ADDR, FILE_HANDLELEN);
		DMLOG_PRINT(DMLVL_DEBUG, "%s passed with rc = %u\n",
			    szFuncName);
		DMVAR_PASS();

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_hash - file handle
	 * EXPECTED: rc = ?
	 */
	if (DMVAR_EXEC(HANDLE_HASH_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_handle_hash(hanp, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed with rc = %u\n",
				    szFuncName);
			DMVAR_PASS();

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
	 * TEST    : dm_handle_hash - fs handle
	 * EXPECTED: rc = ?
	 */
	if (DMVAR_EXEC(HANDLE_HASH_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_handle_hash(hanp, hlen);
			DMLOG_PRINT(DMLVL_DEBUG, "%s passed with rc = %u\n",
				    szFuncName);
			DMVAR_PASS();

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
	 * TEST    : dm_handle_hash - global handle
	 * EXPECTED: rc = ?
	 */
	if (DMVAR_EXEC(HANDLE_HASH_BASE + 4)) {
#ifdef USER_SPACE_FAULTS
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_handle_hash(DM_GLOBAL_HANP, DM_GLOBAL_HLEN);
		DMLOG_PRINT(DMLVL_DEBUG, "%s passed with rc = %u\n",
			    szFuncName);
		DMVAR_PASS();

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	szFuncName = "dm_handle_to_fsid";

	/*
	 * TEST    : dm_handle_to_fsid - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSID_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		dm_fsid_t fsidp;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_handle_to_fsid((char *)INVALID_ADDR, FILE_HANDLELEN,
				       &fsidp);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_fsid - invalid fsidp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSID_BASE + 2)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid fsidp)\n",
				    szFuncName);
			rc = dm_handle_to_fsid(hanp, hlen,
					       (dm_fsid_t *) INVALID_ADDR);
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_fsid - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSID_BASE + 3)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_fsid_t fsidp;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &fshanp,
					     &fshlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp, hlen);
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
			rc = dm_handle_to_fsid(hanp, hlen, &fsidp);
			if (rc == 0) {
				if (memcmp(hanp, &fsidp, sizeof(dm_fsid_t)) ==
				    0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, rc);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected fsid (0x%16llX vs 0x%16llX)\n",
						    szFuncName, rc, fsidp,
						    *(dm_fsid_t *) hanp);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_fsid - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSID_BASE + 4)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_fsid_t fsidp;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &fshanp,
					     &fshlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_handle_to_fsid(hanp, hlen, &fsidp);
			if (rc == 0) {
				if (memcmp(hanp, &fsidp, sizeof(dm_fsid_t)) ==
				    0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, rc);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected fsid (0x%16llX vs 0x%16llX)\n",
						    szFuncName, rc, fsidp,
						    *(dm_fsid_t *) hanp);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_fsid - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_FSID_BASE + 5)) {
		dm_fsid_t fsidp;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_handle_to_fsid(DM_GLOBAL_HANP, DM_GLOBAL_HLEN, &fsidp);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	szFuncName = "dm_handle_to_igen";

	/*
	 * TEST    : dm_handle_to_igen - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		dm_igen_t igen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_handle_to_igen((char *)INVALID_ADDR, FILE_HANDLELEN,
				       &igen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_igen - invalid igenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 2)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid igenp)\n",
				    szFuncName);
			rc = dm_handle_to_igen(hanp, hlen,
					       (dm_igen_t *) INVALID_ADDR);
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_igen - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_igen_t igen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_handle_to_igen(hanp, hlen, &igen);
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
	 * TEST    : dm_handle_to_igen - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_igen_t igen;

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
			rc = dm_handle_to_igen(hanp, hlen, &igen);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

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
	 * TEST    : dm_handle_to_igen - fs handle from file
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_igen_t igen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from file)\n",
				    szFuncName);
			rc = dm_handle_to_igen(hanp, hlen, &igen);
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
	 * TEST    : dm_handle_to_igen - fs handle from directory
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 6)) {
		void *hanp;
		size_t hlen;
		dm_igen_t igen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_SUBDIR, &hanp,
					     &hlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from dir)\n",
				    szFuncName);
			rc = dm_handle_to_igen(hanp, hlen, &igen);
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
	 * TEST    : dm_handle_to_igen - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_IGEN_BASE + 7)) {
		dm_igen_t igen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_handle_to_igen(DM_GLOBAL_HANP, DM_GLOBAL_HLEN, &igen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	szFuncName = "dm_handle_to_ino";

	/*
	 * TEST    : dm_handle_to_ino - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		dm_ino_t ino;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_handle_to_ino((char *)INVALID_ADDR, FILE_HANDLELEN,
				      &ino);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_ino - invalid inop
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 2)) {
#ifdef USER_SPACE_FAULTS
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid inop)\n",
				    szFuncName);
			rc = dm_handle_to_ino(hanp, hlen,
					      (dm_ino_t *) INVALID_ADDR);
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
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_handle_to_ino - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_handle_to_ino(hanp, hlen, &ino);
			if (rc == 0) {
				struct stat statfs;

				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				rc = stat(DUMMY_FILE, &statfs);
				if (rc == 0) {
					if (ino == statfs.st_ino) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "ino %d from stat() matches returned value\n",
							    statfs.st_ino);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "BUT... ino %d from stat() does not match returned value %lld\n",
							    statfs.st_ino, ino);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "BUT... stat() failed with rc = %d (errno = %d)\n",
						    rc, errno);
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
	 * TEST    : dm_handle_to_ino - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

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
			rc = dm_handle_to_ino(hanp, hlen, &ino);
			if (rc == 0) {
				struct stat statfs;

				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, rc);
				rc = stat(DUMMY_SUBDIR, &statfs);
				if (rc == 0) {
					if (ino == statfs.st_ino) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "ino %d from stat() matches returned value\n",
							    statfs.st_ino);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "BUT... ino %d from stat() does not match returned value %lld\n",
							    statfs.st_ino, ino);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "BUT... stat() failed with rc = %d (errno = %d)\n",
						    rc, errno);
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
	 * TEST    : dm_handle_to_ino - fs handle from file
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from file)\n",
				    szFuncName);
			rc = dm_handle_to_ino(hanp, hlen, &ino);
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
	 * TEST    : dm_handle_to_ino - fs handle from directory
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 6)) {
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_SUBDIR, &hanp,
					     &hlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle from dir)\n",
				    szFuncName);
			rc = dm_handle_to_ino(hanp, hlen, &ino);
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
	 * TEST    : dm_handle_to_ino - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_INO_BASE + 7)) {
		dm_ino_t ino;
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_handle_to_ino(DM_GLOBAL_HANP, DM_GLOBAL_HLEN, &ino);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	szFuncName = "dm_make_handle";

	/*
	 * TEST    : dm_make_handle - invalid fsidp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		void *hanp;
		size_t hlen;
		dm_ino_t ino;
		dm_igen_t igen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid fsidp)\n", szFuncName);
		rc = dm_make_handle((dm_fsid_t *) INVALID_ADDR, &ino, &igen,
				    &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_handle - invalid inop
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 2)) {
#ifdef USER_SPACE_FAULTS
		void *hanp;
		size_t hlen;
		dm_fsid_t fsid;
		dm_igen_t igen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid inop)\n", szFuncName);
		rc = dm_make_handle(&fsid, (dm_ino_t *) INVALID_ADDR, &igen,
				    &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_handle - invalid igenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 3)) {
#ifdef USER_SPACE_FAULTS
		void *hanp;
		size_t hlen;
		dm_fsid_t fsid;
		dm_ino_t ino;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid igenp)\n", szFuncName);
		rc = dm_make_handle(&fsid, &ino, (dm_igen_t *) INVALID_ADDR,
				    &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_handle - invalid hanpp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 4)) {
#ifdef USER_SPACE_FAULTS
		size_t hlen;
		dm_fsid_t fsid;
		dm_igen_t igen;
		dm_ino_t ino;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanpp)\n", szFuncName);
		rc = dm_make_handle(&fsid, &ino, &igen, (void **)INVALID_ADDR,
				    &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_handle - invalid hlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 5)) {
#ifdef USER_SPACE_FAULTS
		void *hanp;
		dm_fsid_t fsid;
		dm_igen_t igen;
		dm_ino_t ino;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlenp)\n", szFuncName);
		rc = dm_make_handle(&fsid, &ino, &igen, &hanp,
				    (size_t *) INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_handle - file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 6)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;
		dm_fsid_t fsid;
		dm_igen_t igen;
		dm_ino_t ino;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_FILE, &hanp1, &hlen1))
			   == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if (((rc = dm_handle_to_fsid(hanp1, hlen1, &fsid)) == -1)
			   || ((rc = dm_handle_to_igen(hanp1, hlen1, &igen)) ==
			       -1)
			   || ((rc = dm_handle_to_ino(hanp1, hlen1, &ino)) ==
			       -1)) {
			dm_handle_free(hanp1, hlen1);
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file)\n", szFuncName);
			rc = dm_make_handle(&fsid, &ino, &igen, &hanp2, &hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp2,
					    hlen2);
				dm_LogHandle(hanp2, hlen2);

				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = 0 but unexpected dm_handle_cmp rc = %d\n",
						    szFuncName, rc);
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_make_handle - directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(MAKE_HANDLE_BASE + 7)) {
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;
		dm_fsid_t fsid;
		dm_igen_t igen;
		dm_ino_t ino;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &hanp1,
					   &hlen1)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if (((rc = dm_handle_to_fsid(hanp1, hlen1, &fsid)) == -1)
			   || ((rc = dm_handle_to_igen(hanp1, hlen1, &igen)) ==
			       -1)
			   || ((rc = dm_handle_to_ino(hanp1, hlen1, &ino)) ==
			       -1)) {
			dm_handle_free(hanp1, hlen1);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir)\n", szFuncName);
			rc = dm_make_handle(&fsid, &ino, &igen, &hanp2, &hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp2,
					    hlen2);
				dm_LogHandle(hanp2, hlen2);

				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = 0 but unexpected dm_handle_cmp rc = %d\n",
						    szFuncName, rc);
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	szFuncName = "dm_make_fshandle";

	/*
	 * TEST    : dm_make_fshandle - invalid fsidp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_FSHANDLE_BASE + 1)) {
#ifdef USER_SPACE_FAULTS
		void *hanp;
		size_t hlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid fsidp)\n", szFuncName);
		rc = dm_make_fshandle((dm_fsid_t *) INVALID_ADDR, &hanp, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_fshandle - invalid hanpp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_FSHANDLE_BASE + 2)) {
#ifdef USER_SPACE_FAULTS
		size_t hlen;
		dm_fsid_t fsid;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanpp)\n", szFuncName);
		rc = dm_make_fshandle(&fsid, (void **)INVALID_ADDR, &hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_fshandle - invalid hlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(MAKE_FSHANDLE_BASE + 3)) {
#ifdef USER_SPACE_FAULTS
		void *hanp;
		dm_fsid_t fsid;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlenp)\n", szFuncName);
		rc = dm_make_fshandle(&fsid, &hanp, (size_t *) INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with USER_SPACE_FAULTS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_make_fshandle - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(MAKE_FSHANDLE_BASE + 4)) {
		int fd;
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;
		dm_fsid_t fsid;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &hanp1,
					     &hlen1)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = dm_handle_to_fsid(hanp1, hlen1, &fsid)) == -1) {
			dm_handle_free(hanp1, hlen1);
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
			rc = dm_make_fshandle(&fsid, &hanp2, &hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp2,
					    hlen2);
				dm_LogHandle(hanp2, hlen2);

				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = 0 but unexpected dm_handle_cmp rc = %d\n",
						    szFuncName, rc);
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	/*
	 * TEST    : dm_make_fshandle - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(MAKE_FSHANDLE_BASE + 5)) {
		void *hanp1, *hanp2;
		size_t hlen1, hlen2;
		dm_fsid_t fsid;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_SUBDIR, &hanp1,
					     &hlen1)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = dm_handle_to_fsid(hanp1, hlen1, &fsid)) == -1) {
			dm_handle_free(hanp1, hlen1);
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
			rc = dm_make_fshandle(&fsid, &hanp2, &hlen2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "hanp = %p, hlen = %d\n", hanp2,
					    hlen2);
				dm_LogHandle(hanp2, hlen2);

				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = 0 but unexpected dm_handle_cmp rc = %d\n",
						    szFuncName, rc);
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
			dm_handle_free(hanp1, hlen1);
			dm_handle_free(hanp2, hlen2);
		}
	}

	szFuncName = "dm_handle_to_path";

	/*
	 * TEST    : dm_handle_to_path - invalid dirhanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 1)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid dirhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path((void *)INVALID_ADDR, dirhlen,
					       targhanp, targhlen, PATHBUF_LEN,
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalid dirhlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 2)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid dirhlen)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, INVALID_ADDR, targhanp,
					       targhlen, PATHBUF_LEN, pathbuf,
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
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalid targhanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 3)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid targhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen,
					       (void *)INVALID_ADDR, targhlen,
					       PATHBUF_LEN, pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalid targhlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 4)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid targhlen)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       INVALID_ADDR, PATHBUF_LEN,
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 5)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
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
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, 1, pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalid pathbufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 6)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid pathbufp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, PATHBUF_LEN,
					       (char *)INVALID_ADDR, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 7)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
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
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, PATHBUF_LEN, pathbuf,
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
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - file dirhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 8)) {
		int fd1, fd2;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd1 =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd1, &targhanp, &targhlen)) ==
			   -1) {
			close(fd1);
			remove(DUMMY_FILE);
		} else
		    if ((fd2 =
			 open(DUMMY_FILE2, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			close(fd1);
			remove(DUMMY_FILE);
			dm_handle_free(targhanp, targhlen);
		} else if ((rc = dm_fd_to_handle(fd2, &dirhanp, &dirhlen)) ==
			   -1) {
			close(fd2);
			remove(DUMMY_FILE2);
			close(fd1);
			remove(DUMMY_FILE);
			dm_handle_free(targhanp, targhlen);
		}
		if (fd1 == -1 || fd2 == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file dirhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd1);
			rc |= close(fd2);
			rc |= remove(DUMMY_FILE);
			rc |= remove(DUMMY_FILE2);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - directory targhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 9)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &targhanp,
					   &targhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir targhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - absolute root directory
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #12 (only worked if dirhanp was
	 * current directory)
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 10)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(absolute root dir)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "rlen = %d, pathbuf = \"%s\"\n",
					    rlen, pathbuf);

				if (strncmp(pathbuf, DUMMY_FILE, rlen) == 0) {
					*(pathbuf + rlen) = 0;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and path = %s (length %d)\n",
						    szFuncName, rc, pathbuf,
						    rlen);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected path (%s vs %s)\n",
						    szFuncName, rc, pathbuf,
						    DUMMY_FILE);
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
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - relative root directory
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 11)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle("", &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(relative root dir)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "rlen = %d, pathbuf = \"%s\"\n",
					    rlen, pathbuf);

				if (strncmp(pathbuf, DUMMY_FILE, rlen) == 0) {
					*(pathbuf + rlen) = 0;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and path = %s (length %d)\n",
						    szFuncName, rc, pathbuf,
						    rlen);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected path (%s vs %s)\n",
						    szFuncName, rc, pathbuf,
						    DUMMY_FILE);
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
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - file in subdirectory, one level
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 12)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dirhanp,
					   &dirhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file in subdir)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "rlen = %d, pathbuf = \"%s\"\n",
					    rlen, pathbuf);

				if (strncmp(pathbuf, DUMMY_SUBDIR_FILE, rlen) ==
				    0) {
					*(pathbuf + rlen) = 0;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and path = %s (length %d)\n",
						    szFuncName, rc, pathbuf,
						    rlen);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected path (%s vs %s)\n",
						    szFuncName, rc, pathbuf,
						    DUMMY_SUBDIR_FILE);
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
			rc |= remove(DUMMY_SUBDIR_FILE);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - link in subdirectory, one level
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 13)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dirhanp,
					   &dirhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = link(DUMMY_SUBDIR_FILE, DUMMY_SUBDIR_LINK)) ==
			   -1) {
			close(fd);
			rmdir(DUMMY_SUBDIR);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR_LINK, &targhanp,
					   &targhlen)) == -1) {
			unlink(DUMMY_SUBDIR_LINK);
			close(fd);
			rmdir(DUMMY_SUBDIR);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(link in subdir)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "rlen = %d, pathbuf = \"%s\"\n",
					    rlen, pathbuf);

				if (strncmp(pathbuf, DUMMY_SUBDIR_LINK, rlen) ==
				    0) {
					*(pathbuf + rlen) = 0;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and path = %s (length %d)\n",
						    szFuncName, rc, pathbuf,
						    rlen);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected path (%s vs %s)\n",
						    szFuncName, rc, pathbuf,
						    DUMMY_SUBDIR_LINK);
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
			rc |= remove(DUMMY_SUBDIR_FILE);
			rc |= unlink(DUMMY_SUBDIR_LINK);
			rc |= rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - file in subdirectory, multiple levels
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 14)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DIR_LEVEL1, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DIR_LEVEL2, DUMMY_DIR_RW_MODE)) == -1) {
			rmdir(DIR_LEVEL1);
		} else if ((rc = mkdir(DIR_LEVEL3, DUMMY_DIR_RW_MODE)) == -1) {
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else if ((rc = mkdir(DIR_LEVEL4, DUMMY_DIR_RW_MODE)) == -1) {
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else
		    if ((rc =
			 dm_path_to_handle(DIR_LEVEL4, &dirhanp,
					   &dirhlen)) == -1) {
			rmdir(DIR_LEVEL4);
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else
		    if ((fd =
			 open(FILE_LEVEL4, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DIR_LEVEL4);
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else
		    if ((rc =
			 dm_path_to_handle(FILE_LEVEL4, &targhanp,
					   &targhlen)) == -1) {
			close(fd);
			remove(FILE_LEVEL4);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DIR_LEVEL4);
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(file in multiple subdir)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "rlen = %d, pathbuf = \"%s\"\n",
					    rlen, pathbuf);

				if (strncmp(pathbuf, FILE_LEVEL4, rlen) == 0) {
					*(pathbuf + rlen) = 0;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and path = %s (length %d)\n",
						    szFuncName, rc, pathbuf,
						    rlen);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected path (%s vs %s)\n",
						    szFuncName, rc, pathbuf,
						    FILE_LEVEL4);
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
			rc |= remove(FILE_LEVEL4);
			rc |= rmdir(DIR_LEVEL4);
			rc |= rmdir(DIR_LEVEL3);
			rc |= rmdir(DIR_LEVEL2);
			rc |= rmdir(DIR_LEVEL1);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - targhanp not in dirhanp
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 15)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DIR_LEVEL1, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DIR_LEVEL2, DUMMY_DIR_RW_MODE)) == -1) {
			rmdir(DIR_LEVEL1);
		} else if ((rc = mkdir(DIR_LEVEL3, DUMMY_DIR_RW_MODE)) == -1) {
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else
		    if ((rc =
			 dm_path_to_handle(DIR_LEVEL3, &dirhanp,
					   &dirhlen)) == -1) {
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else if ((rc = mkdir(DIR_LEVEL4, DUMMY_DIR_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else
		    if ((fd =
			 open(FILE_LEVEL4, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DIR_LEVEL4);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		} else
		    if ((rc =
			 dm_path_to_handle(FILE_LEVEL4, &targhanp,
					   &targhlen)) == -1) {
			close(fd);
			remove(FILE_LEVEL4);
			rmdir(DIR_LEVEL4);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DIR_LEVEL3);
			rmdir(DIR_LEVEL2);
			rmdir(DIR_LEVEL1);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(targhanp not in dirhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(FILE_LEVEL4);
			rc |= rmdir(DIR_LEVEL4);
			rc |= rmdir(DIR_LEVEL3);
			rc |= rmdir(DIR_LEVEL2);
			rc |= rmdir(DIR_LEVEL1);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - fs dirhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 16)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc =
		     dm_path_to_fshandle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs dirhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, PATHBUF_LEN, pathbuf,
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
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - fs targhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 17)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_FILE, &targhanp,
					     &targhlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs targhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, PATHBUF_LEN, pathbuf,
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
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - global dirhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 18)) {
		int fd;
		void *targhanp;
		size_t targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(global dirhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					       targhanp, targhlen, PATHBUF_LEN,
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - global targhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 19)) {
		void *dirhanp;
		size_t dirhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		rc = dm_path_to_handle(mountPt, &dirhanp, &dirhlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(global targhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, DM_GLOBAL_HANP,
					       DM_GLOBAL_HLEN, PATHBUF_LEN,
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(dirhanp, dirhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalidated dirhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 20)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dirhanp,
					   &dirhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((fd =
			 open(DUMMY_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = rmdir(DUMMY_SUBDIR)) == -1) {
			dm_handle_free(targhanp, targhlen);
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(dirhanp, dirhlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated dirhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	/*
	 * TEST    : dm_handle_to_path - invalidated targhanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(HANDLE_TO_PATH_BASE + 21)) {
		int fd;
		void *dirhanp, *targhanp;
		size_t dirhlen, targhlen;
		char pathbuf[PATHBUF_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_handle(DUMMY_SUBDIR, &dirhanp,
					   &dirhlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		} else
		    if ((fd =
			 open(DUMMY_SUBDIR_FILE, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = dm_fd_to_handle(fd, &targhanp, &targhlen)) ==
			   -1) {
			close(fd);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(targhanp, targhlen);
			remove(DUMMY_SUBDIR_FILE);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		} else if ((rc = remove(DUMMY_SUBDIR_FILE)) == -1) {
			dm_handle_free(targhanp, targhlen);
			dm_handle_free(dirhanp, dirhlen);
			rmdir(DUMMY_SUBDIR);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated targhanp)\n",
				    szFuncName);
			rc = dm_handle_to_path(dirhanp, dirhlen, targhanp,
					       targhlen, sizeof(pathbuf),
					       pathbuf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dirhanp, dirhlen);
			dm_handle_free(targhanp, targhlen);
		}
	}

	szFuncName = "dm_sync_by_handle";

	/*
	 * TEST    : dm_sync_by_handle - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_sync_by_handle(INVALID_ADDR, hanp, hlen,
					       DM_NO_TOKEN);
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
	 * TEST    : dm_sync_by_handle - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_sync_by_handle(DM_NO_SESSION, hanp, hlen,
					       DM_NO_TOKEN);
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
	 * TEST    : dm_sync_by_handle - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_sync_by_handle(sid, (void *)INVALID_ADDR, hlen,
					       DM_NO_TOKEN);
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
	 * TEST    : dm_sync_by_handle - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_sync_by_handle(sid, hanp, INVALID_ADDR,
					       DM_NO_TOKEN);
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
	 * TEST    : dm_sync_by_handle - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			rc = dm_sync_by_handle(sid, hanp, hlen, INVALID_ADDR);
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
	 * TEST    : dm_sync_by_handle - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((rc =
			 (write(fd, DUMMY_STRING, DUMMY_STRLEN) ==
			  DUMMY_STRLEN) ? 0 : -1) == -1) {
			close(fd);
			remove(DUMMY_FILE);
			dm_handle_free(hanp, hlen);
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
			rc = dm_sync_by_handle(sid, hanp, hlen, DM_NO_TOKEN);
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
	 * TEST    : dm_sync_by_handle - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 7)) {
		void *hanp;
		size_t hlen;

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
			rc = dm_sync_by_handle(sid, hanp, hlen, DM_NO_TOKEN);
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

	/*
	 * TEST    : dm_sync_by_handle - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_sync_by_handle(sid, hanp, hlen, DM_NO_TOKEN);
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
	 * TEST    : dm_sync_by_handle - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 9)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_sync_by_handle(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				       DM_NO_TOKEN);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_sync_by_handle - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SYNC_BY_HANDLE_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			remove(DUMMY_FILE);
			dm_handle_free(hanp, hlen);
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
			rc = dm_sync_by_handle(sid, hanp, hlen, DM_NO_TOKEN);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	dm_handle_free(mtpthanp, mtpthlen);
	dm_handle_free(curdirhanp, curdirhlen);

	DMLOG_STOP();

	tst_exit();

}
