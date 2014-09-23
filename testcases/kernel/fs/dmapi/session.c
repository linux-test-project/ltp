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
 * TEST CASE	: session.c
 *
 * VARIATIONS	: 35
 *
 * API'S TESTED	: dm_create_session
 * 		  dm_destroy_session
 * 		  dm_getall_sessions
 * 		  dm_query_session
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "dm_test.h"

#define NUM_SESSIONS 8

char dmMsgBuf[4096];

void LogSessions(dm_sessid_t * sid, u_int nelem)
{
	int i;

	DMLOG_PRINT(DMLVL_DEBUG, "Sessions:\n");
	for (i = 0; i < nelem; i++) {
		DMLOG_PRINT(DMLVL_DEBUG, "  element %d: %d\n", i, sid[i]);
	}
}

int main(int argc, char **argv)
{

	char *szSessionInfo = "dm_test session info";
	char *szFuncName;
	char *varstr;
	int i;
	int rc;

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	/* CANNOT DO ANYTHING WITHOUT SUCCESSFUL INITIALIZATION!!! */
	if ((rc = dm_init_service(&varstr)) != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_init_service failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else {
		int nexist;
		rc = dm_getall_sessions(0, NULL, &nexist);

		if (rc == -1 && errno == E2BIG) {
			dm_sessid_t *psid;

			DMLOG_PRINT(DMLVL_DEBUG, "%d sessions already exist\n",
				    nexist);

			if ((psid = malloc(nexist * sizeof(dm_sessid_t))) != NULL) {
				if ((rc =
				     dm_getall_sessions(nexist, psid,
							&nexist)) == 0) {
					for (rc = 0, i = 0; i < nexist; i++) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "destroying session %d\n",
							    psid[i]);
						rc |=
						    dm_destroy_session(psid[i]);
					}

					if (rc == -1) {
						DMLOG_PRINT(DMLVL_ERR,
							    "dm_destroy_session failed, unable to destroy existing sessions\n");
						DM_EXIT();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_getall_sessions failed, unable to destroy existing sessions\n");
					DM_EXIT();
				}

				free(psid);
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "malloc failed, unable to destroy existing sessions\n");
				DM_EXIT();
			}
		}
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI session tests\n");

	szFuncName = "dm_create_session";

	/*
	 * TEST    : dm_create_session - invalid oldsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 1)) {
		dm_sessid_t newsid;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid oldsid)\n", szFuncName);
		rc = dm_create_session(INVALID_ADDR, szSessionInfo, &newsid);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_session - NULL sessinfop
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 2)) {
		dm_sessid_t newsid;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL sessinfop)\n", szFuncName);
		rc = dm_create_session(DM_NO_SESSION, NULL, &newsid);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_session - invalid sessinfop
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #2 (0 return code from strnlen_user
	 * ignored, which indicated fault)
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 3)) {
		dm_sessid_t newsid;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sessinfop)\n", szFuncName);
		rc = dm_create_session(DM_NO_SESSION, (char *)INVALID_ADDR,
				       &newsid);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_session - NULL newsidp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 4)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL newsidp)\n", szFuncName);
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, NULL);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_session - invalid newsidp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 5)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid newsidp)\n", szFuncName);
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo,
				       (dm_sessid_t *) INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_session - DM_NO_SESSION oldsid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 6)) {
		dm_sessid_t newsid;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION oldsid)\n",
			    szFuncName);
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "newsid = %d\n", newsid);
		}
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		rc = dm_destroy_session(newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : dm_create_session - valid oldsid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 7)) {
		dm_sessid_t newsid, oldsid;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			oldsid = newsid;

			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid oldsid)\n",
				    szFuncName);
			rc = dm_create_session(oldsid, szSessionInfo, &newsid);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "newsid = %d\n",
					    newsid);
			}
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_create_session - invalidated oldsid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 8)) {
		dm_sessid_t newsid, oldsid, delsid;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_session(oldsid =
					   newsid, szSessionInfo,
					   &newsid)) == -1) {
			dm_destroy_session(oldsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			delsid = newsid;

			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated oldsid)\n",
				    szFuncName);
			rc = dm_create_session(oldsid, szSessionInfo, &newsid);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(delsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_create_session - maximum sessinfo
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 9)) {
		dm_sessid_t newsid;
		char *szBig =
		    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345";

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(max sessionfo)\n", szFuncName);
		rc = dm_create_session(DM_NO_SESSION, szBig, &newsid);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "newsid = %d\n", newsid);
		}
		DMVAR_ENDPASSEXP(szFuncName, 0, rc);

		/* Variation clean up */
		rc = dm_destroy_session(newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : dm_create_session - sessinfo too big
	 * EXPECTED: rc = -1, errno = E2BIG
	 *
	 * This variation uncovered XFS BUG #1 (sessinfo simply truncated, API
	 * passed)
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 10)) {
		dm_sessid_t newsid;
		char *szTooBig =
		    "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456";

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(sessinfo too big)\n", szFuncName);
		rc = dm_create_session(DM_NO_SESSION, szTooBig, &newsid);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_create_session - multiple sessions with same sessinfo
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(CREATE_SESSION_BASE + 11)) {
		dm_sessid_t newsid1, newsid2;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid1);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(same sessinfo)\n",
				    szFuncName);
			rc = dm_create_session(DM_NO_SESSION, szSessionInfo,
					       &newsid2);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "1st newsid = %d, 2nd newsid = %d\n",
					    newsid1, newsid2);
				if (newsid1 != newsid2) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but session IDs same\n",
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
			rc = dm_destroy_session(newsid1);
			rc |= dm_destroy_session(newsid2);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	szFuncName = "dm_destroy_session";

	/*
	 * TEST    : dm_destroy_session - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(DESTROY_SESSION_BASE + 1)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_destroy_session(DM_NO_SESSION);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_destroy_session - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(DESTROY_SESSION_BASE + 2)) {
		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_destroy_session(INVALID_ADDR);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_destroy_session - invalidated sid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(DESTROY_SESSION_BASE + 3)) {
		dm_sessid_t newsid;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) != -1) {
			rc = dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated sid)\n",
				    szFuncName);
			rc = dm_destroy_session(newsid);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_destroy_session - valid sid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(DESTROY_SESSION_BASE + 4)) {
		dm_sessid_t newsid;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid sid)\n", szFuncName);
			rc = dm_destroy_session(newsid);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_destroy_session - sid with oustanding events
	 * EXPECTED: rc = -1, erno = EBUSY
	 */
	if (DMVAR_EXEC(DESTROY_SESSION_BASE + 5)) {
		dm_sessid_t newsid;
		char buf[MSG_DATALEN];
		size_t rlen;

		/* Variation set up */
		memcpy(buf, MSG_DATA, MSG_DATALEN);
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		rc |= dm_send_msg(newsid, DM_MSGTYPE_ASYNC, MSG_DATALEN, buf);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid sid)\n", szFuncName);
			rc = dm_destroy_session(newsid);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			rc = dm_get_events(newsid, 1, 0, sizeof(dmMsgBuf),
					   dmMsgBuf, &rlen);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	szFuncName = "dm_getall_sessions";

	/*
	 * TEST    : dm_getall_sessions - NULL sidbufp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 1)) {
		dm_sessid_t newsid;
		int nelem;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL sidbufp)\n",
				    szFuncName);
			rc = dm_getall_sessions(1, NULL, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - invalid sidbufp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 2)) {
		dm_sessid_t newsid;
		int nelem;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sidbufp)\n",
				    szFuncName);
			rc = dm_getall_sessions(1, (dm_sessid_t *) INVALID_ADDR,
						&nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - NULL nelemp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 3)) {
		dm_sessid_t newsid, sidArray[1];

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL nelemp)\n",
				    szFuncName);
			rc = dm_getall_sessions(1, sidArray, NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - invalid nelemp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 4)) {
		dm_sessid_t newsid, sidArray[1];

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelemp)\n",
				    szFuncName);
			rc = dm_getall_sessions(1, sidArray,
						(u_int *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - zero nelem, zero sessions
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 5)) {
		dm_sessid_t sidArray[1];
		int nelem;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(zero nelem, zero sessions)\n",
			    szFuncName);
		rc = dm_getall_sessions(0, sidArray, &nelem);
		if (rc == 0) {
			DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
			if (nelem == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s passed with expected rc = %d\n",
					    szFuncName, 0);
				DMVAR_PASS();
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
					    szFuncName, 0, nelem, 0);
				DMVAR_FAIL();
			}
		} else {
			DMLOG_PRINT(DMLVL_ERR,
				    "%s failed with unexpected rc = %d (errno = %d)\n",
				    szFuncName, rc, errno);
			DMVAR_FAIL();
		}

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_getall_sessions - zero nelem, one session
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 6)) {
		dm_sessid_t newsid, sidArray[1];
		int nelem;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(zero nelem, one session)\n",
				    szFuncName);
			rc = dm_getall_sessions(0, sidArray, &nelem);

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
						DMVAR_PASS();
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - one nelem, one session
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 7)) {
		dm_sessid_t newsid, sidArray[1];
		int nelem;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one nelem, one session)\n",
				    szFuncName);
			rc = dm_getall_sessions(1, sidArray, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);

				if (nelem == 1) {
					LogSessions(sidArray, nelem);

					if (newsid == sidArray[0]) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and nelem = %d but unexpected session ID (%d vs %d)\n",
							    szFuncName, 0,
							    nelem, newsid,
							    sidArray[0]);
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - two nelem, one session
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 8)) {
		dm_sessid_t newsid, sidArray[2];
		int nelem;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(two nelem, one session)\n",
				    szFuncName);
			rc = dm_getall_sessions(2, sidArray, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);

				if (nelem == 1) {
					LogSessions(sidArray, nelem);

					if (newsid == sidArray[0]) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and nelem = %d but unexpected session ID (%d vs %d)\n",
							    szFuncName, 0,
							    nelem, newsid,
							    sidArray[0]);
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_getall_sessions - ten nelem, eight sessions
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_SESSIONS_BASE + 9)) {
		dm_sessid_t sidExpected[NUM_SESSIONS], sidArray[10];
		int nelem;

		/* Variation set up */
		for (i = 0, rc = 0; i < NUM_SESSIONS && rc == 0; i++) {
			rc = dm_create_session(DM_NO_SESSION, szSessionInfo,
					       &sidExpected[i]);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			for (i--; i >= 0; i--) {
				dm_destroy_session(sidExpected[i]);
			}
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(%d nelem, %d sessions)\n",
				    szFuncName,
				    sizeof(sidArray) / sizeof(dm_sessid_t),
				    NUM_SESSIONS);
			rc = dm_getall_sessions(sizeof(sidArray) /
						sizeof(dm_sessid_t), sidArray,
						&nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);

				if (nelem == NUM_SESSIONS) {
					LogSessions(sidArray, nelem);

					if (memcmp
					    (sidArray, sidExpected,
					     NUM_SESSIONS *
					     sizeof(dm_sessid_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and nelem = %d but unexpected session ID(s)\n",
							    szFuncName, 0,
							    nelem);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    NUM_SESSIONS);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			for (i = 0, rc = 0; i < NUM_SESSIONS; i++) {
				rc |= dm_destroy_session(sidExpected[i]);
			}
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	szFuncName = "dm_query_session";

	/*
	 * TEST    : dm_query_session - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 1)) {
		char buf[64];
		size_t rlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_query_session(DM_NO_SESSION, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_session - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 2)) {
		char buf[64];
		size_t rlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_query_session(INVALID_ADDR, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_query_session - invalidated sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 3)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) != -1) {
			rc = dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated sid)\n",
				    szFuncName);
			rc = dm_query_session(newsid, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : dm_query_session - NULL bufp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 4)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL bufp)\n", szFuncName);
			rc = dm_query_session(newsid, sizeof(buf), NULL, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_session - invalid bufp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 5)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid bufp)\n",
				    szFuncName);
			rc = dm_query_session(newsid, sizeof(buf),
					      (void *)INVALID_ADDR, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_session - NULL rlenp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 6)) {
		dm_sessid_t newsid;
		char buf[64];

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL rlenp)\n",
				    szFuncName);
			rc = dm_query_session(newsid, sizeof(buf), buf, NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_session - invalid rlenp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 7)) {
		dm_sessid_t newsid;
		char buf[64];

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid rlenp)\n",
				    szFuncName);
			rc = dm_query_session(newsid, sizeof(buf), buf,
					      (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_session - zero buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 8)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(buflen zero)\n",
				    szFuncName);
			rc = dm_query_session(newsid, 0, buf, &rlen);
			if (rc == -1) {
				if (errno == E2BIG) {
					DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n",
						    rlen);

					if (rlen == strlen(szSessionInfo) + 1) {
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
							    strlen
							    (szSessionInfo) +
							    1);
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_session - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 9)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName,
				    sizeof(buf));
			rc = dm_query_session(newsid, sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);

				if (rlen == strlen(szSessionInfo) + 1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buf = \"%s\"\n", buf);

					if (strcmp(buf, szSessionInfo) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and rlen = %d but unexpected session info (\"%s\" vs \"%s\")\n",
							    szFuncName, 0, rlen,
							    buf, szSessionInfo);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen (%d vs %d)\n",
						    szFuncName, 0, rlen,
						    strlen(szSessionInfo) + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_query_session - maximum sessionfo
	 *           sessioninfo
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(QUERY_SESSION_BASE + 10)) {
		dm_sessid_t newsid;
		char buf[512];
		size_t rlen;
		char *szBig =
		    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345";

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szBig, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(max sessinfo)\n",
				    szFuncName, sizeof(buf));
			rc = dm_query_session(newsid, sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);

				if (rlen == DM_SESSION_INFO_LEN) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buf = \"%s\"\n", buf);

					if (strncmp
					    (buf, szBig,
					     DM_SESSION_INFO_LEN - 1) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and rlen = %d but unexpected session info (\"%s\" vs \"%s\")\n",
							    szFuncName, 0, rlen,
							    buf, szSessionInfo);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen (%d vs %d)\n",
						    szFuncName, 0, rlen,
						    DM_SESSION_INFO_LEN);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	DMLOG_STOP();

	tst_exit();

}
