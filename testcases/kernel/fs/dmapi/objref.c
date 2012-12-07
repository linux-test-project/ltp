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
 * TEST CASE	: objref.c
 *
 * VARIATIONS	: 24
 *
 * API'S TESTED	: dm_obj_ref_hold
 * 		  dm_obj_ref_rele
 * 		  dm_obj_ref_query
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
char DummySubdir[FILENAME_MAX];
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
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);

		rmdir(DummySubdir);
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

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI object reference tests\n");

	szFuncName = "dm_obj_ref_hold";

	/*
	 * TEST    : dm_obj_ref_hold - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 1)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_obj_ref_hold(INVALID_ADDR, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_hold - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 2)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_obj_ref_hold(sid, INVALID_ADDR, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_hold - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 3)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_obj_ref_hold(sid, token, (void *)INVALID_ADDR, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_hold - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 4)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n", szFuncName);
		rc = dm_obj_ref_hold(sid, token, hanp, INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_hold - multiple holds
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 5)) {

		/* Variation set up */
		rc = dm_obj_ref_hold(sid, token, hanp, hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(multiple holds)\n",
				    szFuncName);
			rc = dm_obj_ref_hold(sid, token, hanp, hlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			rc = dm_obj_ref_rele(sid, token, hanp, hlen);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_obj_ref_hold - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 6)) {
		int rc2;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
		rc = dm_obj_ref_hold(sid, token, hanp, hlen);
		if (rc == 0) {
			if ((rc2 =
			     dm_obj_ref_query(sid, token, hanp, hlen)) == 1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, 0);
				DMVAR_PASS();
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with expected rc = %d but unexpected dm_obj_ref_query rc = %d\n",
					    szFuncName, 0, rc2);
				DMVAR_FAIL();
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with unexpected rc = %d (errno = %d)\n",
				    szFuncName, rc, errno);
			DMVAR_FAIL();
		}

		/* Variation clean up */
		rc = dm_obj_ref_rele(sid, token, hanp, hlen);
	}

	/*
	 * TEST    : dm_obj_ref_hold - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 7)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_obj_ref_hold(DM_NO_SESSION, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_hold - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(OBJ_REF_HOLD_BASE + 8)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_obj_ref_hold(sid, token, DM_GLOBAL_HANP,
				     DM_GLOBAL_HLEN);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	szFuncName = "dm_obj_ref_rele";

	/*
	 * TEST    : dm_obj_ref_rele - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 1)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_obj_ref_rele(INVALID_ADDR, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_rele - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 2)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_obj_ref_rele(sid, INVALID_ADDR, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_rele - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 3)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_obj_ref_rele(sid, token, (void *)INVALID_ADDR, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_rele - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 4)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n", szFuncName);
		rc = dm_obj_ref_rele(sid, token, hanp, INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_rele - no hold
	 * EXPECTED: rc = -1, errno = EACCES
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(no hold)\n", szFuncName);
		rc = dm_obj_ref_rele(sid, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EACCES);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_rele - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 6)) {
		int rc2;

		/* Variation set up */
		rc = dm_obj_ref_hold(sid, token, hanp, hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
			rc = dm_obj_ref_rele(sid, token, hanp, hlen);
			if (rc == 0) {
				if ((rc2 =
				     dm_obj_ref_query(sid, token, hanp,
						      hlen)) == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected dm_obj_ref_query rc = %d\n",
						    szFuncName, 0, rc2);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}
		}
	}

	/*
	 * TEST    : dm_obj_ref_rele - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 7)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_obj_ref_rele(DM_NO_SESSION, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_rele - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(OBJ_REF_RELE_BASE + 8)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_obj_ref_rele(sid, token, DM_GLOBAL_HANP,
				     DM_GLOBAL_HLEN);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	szFuncName = "dm_obj_ref_query";

	/*
	 * TEST    : dm_obj_ref_query - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 1)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_obj_ref_query(INVALID_ADDR, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_query - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 2)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_obj_ref_query(sid, INVALID_ADDR, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_query - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 3)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n", szFuncName);
		rc = dm_obj_ref_query(sid, token, (void *)INVALID_ADDR, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_query - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 4)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n", szFuncName);
		rc = dm_obj_ref_query(sid, token, hanp, INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_query - not held
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 5)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(not held)\n", szFuncName);
		rc = dm_obj_ref_query(sid, token, hanp, hlen);
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_query - held
	 * EXPECTED: rc = 1
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 6)) {

		/* Variation set up */
		rc = dm_obj_ref_hold(sid, token, hanp, hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(held)\n", szFuncName);
			rc = dm_obj_ref_query(sid, token, hanp, hlen);
			DMVAR_ENDPASSEXP(szFuncName, 1, rc);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_obj_ref_query - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 7)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_obj_ref_query(DM_NO_SESSION, token, hanp, hlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_obj_ref_query - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(OBJ_REF_QUERY_BASE + 8)) {

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_obj_ref_query(sid, token, DM_GLOBAL_HANP,
				      DM_GLOBAL_HLEN);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
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
