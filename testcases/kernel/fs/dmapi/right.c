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
 * TEST CASE	: right.c
 *
 * VARIATIONS	: 63
 *
 * API'S TESTED	: dm_request_right
 * 		  dm_release_right
 * 		  dm_query_right
 * 		  dm_upgrade_right
 * 		  dm_downgrade_right
 */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include "dm_test.h"

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char command[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummySubdir[FILENAME_MAX];
int fd_f;
int runTestOnCreate;

void *Thread(void *);

int main(int argc, char **argv)
{

	char *varstr;
	int rc;
	char *szSessionInfo = "dm_test session info";
	dm_eventset_t events;

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_MOUNT, events);

	/* CANNOT DO ANYTHING WITHOUT SUCCESSFUL INITIALIZATION!!! */
	if ((rc = dm_init_service(&varstr)) != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_init_service failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else if ((rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &sid))
		   == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_create_session failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else
	    if ((rc =
		 dm_set_disp(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN, DM_NO_TOKEN,
			     &events, DM_EVENT_MAX)) == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_disp failed! (rc = %d, errno = %d)\n", rc,
			    errno);
		dm_destroy_session(sid);
		DM_EXIT();
	} else if ((rc = pthread_create(&tid, NULL, Thread, NULL)) != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "pthread_create failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	} else if ((rc = dmimpl_mount(&mountPt, &deviceNm)) == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dmimpl_mount failed! (rc = %d, errno = %d)\n", rc,
			    errno);
		dm_destroy_session(sid);
		DM_EXIT();
	} else {
		sprintf(DummyFile, "%s/%s", mountPt, DUMMY_FILE);
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);

		remove(DummyFile);
		rmdir(DummySubdir);
	}

	fd_f = open(DummyFile, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
	if (fd_f == -1) {
		DMLOG_PRINT(DMLVL_ERR, "open failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	/* This is what kicks off the test case, variations done in thread */
	runTestOnCreate = 1;
	rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
	runTestOnCreate = 0;
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR, "mkdir failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	rc = rmdir(DummySubdir);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR, "rmdir failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	rc = close(fd_f);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR, "close failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	rc = remove(DummyFile);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR, "remove failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	rc = umount(mountPt);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR, "umount failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	pthread_join(tid, NULL);

	rc = dm_destroy_session(sid);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_destroy_session failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	DMLOG_STOP();

	tst_exit();
}

void DoTest(dm_token_t token, void *hanp, size_t hlen)
{

	char *szFuncName;
	int rc;

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI rights tests\n");

	szFuncName = "dm_request_right";

	/*
	 * TEST    : dm_request_right - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 1)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_request_right(INVALID_ADDR, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 2)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_request_right(sid, (void *)INVALID_ADDR, hlen, token, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 3)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n", szFuncName);
		rc = dm_request_right(sid, hanp, INVALID_ADDR, token, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 4)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_request_right(sid, hanp, hlen, INVALID_ADDR, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - invalid right
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #29 (0 returned instead of -1 and
	 * errno EINVAL)
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid right)\n", szFuncName);
		rc = dm_request_right(sid, hanp, hlen, token, 0, INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - DM_NO_TOKEN
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 6)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_TOKEN)\n", szFuncName);
		rc = dm_request_right(sid, hanp, hlen, DM_NO_TOKEN, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_SHARED from DM_RIGHT_NULL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 7)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG,
			    "%s(DM_RIGHT_NULL -> DM_RIGHT_SHARED)\n",
			    szFuncName);
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		rc = dm_release_right(sid, hanp, hlen, token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_EXCL from DM_RIGHT_NULL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 8)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_NULL -> DM_RIGHT_EXCL)\n",
			    szFuncName);
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		rc = dm_release_right(sid, hanp, hlen, token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_SHARED from DM_RIGHT_SHARED
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 9)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_RIGHT_SHARED -> DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_request_right(sid, hanp, hlen, token, 0,
					      DM_RIGHT_SHARED);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_EXCL from DM_RIGHT_SHARED,
	 *              DM_RR_WAIT clear
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 10)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_RIGHT_SHARED -> DM_RIGHT_EXCL, DM_RR_WAIT clear)\n",
				    szFuncName);
			rc = dm_request_right(sid, hanp, hlen, token, 0,
					      DM_RIGHT_EXCL);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_EXCL from DM_RIGHT_SHARED,
	 *              DM_RR_WAIT set
	 * EXPECTED: rc = -1, errno = EACCES
	 *
	 * This variation uncovered XFS BUG #30 (0 returned instead of -1 and
	 * errno EACCES)
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 11)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_RIGHT_SHARED -> DM_RIGHT_EXCL, DM_RR_WAIT set)\n",
				    szFuncName);
			rc = dm_request_right(sid, hanp, hlen, token,
					      DM_RR_WAIT, DM_RIGHT_EXCL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EACCES);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_EXCL from DM_RIGHT_EXCL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 12)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_RIGHT_EXCL -> DM_RIGHT_EXCL)\n",
				    szFuncName);
			rc = dm_request_right(sid, hanp, hlen, token, 0,
					      DM_RIGHT_SHARED);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_request_right - DM_RIGHT_SHARED from DM_RIGHT_EXCL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 13)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_RIGHT_EXCL -> DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_request_right(sid, hanp, hlen, token, 0,
					      DM_RIGHT_SHARED);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_request_right - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 14)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_request_right(DM_NO_SESSION, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 15)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_request_right(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      token, 0, DM_RIGHT_SHARED);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_request_right - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 16)) {
		void *fhanp;
		size_t fhlen;

		/* Variation set up */
		rc = dm_fd_to_handle(fd_f, &fhanp, &fhlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_request_right(sid, fhanp, fhlen, token, 0,
					      DM_RIGHT_SHARED);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, fhanp, fhlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_request_right - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(REQUEST_RIGHT_BASE + 17)) {
		void *fshanp;
		size_t fshlen;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &fshanp, &fshlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_request_right(sid, fshanp, fshlen, token, 0,
					      DM_RIGHT_SHARED);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, fshanp, fshlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
		}
	}

	szFuncName = "dm_release_right";

	/*
	 * TEST    : dm_release_right - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 1)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_release_right(INVALID_ADDR, hanp, hlen, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 2)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_release_right(sid, (void *)INVALID_ADDR, hlen, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 3)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n", szFuncName);
		rc = dm_release_right(sid, hanp, INVALID_ADDR, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 4)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_release_right(sid, hanp, hlen, INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - DM_NO_TOKEN
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_TOKEN)\n", szFuncName);
		rc = dm_release_right(sid, hanp, hlen, DM_NO_TOKEN);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - DM_RIGHT_NULL
	 * EXPECTED: rc = -1, errno = EACCES
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 6)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_NULL)\n", szFuncName);
		rc = dm_release_right(sid, hanp, hlen, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EACCES);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - DM_RIGHT_SHARED
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 7)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_release_right(sid, hanp, hlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_release_right - DM_RIGHT_EXCL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 8)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_EXCL)\n",
				    szFuncName);
			rc = dm_release_right(sid, hanp, hlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_release_right - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 9)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_release_right(DM_NO_SESSION, hanp, hlen, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 10)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_release_right(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_release_right - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 11)) {
		void *fhanp;
		size_t fhlen;

		/* Variation set up */
		if ((rc == dm_fd_to_handle(fd_f, &fhanp, &fhlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fhanp, fhlen, token, 0,
					  DM_RIGHT_SHARED)) == -1) {
			dm_handle_free(fhanp, fhlen);
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
			rc = dm_release_right(sid, fhanp, fhlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_release_right - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(RELEASE_RIGHT_BASE + 12)) {
		void *fshanp;
		size_t fshlen;

		/* Variation set up */
		if ((rc == dm_path_to_fshandle(DummyFile, &fshanp, &fshlen)) ==
		    -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fshanp, fshlen, token, 0,
					  DM_RIGHT_SHARED)) == -1) {
			dm_handle_free(fshanp, fshlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_release_right(sid, fshanp, fshlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			dm_handle_free(fshanp, fshlen);
		}
	}

	szFuncName = "dm_query_right";

	/*
	 * TEST    : dm_query_right - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 1)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_query_right(INVALID_ADDR, hanp, hlen, token, &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 2)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_query_right(sid, (void *)INVALID_ADDR, hlen, token,
				    &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 3)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n", szFuncName);
		rc = dm_query_right(sid, hanp, INVALID_ADDR, token, &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 4)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_query_right(sid, hanp, hlen, INVALID_ADDR, &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - invalid rightp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rightp)\n", szFuncName);
		rc = dm_query_right(sid, hanp, hlen, token,
				    (dm_right_t *) INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - DM_NO_TOKEN
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 6)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_TOKEN)\n", szFuncName);
		rc = dm_query_right(sid, hanp, hlen, DM_NO_TOKEN, &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - DM_RIGHT_SHARED
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 7)) {
		dm_right_t right;

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_query_right(sid, hanp, hlen, token, &right);
			if (rc == 0) {
				if (right == DM_RIGHT_SHARED) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected right (%d vs %d)\n",
						    szFuncName, 0, right,
						    DM_RIGHT_SHARED);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_right - DM_RIGHT_EXCL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 8)) {
		dm_right_t right;

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_query_right(sid, hanp, hlen, token, &right);
			if (rc == 0) {
				if (right == DM_RIGHT_EXCL) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected right (%d vs %d)\n",
						    szFuncName, 0, right,
						    DM_RIGHT_EXCL);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_right - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 9)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_query_right(DM_NO_SESSION, hanp, hlen, token, &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 10)) {
		dm_right_t right;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_query_right(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN, token,
				    &right);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_right - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 11)) {
		void *fhanp;
		size_t fhlen;
		dm_right_t right;

		/* Variation set up */
		if ((rc = dm_fd_to_handle(fd_f, &fhanp, &fhlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fhanp, fhlen, token, 0,
					  DM_RIGHT_SHARED)) == -1) {
			dm_handle_free(fhanp, fhlen);
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
			rc = dm_query_right(sid, fhanp, fhlen, token, &right);
			if (rc == 0) {
				if (right == DM_RIGHT_SHARED) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected right (%d vs %d)\n",
						    szFuncName, 0, right,
						    DM_RIGHT_SHARED);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_release_right(sid, fhanp, fhlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_query_right - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(QUERY_RIGHT_BASE + 12)) {
		void *fshanp;
		size_t fshlen;
		dm_right_t right;

		/* Variation set up */
		if ((rc =
		     dm_path_to_fshandle(DummyFile, &fshanp, &fshlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fshanp, fshlen, token, 0,
					  DM_RIGHT_SHARED)) == -1) {
			dm_handle_free(fshanp, fshlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_query_right(sid, fshanp, fshlen, token, &right);
			if (rc == 0) {
				if (right == DM_RIGHT_SHARED) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected right (%d vs %d)\n",
						    szFuncName, 0, right,
						    DM_RIGHT_SHARED);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_release_right(sid, fshanp, fshlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
		}
	}

	szFuncName = "dm_upgrade_right";

	/*
	 * TEST    : dm_upgrade_right - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 1)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_upgrade_right(INVALID_ADDR, hanp, hlen, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 2)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_upgrade_right(sid, (void *)INVALID_ADDR, hlen,
					      token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 3)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_upgrade_right(sid, hanp, INVALID_ADDR, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 4)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_upgrade_right(sid, hanp, hlen, INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - DM_RIGHT_NULL
	 * EXPECTED: rc = -1, errno = EPERM
	 *
	 * This variation uncovered XFS BUG #31 (EACCES returned instead of
	 * EPERM)
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_NULL)\n", szFuncName);
		rc = dm_upgrade_right(sid, hanp, hlen, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EPERM);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_upgrade_right - DM_RIGHT_SHARED
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 6)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_upgrade_right(sid, hanp, hlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - DM_RIGHT_EXCL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 7)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_EXCL)\n",
				    szFuncName);
			rc = dm_upgrade_right(sid, hanp, hlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 8)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_upgrade_right(DM_NO_SESSION, hanp, hlen, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 9)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n",
				    szFuncName);
			rc = dm_upgrade_right(sid, DM_GLOBAL_HANP,
					      DM_GLOBAL_HLEN, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_upgrade_right - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 10)) {
		void *fhanp;
		size_t fhlen;

		/* Variation set up */
		if ((rc = dm_fd_to_handle(fd_f, &fhanp, &fhlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fhanp, fhlen, token, 0,
					  DM_RIGHT_SHARED)) == -1) {
			dm_handle_free(fhanp, fhlen);
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
			rc = dm_upgrade_right(sid, fhanp, fhlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, fhanp, fhlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_upgrade_right - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(UPGRADE_RIGHT_BASE + 11)) {
		void *fshanp;
		size_t fshlen;

		/* Variation set up */
		if ((rc =
		     dm_path_to_fshandle(DummyFile, &fshanp, &fshlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fshanp, fshlen, token, 0,
					  DM_RIGHT_SHARED)) == -1) {
			dm_handle_free(fshanp, fshlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_upgrade_right(sid, fshanp, fshlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, fshanp, fshlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
		}
	}

	szFuncName = "dm_downgrade_right";

	/*
	 * TEST    : dm_downgrade_right - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 1)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_downgrade_right(INVALID_ADDR, hanp, hlen,
						token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 2)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_downgrade_right(sid, (void *)INVALID_ADDR, hlen,
						token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 3)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_downgrade_right(sid, hanp, INVALID_ADDR, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 4)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_downgrade_right(sid, hanp, hlen, INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - DM_RIGHT_NULL
	 * EXPECTED: rc = -1, errno = EPERM
	 *
	 * This variation uncovered XFS BUG #32 (EACCES returned instead of
	 * EPERM)
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_NULL)\n", szFuncName);
		rc = dm_downgrade_right(sid, hanp, hlen, token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EPERM);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_downgrade_right - DM_RIGHT_SHARED
	 * EXPECTED: rc = -1, errno = EPERM
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 6)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_SHARED)\n",
				    szFuncName);
			rc = dm_downgrade_right(sid, hanp, hlen, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EPERM);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - DM_RIGHT_EXCL
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 7)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0, DM_RIGHT_EXCL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RIGHT_EXCL)\n",
				    szFuncName);
			rc = dm_downgrade_right(sid, hanp, hlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 8)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_downgrade_right(DM_NO_SESSION, hanp, hlen,
						token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 9)) {

		/* Variation set up */
		rc = dm_request_right(sid, hanp, hlen, token, 0,
				      DM_RIGHT_SHARED);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_downgrade_right(sid, DM_GLOBAL_HANP,
						DM_GLOBAL_HLEN, token);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_release_right(sid, hanp, hlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_downgrade_right - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 10)) {
		void *fhanp;
		size_t fhlen;

		/* Variation set up */
		if ((rc = dm_fd_to_handle(fd_f, &fhanp, &fhlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fhanp, fhlen, token, 0,
					  DM_RIGHT_EXCL)) == -1) {
			dm_handle_free(fhanp, fhlen);
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
			rc = dm_downgrade_right(sid, fhanp, fhlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, fhanp, fhlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fhanp, fhlen);
		}
	}

	/*
	 * TEST    : dm_downgrade_right - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(DOWNGRADE_RIGHT_BASE + 11)) {
		void *fshanp;
		size_t fshlen;

		/* Variation set up */
		if ((rc =
		     dm_path_to_fshandle(DummyFile, &fshanp, &fshlen)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_request_right(sid, fshanp, fshlen, token, 0,
					  DM_RIGHT_EXCL)) == -1) {
			dm_handle_free(fshanp, fshlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_downgrade_right(sid, fshanp, fshlen, token);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_release_right(sid, fshanp, fshlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
		}
	}

}

void *Thread(void *parm)
{
	int rc;
	size_t dmMsgBufLen;
	dm_eventmsg_t *dmMsg;
	int bMounted = DM_FALSE;
	dm_eventtype_t type;
	dm_token_t token;
	dm_eventset_t events;
	dm_response_t response;

	do {
		/* Loop until message received (wait could be interrupted) */
		do {
			DMLOG_PRINT(DMLVL_DEBUG, "Waiting for event...\n");
			dmMsgBufLen = 0;

			rc = dm_get_events(sid, 1, DM_EV_WAIT, sizeof(dmMsgBuf),
					   dmMsgBuf, &dmMsgBufLen);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "... dm_get_events returned %d (errno %d)\n",
				    rc, errno);
		} while ((rc == -1) && (errno == EINTR) && (dmMsgBufLen == 0));

		if (rc) {
			DMLOG_PRINT(DMLVL_ERR,
				    "dm_get_events failed with rc = %d, errno = %d\n",
				    rc, errno);
			dm_destroy_session(sid);
			DM_EXIT();
		} else {
			dmMsg = (dm_eventmsg_t *) dmMsgBuf;
			token = dmMsg->ev_token;
			type = dmMsg->ev_type;

			DMLOG_PRINT(DMLVL_DEBUG, "Received message %d\n", type);
		}

		if (type == DM_EVENT_MOUNT) {
			/* SPECIAL CASE: need to set disposition, events and response */
			dm_mount_event_t *me =
			    DM_GET_VALUE(dmMsg, ev_data, dm_mount_event_t *);
			void *lhanp = DM_GET_VALUE(me, me_handle1, void *);
			size_t lhlen = DM_GET_LEN(me, me_handle1);

			DMLOG_PRINT(DMLVL_DEBUG, "Message is DM_EVENT_MOUNT\n");
			DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n", me->me_mode);
			DMLOG_PRINT(DMLVL_DEBUG, "  File system handle: %p\n",
				    lhanp);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  File system handle length: %d\n", lhlen);
			DMLOG_PRINT(DMLVL_DEBUG, "  Mountpoint handle: %p\n",
				    DM_GET_VALUE(me, me_handle2, void *));
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  Mountpoint handle length: %d\n",
				    DM_GET_LEN(me, me_handle2));
			DMLOG_PRINT(DMLVL_DEBUG, "  Mountpoint path: %s\n",
				    DM_GET_VALUE(me, me_name1, char *));
			DMLOG_PRINT(DMLVL_DEBUG, "  Media designator: %s\n",
				    DM_GET_VALUE(me, me_name2, char *));
			DMLOG_PRINT(DMLVL_DEBUG, "  Root handle: %p\n",
				    DM_GET_VALUE(me, me_roothandle, void *));
			DMLOG_PRINT(DMLVL_DEBUG, "  Root handle length: %d\n",
				    DM_GET_LEN(me, me_roothandle));

			bMounted = dm_handle_is_valid(lhanp, lhlen);

			rc = dm_request_right(sid, lhanp, lhlen, token,
					      DM_RR_WAIT, DM_RIGHT_EXCL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_request_right failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			DMEV_ZERO(events);
			DMEV_SET(DM_EVENT_PREUNMOUNT, events);
			DMEV_SET(DM_EVENT_UNMOUNT, events);
			DMEV_SET(DM_EVENT_CREATE, events);
			rc = dm_set_disp(sid, lhanp, lhlen, token, &events,
					 DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			rc = dm_set_eventlist(sid, lhanp, lhlen, token, &events,
					      DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_eventlist failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			rc = dm_release_right(sid, lhanp, lhlen, token);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_request_right failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			response = DM_RESP_CONTINUE;
		} else if (type == DM_EVENT_UNMOUNT) {
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);
			if (nse->ne_retcode == 0) {
				bMounted = DM_FALSE;
			}

			response = DM_RESP_CONTINUE;
		} else if (type == DM_EVENT_CREATE) {
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);
			void *hanp = DM_GET_VALUE(nse, ne_handle1, void *);
			size_t hlen = DM_GET_LEN(nse, ne_handle1);

			if (runTestOnCreate) {
				DoTest(token, hanp, hlen);
			}

			response = DM_RESP_CONTINUE;
		} else {
			switch (type) {
			case DM_EVENT_PREUNMOUNT:
				response = DM_RESP_CONTINUE;
				break;

			default:
				{
					DMLOG_PRINT(DMLVL_ERR,
						    "Message is unexpected!\n");
					response = DM_RESP_ABORT;
					break;
				}
			}
		}

		if (response != DM_RESP_INVALID) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Responding to message %d with %d\n", type,
				    response);
			rc = dm_respond_event(sid, token, response,
					      response ==
					      DM_RESP_ABORT ? ABORT_ERRNO : 0,
					      0, NULL);
		}
	} while (bMounted);

	pthread_exit(0);
}
