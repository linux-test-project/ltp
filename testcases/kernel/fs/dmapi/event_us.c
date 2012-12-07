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
 * TEST CASE	: event_us.c
 *
 * VARIATIONS	: 21
 *
 * EVENTS TESTED: DM_EVENT_USER
 *
 * API'S TESTED	: dm_create_userevent
 * 		  dm_send_msg
 * 		  dm_find_eventmsg
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mount.h>
#include "dm_test.h"

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char *mountPt;
char *deviceNm;
void *fshanp;
size_t fshlen;
dm_size_t maxMsgDat;

/* Variables for thread communications */
dm_eventtype_t eventExpected;
dm_eventtype_t eventReceived;
dm_response_t eventResponse;
void *hanp2;
size_t hlen2;
char name1[FILENAME_MAX];
dm_token_t tokenReceived;
char msgDataReceived[MSG_DATALEN];
dm_size_t msgDataLenReceived;

void *Thread(void *);

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int rc;
	int varStatus;
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
		rc = dm_get_config(fshanp, fshlen, DM_CONFIG_MAX_MESSAGE_DATA,
				   &maxMsgDat);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "dm_get_config failed! (rc = %d, errno = %d)\n",
				    rc, errno);
			umount(mountPt);
			dm_destroy_session(sid);
			DM_EXIT();
		}
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI user event tests\n");

	szFuncName = "dm_create_userevent";

	/*
	 * TEST    : dm_create_uservent - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CREATE_USEREVENT_BASE + 1)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_create_userevent(INVALID_ADDR, MSG_DATALEN, buf,
					 &token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_uservent - invalid msglen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(CREATE_USEREVENT_BASE + 2)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid msglen)\n", szFuncName);
		rc = dm_create_userevent(sid, maxMsgDat + 1, buf, &token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_uservent - invalid msgdatap
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(CREATE_USEREVENT_BASE + 3)) {
		dm_token_t token;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid msgdatap)\n", szFuncName);
		rc = dm_create_userevent(sid, MSG_DATALEN, (void *)INVALID_ADDR,
					 &token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_uservent - invalid tokenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #11 (unused tevp left on queue)
	 */
	if (DMVAR_EXEC(CREATE_USEREVENT_BASE + 4)) {
		char buf[MSG_DATALEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid tokenp)\n", szFuncName);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf,
					 (dm_token_t *) INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_uservent - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(CREATE_USEREVENT_BASE + 5)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0, 0, NULL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : dm_create_uservent - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CREATE_USEREVENT_BASE + 6)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_create_userevent(DM_NO_SESSION, MSG_DATALEN, buf,
					 &token);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	szFuncName = "dm_send_msg";

	/*
	 * TEST    : dm_send_msg - invalid targetsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 1)) {
		char buf[MSG_DATALEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid targetsid)\n", szFuncName);
		rc = dm_send_msg(INVALID_ADDR, DM_MSGTYPE_SYNC, MSG_DATALEN,
				 buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - invalid msgtype
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 2)) {
		char buf[MSG_DATALEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid msgtype)\n", szFuncName);
		rc = dm_send_msg(sid, INVALID_ADDR, MSG_DATALEN, buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 3)) {
		char buf[MSG_DATALEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid buflen)\n", szFuncName);
		rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, maxMsgDat + 1, buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 4)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n", szFuncName);
		rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN,
				 (void *)INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - DM_RESP_CONTINUE
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 5)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		eventExpected = DM_EVENT_USER;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(continue response)\n", szFuncName);
		rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
		if ((varStatus =
		     DMVAR_CHKPASSEXP(0, rc, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			if (tokenReceived == 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Token NOT correct! (%d vs non-zero)\n",
					    tokenReceived);
				varStatus = DMSTAT_FAIL;
			}
			if (msgDataLenReceived != MSG_DATALEN) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Message lengths NOT same! (%d vs %d)\n",
					    msgDataLenReceived, MSG_DATALEN);
				varStatus = DMSTAT_FAIL;
			} else if (memcmp(msgDataReceived, buf, MSG_DATALEN) !=
				   0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Message data NOT same! (%s vs %s)\n",
					    msgDataReceived, buf);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - DM_RESP_ABORT
	 * EXPECTED: rc = -1, errno = ABORT_ERRNO
	 *
	 * This variation uncovered XFS BUG #39 (response reterror returned
	 * instead of -1 and errno set to reterror)
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 6)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		eventExpected = DM_EVENT_USER;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_ABORT;
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(abort response)\n", szFuncName);
		rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
		if ((varStatus =
		     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			if (tokenReceived == 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Token NOT correct! (%d vs non-zero)\n",
					    tokenReceived);
				varStatus = DMSTAT_FAIL;
			}
			if (msgDataLenReceived != MSG_DATALEN) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Message lengths NOT same! (%d vs %d)\n",
					    msgDataLenReceived, MSG_DATALEN);
				varStatus = DMSTAT_FAIL;
			} else if (memcmp(msgDataReceived, buf, MSG_DATALEN) !=
				   0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Message data NOT same! (%s vs %s)\n",
					    msgDataReceived, buf);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - DM_MSGTYPE_ASYNC
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 7)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		eventExpected = DM_EVENT_USER;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_MSGTYPE_ASYNC)\n", szFuncName);
		rc = dm_send_msg(sid, DM_MSGTYPE_ASYNC, MSG_DATALEN, buf);
		EVENT_DELIVERY_DELAY;
		if ((varStatus =
		     DMVAR_CHKPASSEXP(0, rc, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			if (tokenReceived != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Token NOT correct! (%d vs %d)\n",
					    tokenReceived, 0);
				varStatus = DMSTAT_FAIL;
			}
			if (msgDataLenReceived != MSG_DATALEN) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Message lengths NOT same! (%d vs %d)\n",
					    msgDataLenReceived, MSG_DATALEN);
				varStatus = DMSTAT_FAIL;
			} else if (memcmp(msgDataReceived, buf, MSG_DATALEN) !=
				   0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Message data NOT same! (%s vs %s)\n",
					    msgDataReceived, buf);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_send_msg - DM_NO_SESSION targetsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SEND_MSG_BASE + 8)) {
		char buf[MSG_DATALEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION targetsid)\n",
			    szFuncName);
		rc = dm_send_msg(DM_NO_SESSION, DM_MSGTYPE_SYNC, MSG_DATALEN,
				 buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	szFuncName = "dm_find_eventmsg";

	/*
	 * TEST    : dm_find_eventmsg - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 1)) {
		dm_token_t token;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_find_eventmsg(INVALID_ADDR, token, MSG_DATALEN,
					      buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_find_eventmsg - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 2)) {
		dm_token_t token;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_find_eventmsg(sid, INVALID_ADDR, MSG_DATALEN,
					      buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_find_eventmsg - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 3)) {
		dm_token_t token;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid buflen)\n",
				    szFuncName);
			rc = dm_find_eventmsg(sid, token, MSG_DATALEN - 1, buf,
					      &rlen);
			if (rc == -1) {
				if (errno == E2BIG) {
					DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n",
						    rlen);
					if (rlen ==
					    MSG_DATALEN +
					    sizeof(dm_eventmsg_t)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d, expected errno = %d, and expected rlen = %d\n",
							    szFuncName, rc,
							    errno, rlen);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and expected errno = %d but unexpected rlen (%d vs %d)\n",
							    szFuncName, rc,
							    errno, rlen,
							    MSG_DATALEN);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected errno = %d\n",
						    szFuncName, rc, errno);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d\n",
					    szFuncName, rc);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_find_eventmsg - invalid bufp
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 4)) {
		dm_token_t token;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n",
				    szFuncName);
			rc = dm_find_eventmsg(sid, token, MSG_DATALEN,
					      (void *)INVALID_ADDR, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_find_eventmsg - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 5)) {
		dm_token_t token;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rlenp)\n",
				    szFuncName);
			rc = dm_find_eventmsg(sid, token, MSG_DATALEN, buf,
					      (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_find_eventmsg - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 6)) {
		dm_token_t token;
		char bufin[MSG_DATALEN],
		    bufout[MSG_DATALEN + sizeof(dm_eventmsg_t)];
		size_t rlen;

		/* Variation set up */
		memcpy(bufin, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, bufin, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
			rc = dm_find_eventmsg(sid, token, sizeof(bufout),
					      bufout, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				if (rlen == MSG_DATALEN + sizeof(dm_eventmsg_t)) {
					if (memcmp
					    (bufin,
					     bufout + sizeof(dm_eventmsg_t),
					     MSG_DATALEN) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d, expected rlen %d, and expected buffer %s\n",
							    szFuncName, rc,
							    rlen,
							    bufout +
							    sizeof
							    (dm_eventmsg_t));
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and expected rlen %d but expected buffer %s\n",
							    szFuncName, rc,
							    rlen, bufout);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s passed with expected rc = %d but unexpected rlen %d\n",
						    szFuncName, rc, rlen);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_find_eventmsg - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(FIND_EVENTMSG_BASE + 7)) {
		dm_token_t token;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to initialize variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_find_eventmsg(DM_NO_SESSION, token, MSG_DATALEN,
					      buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	rc = umount(mountPt);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR, "umount failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	EVENT_DELIVERY_DELAY;
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

void *Thread(void *parm)
{
	int rc;
	size_t dmMsgBufLen;
	dm_eventmsg_t *dmMsg;
	int bMounted = DM_FALSE;
	int type;
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
			fshanp = DM_GET_VALUE(me, me_handle1, void *);
			fshlen = DM_GET_LEN(me, me_handle1);

			DMLOG_PRINT(DMLVL_DEBUG, "Message is DM_EVENT_MOUNT\n");
			DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n", me->me_mode);
			DMLOG_PRINT(DMLVL_DEBUG, "  File system handle: %p\n",
				    fshanp);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  File system handle length: %d\n",
				    fshlen);
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

			bMounted = dm_handle_is_valid(fshanp, fshlen);

			/*rc = dm_request_right(sid, fshanp, fshlen, token, DM_RR_WAIT, DM_RIGHT_EXCL);
			   if (rc == -1) {
			   DMLOG_PRINT(DMLVL_ERR, "dm_request_right failed! (rc = %d, errno = %d)\n", rc, errno);
			   dm_destroy_session(sid);
			   DM_EXIT();
			   } */

			DMEV_ZERO(events);
			DMEV_SET(DM_EVENT_PREUNMOUNT, events);
			DMEV_SET(DM_EVENT_UNMOUNT, events);
			DMEV_SET(DM_EVENT_POSTCREATE, events);
			DMEV_SET(DM_EVENT_ATTRIBUTE, events);
			DMEV_SET(DM_EVENT_CLOSE, events);
			DMEV_SET(DM_EVENT_DESTROY, events);
			rc = dm_set_disp(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &events, DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			rc = dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					      &events, DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_eventlist failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			/*rc = dm_release_right(sid, fshanp, fshlen, token);
			   if (rc == -1) {
			   DMLOG_PRINT(DMLVL_ERR, "dm_request_right failed! (rc = %d, errno = %d)\n", rc, errno);
			   dm_destroy_session(sid);
			   DM_EXIT();
			   } */

			response = DM_RESP_CONTINUE;
		} else if (type == DM_EVENT_PREUNMOUNT) {
			/* SPECIAL CASE: need to set response */
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);

			DMLOG_PRINT(DMLVL_DEBUG,
				    "Message is DM_EVENT_PREUNMOUNT\n");
			DMLOG_PRINT(DMLVL_DEBUG, "  Unmount mode: %x\n",
				    nse->ne_mode);
			DMLOG_PRINT(DMLVL_DEBUG, "  File system handle: %p\n",
				    DM_GET_VALUE(nse, ne_handle1, void *));
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  File system handle length: %d\n",
				    DM_GET_LEN(nse, ne_handle1));
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  Root directory handle: %p\n",
				    DM_GET_VALUE(nse, ne_handle2, void *));
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  Root directory handle length: %d\n",
				    DM_GET_LEN(nse, ne_handle2));

			response = DM_RESP_CONTINUE;
		} else if (type == DM_EVENT_UNMOUNT) {
			/* SPECIAL CASE: need to set response and bMounted */
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);

			DMLOG_PRINT(DMLVL_DEBUG,
				    "Message is DM_EVENT_UNMOUNT\n");
			DMLOG_PRINT(DMLVL_DEBUG, "  Unmount mode: %x\n",
				    nse->ne_mode);
			DMLOG_PRINT(DMLVL_DEBUG, "  File system handle: %p\n",
				    DM_GET_VALUE(nse, ne_handle1, void *));
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  File system handle length: %d\n",
				    DM_GET_LEN(nse, ne_handle1));
			DMLOG_PRINT(DMLVL_DEBUG, "  Return code: %x\n",
				    nse->ne_retcode);
			if (nse->ne_retcode == 0) {
				bMounted = DM_FALSE;
			}

			response = DM_RESP_CONTINUE;
		} else if (type == DM_EVENT_POSTCREATE) {
			/* SPECIAL CASE: need to save entry info */
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);
			hanp2 = DM_GET_VALUE(nse, ne_handle2, void *);
			hlen2 = DM_GET_LEN(nse, ne_handle2);
			strcpy(name1, DM_GET_VALUE(nse, ne_name1, char *));

			DMLOG_PRINT(DMLVL_DEBUG,
				    "Message is DM_EVENT_POSTCREATE\n");
			DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n", nse->ne_mode);
			DMLOG_PRINT(DMLVL_DEBUG, "  Parent handle: %p\n",
				    DM_GET_VALUE(nse, ne_handle1, void *));
			DMLOG_PRINT(DMLVL_DEBUG, "  Parent handle length: %d\n",
				    DM_GET_LEN(nse, ne_handle1));
			DMLOG_PRINT(DMLVL_DEBUG, "  Entry handle: %p\n", hanp2);
			DMLOG_PRINT(DMLVL_DEBUG, "  Entry handle length: %d\n",
				    hlen2);
			DMLOG_PRINT(DMLVL_DEBUG, "  Entry name: %s\n", name1);
			DMLOG_PRINT(DMLVL_DEBUG, "  Return code: %x\n",
				    nse->ne_retcode);

			/* No response needed */
			response = DM_RESP_INVALID;
		} else {
			eventReceived = type;
			response = eventResponse;

			switch (type) {
			case DM_EVENT_USER:
				{
					memcpy(&tokenReceived, &token,
					       sizeof(dm_token_t));
					msgDataLenReceived =
					    DM_GET_LEN(dmMsg, ev_data);
					memcpy(msgDataReceived,
					       DM_GET_VALUE(dmMsg, ev_data,
							    char *),
					       msgDataLenReceived);

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_USER\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Message length: %d\n",
						    msgDataLenReceived);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Message data: %s\n",
						    msgDataReceived);

					response = eventResponse;
					break;
				}

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
