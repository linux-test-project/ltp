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
 * TEST CASE	: token.c
 *
 * VARIATIONS	: 9
 *
 * API'S TESTED	: dm_getall_tokens
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include "dm_test.h"

#define TOKBUF_NUM 10
#define TOKBUF_LEN (TOKBUF_NUM * sizeof(dm_token_t))

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char *mountPt;
char *deviceNm;
char DummySubdir[FILENAME_MAX];
dm_token_t createToken;

void *Thread(void *);
void *TokenThread(void *);

int main(int argc, char **argv)
{

	char *szFuncName;
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

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI tokens tests\n");

	szFuncName = "dm_getall_tokens";

	/*
	 * TEST    : dm_getall_tokens - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 1)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, msgbuf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_getall_tokens(INVALID_ADDR, TOKBUF_NUM, buf,
					      &nelem);
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
	 * TEST    : dm_getall_tokens - invalid nelem
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 2)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, msgbuf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelem)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, 0, buf, &nelem);
			if (rc == -1) {
				if (errno == E2BIG) {
					DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n",
						    nelem);
					if (nelem == 1) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and expected errno = %d\n",
							    szFuncName, -1,
							    E2BIG);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and expected errno = %d but unexpected nelem (%d vs %d)\n",
							    szFuncName, -1,
							    E2BIG, nelem, 1);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected errno = %d\n",
						    szFuncName, -1, errno);
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
	 * TEST    : dm_getall_tokens - invalid tokenbufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 3)) {
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, msgbuf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid tokenbufp)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, TOKBUF_NUM,
					      (dm_token_t *) INVALID_ADDR,
					      &nelem);
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
	 * TEST    : dm_getall_tokens - invalid nelemp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 4)) {
		dm_token_t buf[TOKBUF_NUM];
		char msgbuf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, msgbuf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelemp)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, TOKBUF_NUM, buf,
					      (u_int *) INVALID_ADDR);
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
	 * TEST    : dm_getall_tokens - one userevent token
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 5)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, msgbuf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one userevent token)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, TOKBUF_NUM, buf, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == 1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "token 0: %d\n", buf[0]);
					if (token == buf[0]) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected token (%d vs %d)\n",
							    szFuncName, 0,
							    token, buf[0]);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem, 1);
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
	 * TEST    : dm_getall_tokens - two userevent tokens
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 6)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token1, token2;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		if ((rc =
		     dm_create_userevent(sid, MSG_DATALEN, msgbuf,
					 &token1)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_userevent(sid, MSG_DATALEN, msgbuf,
					     &token2)) == -1) {
			dm_respond_event(sid, token1, DM_RESP_CONTINUE, 0, 0,
					 NULL);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(two userevent tokens)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, TOKBUF_NUM, buf, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == 2) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "token 0: %d\n", buf[0]);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "token 1: %d\n", buf[1]);
					if (token1 == buf[0]
					    || token1 == buf[1]) {
						if (token2 == buf[0]
						    || token2 == buf[1]) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s passed with expected rc = %d\n",
								    szFuncName,
								    0);
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s failed with expected rc = %d but token = %d not in buf\n",
								    szFuncName,
								    0, token2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but token = %d not in buf\n",
							    szFuncName, 0,
							    token1);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem, 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_respond_event(sid, token2, DM_RESP_CONTINUE, 0,
					      0, NULL);
			rc |=
			    dm_respond_event(sid, token1, DM_RESP_CONTINUE, 0,
					     0, NULL);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_tokens - one event token
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 7)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		pthread_t tidToken;

		/* Variation set up */
		createToken = 0;
		rc = pthread_create(&tidToken, NULL, TokenThread, NULL);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			/* Wait for DM_EVENT_CREATE event to set token */
			while (createToken == 0) {
				EVENT_DELIVERY_DELAY;
			}
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one event token)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, TOKBUF_NUM, buf, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == 1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "token 0: %d\n", buf[0]);
					if (buf[0] == createToken) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected token (%d vs %d)\n",
							    szFuncName, 0,
							    buf[0],
							    createToken);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem, 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			pthread_join(tidToken, NULL);
		}
	}

	/*
	 * TEST    : dm_getall_tokens - one event token, one userevent token
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 8)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token;
		pthread_t tidToken;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		createToken = 0;
		if ((rc =
		     dm_create_userevent(sid, MSG_DATALEN, msgbuf,
					 &token)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 pthread_create(&tidToken, NULL, TokenThread,
					NULL)) == -1) {
			dm_respond_event(sid, token, DM_RESP_CONTINUE, 0, 0,
					 NULL);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			/* Wait for DM_EVENT_CREATE event to set token */
			while (createToken == 0) {
				EVENT_DELIVERY_DELAY;
			}
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(one event token, one userevent token)\n",
				    szFuncName);
			rc = dm_getall_tokens(sid, TOKBUF_NUM, buf, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == 2) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "token 0: %d\n", buf[0]);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "token 1: %d\n", buf[1]);
					if (token == buf[0] || token == buf[1]) {
						if (createToken == buf[0]
						    || createToken == buf[1]) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s passed with expected rc = %d\n",
								    szFuncName,
								    0);
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s failed with expected rc = %d but token = %d not in buf\n",
								    szFuncName,
								    0,
								    createToken);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but token = %d not in buf\n",
							    szFuncName, 0,
							    token);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem, 1);
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
			pthread_join(tidToken, NULL);
		}
	}

	/*
	 * TEST    : dm_getall_tokens - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_TOKENS_BASE + 9)) {
		dm_token_t buf[TOKBUF_NUM];
		u_int nelem;
		char msgbuf[MSG_DATALEN];
		dm_token_t token;

		/* Variation set up */
		memcpy(msgbuf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_userevent(sid, MSG_DATALEN, msgbuf, &token);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION, sid)\n",
				    szFuncName);
			rc = dm_getall_tokens(DM_NO_SESSION, TOKBUF_NUM, buf,
					      &nelem);
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
			void *hanp = DM_GET_VALUE(me, me_handle1, void *);
			size_t hlen = DM_GET_LEN(me, me_handle1);

			DMLOG_PRINT(DMLVL_DEBUG, "Message is DM_EVENT_MOUNT\n");
			DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n", me->me_mode);
			DMLOG_PRINT(DMLVL_DEBUG, "  File system handle: %p\n",
				    hanp);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "  File system handle length: %d\n", hlen);
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

			bMounted = dm_handle_is_valid(hanp, hlen);

			rc = dm_request_right(sid, hanp, hlen, token,
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
			rc = dm_set_disp(sid, hanp, hlen, token, &events,
					 DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			rc = dm_set_eventlist(sid, hanp, hlen, token, &events,
					      DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_eventlist failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			rc = dm_release_right(sid, hanp, hlen, token);
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
		} else {
			switch (type) {
			case DM_EVENT_PREUNMOUNT:
				response = DM_RESP_CONTINUE;
				break;

			case DM_EVENT_CREATE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_CREATE\n");
					DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n",
						    nse->ne_mode);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    DM_GET_VALUE(nse,
								 ne_handle1,
								 void *));
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    DM_GET_LEN(nse,
							       ne_handle1));
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry name: %s\n",
						    DM_GET_VALUE(nse, ne_name1,
								 char *));

					createToken = token;
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Token: %d\n",
						    createToken);

					/* Wait for main thread to call dm_getall_tokens */
					sleep(3);

					response = DM_RESP_CONTINUE;
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

void *TokenThread(void *parm)
{
	int rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);

	if (rc != -1) {
		rmdir(DummySubdir);
	}

	pthread_exit(0);
}
