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
 * TEST CASE	: event.c
 *
 * VARIATIONS	: 41
 *
 * API'S TESTED	: dm_get_events
 * 		  dm_respond_event
 * 		  dm_move_event
 * 		  dm_pending
 */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <signal.h>
#include "dm_test.h"

#define MAX_EVENT (sizeof(dmMsgBuf)/(MSG_DATALEN+sizeof(dm_eventmsg_t)))

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char command[4096];
char mountPt[FILENAME_MAX];
char deviceNm[FILENAME_MAX];
char *szFuncName;

/* Variables for thread communications */
int expectedNumMsg;
u_int eventsFlags;
int rcRespond;
int errnoRespond;

void *Thread(void *);

void LogEventMsgs(void *bufp)
{
	int i = 0;
	dm_eventmsg_t *em = (dm_eventmsg_t *) bufp;

	while (em != NULL) {
		DMLOG_PRINT(DMLVL_DEBUG, "  eventmsg %d:\n", i++);
		DMLOG_PRINT(DMLVL_DEBUG, "    ev_type: %d\n", em->ev_type);
		DMLOG_PRINT(DMLVL_DEBUG, "    ev_token: %d\n", em->ev_token);
		DMLOG_PRINT(DMLVL_DEBUG, "    ev_sequence: %d\n",
			    em->ev_sequence);
		DMLOG_PRINT(DMLVL_DEBUG, "    ev_data: length %d, value %s\n",
			    DM_GET_LEN(em, ev_data), DM_GET_VALUE(em, ev_data,
								  dm_eventtype_t));

		em = DM_STEP_TO_NEXT(em, dm_eventmsg_t *);
	}
}

dm_eventmsg_t *GetSyncEventMsg(void *bufp)
{
	dm_eventmsg_t *em = (dm_eventmsg_t *) bufp;

	while (em != NULL) {
		if ((em->ev_type == DM_EVENT_USER)
		    && (em->ev_token != DM_INVALID_TOKEN)) {
			return em;
		}
		em = DM_STEP_TO_NEXT(em, dm_eventmsg_t *);
	}

	return NULL;
}

int GetNumEventMsg(void *bufp)
{
	dm_eventmsg_t *em = (dm_eventmsg_t *) bufp;
	int i = 0;

	while (em != NULL) {
		i++;
		em = DM_STEP_TO_NEXT(em, dm_eventmsg_t *);
	}
	return i;
}

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
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI move event tests\n");

	szFuncName = "dm_get_events";

	/*
	 * TEST    : dm_get_events - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 1)) {
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_get_events(INVALID_ADDR, 0, 0, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_events - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 2)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(GET_EVENTS_BASE + 2));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 3)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(GET_EVENTS_BASE + 3));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 4)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(GET_EVENTS_BASE + 4));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - !DM_EV_WAIT with no messages
	 * EXPECTED: rc = -1, errno = EAGAIN
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 5)) {

		/* Variation set up */
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(GET_EVENTS_BASE + 5));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - !DM_EV_WAIT with one message
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 6)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		expectedNumMsg = 1;
		eventsFlags = 0;
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(GET_EVENTS_BASE + 6));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - DM_EV_WAIT with one message
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 7)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		expectedNumMsg = 1;
		eventsFlags = DM_EV_WAIT;
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(GET_EVENTS_BASE + 7));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - !DM_EV_WAIT with two messages
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 8)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		expectedNumMsg = 2;
		eventsFlags = 0;
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_send_msg(sid, DM_MSGTYPE_ASYNC, MSG_DATALEN, buf);
		if (rc != -1) {
			rc = pthread_create(&tid, NULL, Thread,
					    (void *)(GET_EVENTS_BASE + 8));
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - DM_EV_WAIT with two messages
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 9)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		expectedNumMsg = 2;
		eventsFlags = DM_EV_WAIT;
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_send_msg(sid, DM_MSGTYPE_ASYNC, MSG_DATALEN, buf);
		if (rc != -1) {
			rc = pthread_create(&tid, NULL, Thread,
					    (void *)(GET_EVENTS_BASE + 9));
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_get_events - !DM_EV_WAIT with more than MAX_EVENT messages
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTS_BASE + 10)) {
		int i, j;
		char buf[MSG_DATALEN];

		/* Variation set up */
		expectedNumMsg = MAX_EVENT;
		eventsFlags = 0;
		for (i = 0, rc = 0; i < MAX_EVENT + 1 && rc == 0; i++) {
			j = sprintf(buf, "Multi event message %d", i);
			rc = dm_send_msg(sid, DM_MSGTYPE_ASYNC, j + 1, buf);
		}
		if (rc != -1) {
			rc = pthread_create(&tid, NULL, Thread,
					    (void *)(GET_EVENTS_BASE + 10));
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			j = sprintf(buf, "Multi event message %d", i);
			dm_send_msg(sid, DM_MSGTYPE_SYNC, j + 1, buf);
			pthread_join(tid, NULL);
		}
	}

	szFuncName = "dm_respond_event";

	/*
	 * TEST    : dm_respond_event - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 1)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_respond_event(INVALID_ADDR, token,
					      DM_RESP_CONTINUE, 0, sizeof(buf),
					      buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 2)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_respond_event(DM_NO_SESSION, token,
					      DM_RESP_CONTINUE, 0, sizeof(buf),
					      buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_respond_event - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 3)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_respond_event(sid, INVALID_ADDR, DM_RESP_CONTINUE, 0,
				      sizeof(buf), buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_respond_event - DM_NO_TOKEN token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 4)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_TOKEN token)\n", szFuncName);
		rc = dm_respond_event(sid, DM_NO_TOKEN, DM_RESP_CONTINUE, 0,
				      sizeof(buf), buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_respond_event - DM_INVALID_TOKEN token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 5)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_INVALID_TOKEN token)\n",
			    szFuncName);
		rc = dm_respond_event(sid, DM_INVALID_TOKEN, DM_RESP_CONTINUE,
				      0, sizeof(buf), buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_respond_event - invalid response
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 6)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid response)\n",
				    szFuncName);
			rc = dm_respond_event(sid, token, INVALID_ADDR, 0,
					      sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_respond_event - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 *
	 * This variation uncovered XFS BUG #37 (0 returned instead of -1 and
	 * E2BIG)
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 7)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid response)\n",
				    szFuncName);
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      INVALID_ADDR, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_respond_event - invalidated token
	 * EXPECTED: rc = -1, errno = ESRCH
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 8)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc != -1) {
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      sizeof(buf), buf);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated token)\n",
				    szFuncName);
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      sizeof(buf), buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ESRCH);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_RESP_INVALID
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 9)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(RESPOND_EVENT_BASE + 9));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_RESP_CONTINUE with zero reterror
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 10)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(RESPOND_EVENT_BASE + 10));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread continues variation */
			rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN,
					 buf);
			if (rcRespond == 0) {
				if (rc == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d and expected dm_send_msg rc = %d\n",
						    szFuncName, rcRespond, rc);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected dm_send_msg rc = %d and errno %d\n",
						    szFuncName, rcRespond, rc,
						    errno);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rcRespond,
					    errnoRespond);
				DMVAR_FAIL();
			}
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_RESP_CONTINUE with non-zero reterror
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 11)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(RESPOND_EVENT_BASE + 11));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_RESP_ABORT with zero reterror
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 12)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(RESPOND_EVENT_BASE + 12));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_RESP_ABORT with non-zero reterror
	 * EXPECTED: rc = ABORT_ERRNO
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 13)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(RESPOND_EVENT_BASE + 13));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread continues variation */
			rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN,
					 buf);
			if (rcRespond == 0) {
				if (rc == -1) {
					if (errno == ABORT_ERRNO) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and dm_send_msg rc = %d\n",
							    szFuncName,
							    rcRespond, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and dm_send_msg rc = %d but unexpected errno (%d vs %d)\n",
							    szFuncName,
							    rcRespond, rc,
							    errno, ABORT_ERRNO);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected dm_send_msg rc (%d vs %d)\n",
						    szFuncName, rcRespond, rc,
						    -1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rcRespond,
					    errnoRespond);
				DMVAR_FAIL();
			}
			pthread_join(tid, NULL);
		}
	}

	/*
	 * TEST    : dm_respond_event - DM_RESP_DONTCARE
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(RESPOND_EVENT_BASE + 14)) {
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = pthread_create(&tid, NULL, Thread,
				    (void *)(RESPOND_EVENT_BASE + 14));
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Message thread finishes off variation */
			dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
			pthread_join(tid, NULL);
		}
	}

	szFuncName = "dm_move_event";

	/*
	 * TEST    : dm_move_event - invalid srcsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 1)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid srcsid)\n",
				    szFuncName);
			rc = dm_move_event(INVALID_ADDR, token, targsid,
					   &rtoken);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - DM_NO_SESSION srcsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 2)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION srcsid)\n",
				    szFuncName);
			rc = dm_move_event(DM_NO_SESSION, token, targsid,
					   &rtoken);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 3)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
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
			rc = dm_move_event(sid, INVALID_ADDR, targsid, &rtoken);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - invalid targetsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 4)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid targetsid)\n",
				    szFuncName);
			rc = dm_move_event(sid, token, INVALID_ADDR, &rtoken);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - DM_NO_SESSION targetsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 5)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(DM_NO_SESSION targetsid)\n",
				    szFuncName);
			rc = dm_move_event(sid, token, DM_NO_SESSION, &rtoken);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - invalid rtokenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #36 (event moved to targetsid
	 * despite failure)
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 6)) {
		dm_sessid_t targsid;
		dm_token_t token;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rtokenp)\n",
				    szFuncName);
			rc = dm_move_event(sid, token, targsid,
					   (dm_token_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - token not in session
	 * EXPECTED: rc = -1, errno = ENOENT
	 *
	 * This variation uncovered XFS BUG #34 (ESRCH returned instead of
	 * ENOENT)
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 7)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(token not in session)\n",
				    szFuncName);
			rc = dm_move_event(targsid, token, sid, &rtoken);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ENOENT);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - srcsid == targetsid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 8)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(srcsid == targetsid)\n",
				    szFuncName);
			rc = dm_move_event(sid, token, sid, &rtoken);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rtoken = %d\n",
					    rtoken);
				rc = dm_find_eventmsg(sid, rtoken,
						      sizeof(dmMsgBuf),
						      dmMsgBuf, &rlen);
				if (rc == 0 && rlen > 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but token %d NOT in src/target session %d\n",
						    szFuncName, 0, token, sid);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_respond_event(sid, rtoken, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_move_event - srcsid != targetsid
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #35 (ESRCH returned instead of
	 * EINVAL)
	 */
	if (DMVAR_EXEC(MOVE_EVENT_BASE + 9)) {
		dm_sessid_t targsid;
		dm_token_t token, rtoken;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &targsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, buf,
					     &token)) == -1) {
			dm_destroy_session(targsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(srcsid != targetsid)\n",
				    szFuncName);
			rc = dm_move_event(sid, token, targsid, &rtoken);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rtoken = %d\n",
					    rtoken);
				rc = dm_find_eventmsg(sid, token,
						      sizeof(dmMsgBuf),
						      dmMsgBuf, &rlen);
				if (rc == -1 && errno == EINVAL) {
					rc = dm_find_eventmsg(targsid, rtoken,
							      sizeof(dmMsgBuf),
							      dmMsgBuf, &rlen);
					if (rc == 0 && rlen > 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but token %d NOT in target session %d\n",
							    szFuncName, 0,
							    token, targsid);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but token %d still in source session %d\n",
						    szFuncName, 0, token, sid);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_respond_event(targsid, rtoken, DM_RESP_CONTINUE,
					      0, 0, NULL);
			rc |= dm_destroy_session(targsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	szFuncName = "dm_pending";

	/*
	 * TEST    : dm_pending - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PENDING_BASE + 1)) {
		char buf[MSG_DATALEN];
		dm_token_t token;
		dm_timestruct_t delay;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_pending(INVALID_ADDR, token, &delay);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_pending - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PENDING_BASE + 2)) {
		char buf[MSG_DATALEN];
		dm_token_t token;
		dm_timestruct_t delay;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_pending(DM_NO_SESSION, token, &delay);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_pending - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PENDING_BASE + 3)) {
		dm_timestruct_t delay;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n", szFuncName);
		rc = dm_pending(sid, INVALID_ADDR, &delay);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_pending - DM_NO_TOKEN token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PENDING_BASE + 4)) {
		dm_timestruct_t delay;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_TOKEN token)\n", szFuncName);
		rc = dm_pending(sid, DM_NO_TOKEN, &delay);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_pending - DM_INVALID_TOKEN token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PENDING_BASE + 5)) {
		dm_timestruct_t delay;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_INVALID_TOKEN token)\n",
			    szFuncName);
		rc = dm_pending(sid, DM_INVALID_TOKEN, &delay);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_pending - invalid delay
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PENDING_BASE + 6)) {
		char buf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_pending(sid, token,
					(dm_timestruct_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_respond_event - invalidated token
	 * EXPECTED: rc = -1, errno = ESRCH
	 */
	if (DMVAR_EXEC(PENDING_BASE + 7)) {
		char buf[MSG_DATALEN];
		dm_token_t token;
		dm_timestruct_t delay;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc != -1) {
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      sizeof(buf), buf);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated token)\n",
				    szFuncName);
			rc = dm_pending(sid, token, &delay);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, ESRCH);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_pending - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PENDING_BASE + 8)) {
		char buf[MSG_DATALEN];
		dm_token_t token;
		dm_timestruct_t delay;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, buf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
			rc = dm_pending(sid, token, &delay);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		}
	}

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
	dm_token_t token;
	size_t rlen;
	dm_eventmsg_t *dmMsg;
	int numMsg;

	EVENT_DELIVERY_DELAY;

	switch ((long)parm) {
	case GET_EVENTS_BASE + 2:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid buflen)\n", szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, 0, dmMsgBuf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

		/* Variation clean up */
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0) {
			token = ((dm_eventmsg_t *) dmMsgBuf)->ev_token;
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno	= %d)\n",
				    errno);
		}
		break;

	case GET_EVENTS_BASE + 3:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n", szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   (void *)INVALID_ADDR, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0) {
			token = ((dm_eventmsg_t *) dmMsgBuf)->ev_token;
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno	= %d)\n",
				    errno);
		}
		break;

	case GET_EVENTS_BASE + 4:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rlenp)\n", szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, (size_t *) INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0) {
			token = ((dm_eventmsg_t *) dmMsgBuf)->ev_token;
			rc = dm_respond_event(sid, token, DM_RESP_CONTINUE, 0,
					      0, NULL);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno	= %d)\n",
				    errno);
		}
		break;

	case GET_EVENTS_BASE + 5:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(!DM_EV_WAIT with no messages)\n",
			    szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EAGAIN);

		/* Variation clean up */

		break;

	case GET_EVENTS_BASE + 6:
	case GET_EVENTS_BASE + 7:
	case GET_EVENTS_BASE + 8:
	case GET_EVENTS_BASE + 9:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(%s with %d message(s))\n",
			    szFuncName,
			    (eventsFlags & DM_EV_WAIT) ? "DM_EV_WAIT" :
			    "!DM_EV_WAIT", expectedNumMsg);
		do {
			rlen = 0;
			rc = dm_get_events(sid, MAX_EVENT, eventsFlags,
					   sizeof(dmMsgBuf), dmMsgBuf, &rlen);
		} while ((eventsFlags & DM_EV_WAIT) && (rc == -1)
			 && (errno == EINTR) && (rlen == 0));
		if (rc == 0) {
			LogEventMsgs(dmMsgBuf);
			numMsg = GetNumEventMsg(dmMsgBuf);
			dmMsg = GetSyncEventMsg(dmMsgBuf);
			DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);

			if (numMsg == expectedNumMsg) {
				if (dmMsg != NULL) {
					if (dmMsg->ev_type == DM_EVENT_USER) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected event (%d vs %d)\n",
							    szFuncName, 0,
							    dmMsg->ev_type,
							    DM_EVENT_USER);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but no synchronous event\n",
						    szFuncName, 0);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with expected rc = %d but unexpected number of events (%d vs %d)\n",
					    szFuncName, 0, numMsg,
					    expectedNumMsg);
				DMVAR_FAIL();
			}
		} else {
			dmMsg = NULL;
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with unexpected rc = %d (errno = %d)\n",
				    szFuncName, rc, errno);
			DMVAR_FAIL();
		}

		/* Variation clean up */
		rc = dm_respond_event(sid,
				      dmMsg ? dmMsg->
				      ev_token : DM_INVALID_TOKEN,
				      DM_RESP_CONTINUE, 0, 0, NULL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno	= %d)\n",
				    errno);
		}

		break;

	case GET_EVENTS_BASE + 10:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(%s with %d messages)\n",
			    szFuncName,
			    (eventsFlags & DM_EV_WAIT) ? "DM_EV_WAIT" :
			    "!DM_EV_WAIT", expectedNumMsg);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "1st call:\n");
			LogEventMsgs(dmMsgBuf);
			numMsg = GetNumEventMsg(dmMsgBuf);
			DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
			rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
					   dmMsgBuf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "2nd call:\n");
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				LogEventMsgs(dmMsgBuf);
				dmMsg = GetSyncEventMsg(dmMsgBuf);

				if (numMsg == expectedNumMsg) {
					if (dmMsg != NULL) {
						if (dmMsg->ev_type ==
						    DM_EVENT_USER) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s passed with expected rc = %d\n",
								    szFuncName,
								    0);
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s failed with expected rc = %d but unexpected event (%d vs %d)\n",
								    szFuncName,
								    0,
								    dmMsg->
								    ev_type,
								    DM_EVENT_USER);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but no synchronous event\n",
							    szFuncName, 0);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected number of events (%d vs %d)\n",
						    szFuncName, 0, numMsg,
						    expectedNumMsg);
					DMVAR_FAIL();
				}
			} else {
				dmMsg = NULL;
				DMLOG_PRINT(DMLVL_ERR,
					    "%s 2nd call failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}
		} else {
			dmMsg = NULL;
			DMLOG_PRINT(DMLVL_ERR,
				    "%s 1st call failed with unexpected rc = %d (errno = %d)\n",
				    szFuncName, rc, errno);
			DMVAR_FAIL();
		}

		/* Variation clean up */
		rc = dm_respond_event(sid,
				      dmMsg ? dmMsg->
				      ev_token : DM_INVALID_TOKEN,
				      DM_RESP_CONTINUE, 0, 0, NULL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno	= %d)\n",
				    errno);
		}

		break;

	case RESPOND_EVENT_BASE + 9:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RESP_INVALID)\n", szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0 && ((dmMsg = GetSyncEventMsg(dmMsgBuf)) != NULL)) {
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_INVALID, 0, 0, NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_CONTINUE, 0, 0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Unable to obtain synchronous event! (rc = %d, errno = %d)\n",
				    rc, errno);
		}
		break;

	case RESPOND_EVENT_BASE + 10:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG,
			    "%s(DM_RESP_CONTINUE with zero reterror)\n",
			    szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0 && ((dmMsg = GetSyncEventMsg(dmMsgBuf)) != NULL)) {
			rcRespond =
			    dm_respond_event(sid, dmMsg->ev_token,
					     DM_RESP_CONTINUE, 0, 0, NULL);
			errnoRespond = rcRespond == -1 ? errno : 0;
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Unable to obtain synchronous event! (rc = %d, errno = %d)\n",
				    rc, errno);
		}
		break;

	case RESPOND_EVENT_BASE + 11:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG,
			    "%s(DM_RESP_CONTINUE with non-zero reterror)\n",
			    szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0 && ((dmMsg = GetSyncEventMsg(dmMsgBuf)) != NULL)) {
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_CONTINUE, ABORT_ERRNO, 0,
					      NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_CONTINUE, 0, 0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Unable to obtain synchronous event! (rc = %d, errno = %d)\n",
				    rc, errno);
		}
		break;

	case RESPOND_EVENT_BASE + 12:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG,
			    "%s(DM_RESP_ABORT with zero reterror)\n",
			    szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0 && ((dmMsg = GetSyncEventMsg(dmMsgBuf)) != NULL)) {
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_ABORT, 0, 0, NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_CONTINUE, 0, 0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Unable to obtain synchronous event! (rc = %d, errno = %d)\n",
				    rc, errno);
		}
		break;

	case RESPOND_EVENT_BASE + 13:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG,
			    "%s(DM_RESP_ABORT with non-zero reterror)\n",
			    szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0 && ((dmMsg = GetSyncEventMsg(dmMsgBuf)) != NULL)) {
			rcRespond =
			    dm_respond_event(sid, dmMsg->ev_token,
					     DM_RESP_ABORT, ABORT_ERRNO, 0,
					     NULL);
			errnoRespond = rcRespond == -1 ? errno : 0;
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Unable to obtain synchronous event! (rc = %d, errno = %d)\n",
				    rc, errno);
		}
		break;

	case RESPOND_EVENT_BASE + 14:
		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_RESP_DONTCARE)\n", szFuncName);
		rc = dm_get_events(sid, MAX_EVENT, 0, sizeof(dmMsgBuf),
				   dmMsgBuf, &rlen);
		if (rc == 0 && ((dmMsg = GetSyncEventMsg(dmMsgBuf)) != NULL)) {
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_ABORT, 0, 0, NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_respond_event(sid, dmMsg->ev_token,
					      DM_RESP_CONTINUE, 0, 0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno	= %d)\n",
					    errno);
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "Unable to obtain synchronous event! (rc = %d, errno = %d)\n",
				    rc, errno);
		}
		break;

	}

	pthread_exit(0);
}
