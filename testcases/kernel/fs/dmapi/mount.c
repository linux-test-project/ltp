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
 * TEST CASE	: mount.c
 *
 * VARIATIONS	: 12
 *
 * API'S TESTED	: dm_get_mountinfo
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

#define MOUNTEVENT_LEN 100

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummySubdir[FILENAME_MAX];
dm_mount_event_t *me_ptr;
dm_size_t me_len;

void *Thread(void *);

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
		sprintf(DummyFile, "%s/%s", mountPt, DUMMY_FILE);
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);

		remove(DummyFile);
		rmdir(DummySubdir);
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI mount tests\n");

	szFuncName = "dm_set_dmattr";

	/*
	 * TEST    : dm_get_mountinfo - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, (void *)INVALID_ADDR, hlen,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, hanp, hlen, INVALID_ADDR,
					      sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - invalid buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, hanp, hlen, DM_NO_TOKEN, 0,
					      buf, &rlen);
			if (rc == -1) {
				if (errno == E2BIG) {
					DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n",
						    rlen);
					if (rlen == me_len) {
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
							    me_len);
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
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), (void *)INVALID_ADDR,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf,
					      (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
			rc = dm_get_mountinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				if (rlen == me_len) {
					if (memcmp(buf, me_ptr, rlen) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but mount info not same\n",
							    szFuncName, 0);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but mount info len not same (%d vs %d)\n",
						    szFuncName, 0, rlen,
						    me_len);
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
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, sizeof(buf), buf,
					      &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 10)) {
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_get_mountinfo(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      DM_NO_TOKEN, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_mountinfo - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
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
			rc = dm_get_mountinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_mountinfo - dir handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_MOUNTINFO_BASE + 12)) {
		void *hanp;
		size_t hlen;
		char buf[MOUNTEVENT_LEN];
		size_t rlen;

		/* Variation set up */
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummySubdir, &hanp, &hlen))
			   == -1) {
			rmdir(DummySubdir);
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
			rc = dm_get_mountinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	if (me_ptr != NULL) {
		free(me_ptr);
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

			me_len =
			    me->me_roothandle.vd_offset +
			    me->me_roothandle.vd_length;
			if ((me_ptr = malloc(me_len)) == NULL) {
				DMLOG_PRINT(DMLVL_ERR,
					    "malloc failed! (errno = %d)\n",
					    errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}
			memcpy(me_ptr, me, me_len);

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
