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
 * TEST CASE	: disp.c
 *
 * VARIATIONS	: 155
 *
 * API'S TESTED	: dm_get_config_events
 * 		  dm_set_disp
 * 		  dm_getall_disp
 * 		  dm_set_eventlist
 * 		  dm_get_eventlist
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

#define TMP_FILELEN 1000

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char command[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummyFile2[FILENAME_MAX];
char DummySubdir[FILENAME_MAX];

void *Thread(void *);

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int i;
	int rc;
	char *szSessionInfo = "dm_test session info";
	dm_eventset_t events, maxEvents, minEvents;
	void *fshanp;
	size_t fshlen;
	int varNum;

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
		int fd;

		sprintf(DummyFile, "%s/%s", mountPt, DUMMY_FILE);
		sprintf(DummyFile2, "%s/%s", mountPt, DUMMY_FILE2);
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);

		remove(DummyFile);
		remove(DummyFile2);
		rmdir(DummySubdir);

		EVENT_DELIVERY_DELAY;
		fd = open(DummyFile, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		if (fd != -1) {
			for (i = 0; i < TMP_FILELEN / DUMMY_STRLEN; i++) {
				if (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
				    DUMMY_STRLEN) {
					rc = -1;
					break;
				}
			}
		} else {
			rc = -1;
		}
		if (rc != -1) {
			rc = fsync(fd);
		}
		if (rc != -1) {
			rc = close(fd);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "creating dummy file failed! (rc = %d, errno = %d)\n",
				    rc, errno);
			dm_destroy_session(sid);
			DM_EXIT();
		}

		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_ERR,
				    "creating dummy dir failed! (rc = %d, errno = %d)\n",
				    rc, errno);
			dm_destroy_session(sid);
			DM_EXIT();
		}

		DMEV_ZERO(maxEvents);
		DMEV_SET(DM_EVENT_PREUNMOUNT, maxEvents);
		DMEV_SET(DM_EVENT_UNMOUNT, maxEvents);
		DMEV_SET(DM_EVENT_CREATE, maxEvents);
		DMEV_SET(DM_EVENT_CLOSE, maxEvents);
		DMEV_SET(DM_EVENT_POSTCREATE, maxEvents);
		DMEV_SET(DM_EVENT_REMOVE, maxEvents);
		DMEV_SET(DM_EVENT_POSTREMOVE, maxEvents);
		DMEV_SET(DM_EVENT_RENAME, maxEvents);
		DMEV_SET(DM_EVENT_POSTRENAME, maxEvents);
		DMEV_SET(DM_EVENT_LINK, maxEvents);
		DMEV_SET(DM_EVENT_POSTLINK, maxEvents);
		DMEV_SET(DM_EVENT_SYMLINK, maxEvents);
		DMEV_SET(DM_EVENT_POSTSYMLINK, maxEvents);
		DMEV_SET(DM_EVENT_ATTRIBUTE, maxEvents);
		DMEV_SET(DM_EVENT_DESTROY, maxEvents);
		DMEV_SET(DM_EVENT_NOSPACE, maxEvents);

		DMEV_ZERO(minEvents);
		DMEV_SET(DM_EVENT_PREUNMOUNT, minEvents);
		DMEV_SET(DM_EVENT_UNMOUNT, minEvents);
	}

	DMLOG_PRINT(DMLVL_DEBUG,
		    "Starting DMAPI disposition/eventlist tests\n");

	szFuncName = "dm_get_config_events";

	/*
	 * TEST    : dm_get_config_events - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 1)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_get_config_events((void *)INVALID_ADDR, hlen,
						  DM_EVENT_MAX, &eventset,
						  &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_config_events - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 2)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, INVALID_ADDR,
						  DM_EVENT_MAX, &eventset,
						  &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_config_events - invalid nelem
	 * EXPECTED: rc = -1, errno = E2BIG
	 *
	 * This variation uncovered XFS BUG #18 (nelem not updated)
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelem)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, hlen, 0, &eventset,
						  &nelem);
			if (rc == -1) {
				if (errno == E2BIG) {
					if (nelem == DM_EVENT_MAX) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and expected errno = %d (nelem = %d)\n",
							    szFuncName, rc,
							    errno, nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
							    szFuncName, rc,
							    nelem,
							    DM_EVENT_MAX);
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
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_config_events - invalid eventsetp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 4)) {
		void *hanp;
		size_t hlen;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid eventsetp)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, hlen, DM_EVENT_MAX,
						  (dm_eventset_t *)
						  INVALID_ADDR, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_config_events - invalid nelemp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 5)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelemp)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, hlen, DM_EVENT_MAX,
						  &eventset,
						  (u_int *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_config_events - fs handle
	 * EXPECTED: rc = 0
	 *
	 * This variation uncovered XFS BUG #19 (DM_EVENT_USER not returned)
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 6)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_config_events(hanp, hlen, DM_EVENT_MAX,
						  &eventset, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_MAX) {
					if (memcmp
					    (&eventset, &dmimpl_eventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    eventset,
							    dmimpl_eventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_DESTROY + 1);
					DMVAR_FAIL();
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
	}

	/*
	 * TEST    : dm_get_config_events - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 7)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_handle(DummyFile, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, hlen, DM_EVENT_MAX,
						  &eventset, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_MAX) {
					if (memcmp
					    (&eventset, &dmimpl_eventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    eventset,
							    dmimpl_eventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_DESTROY + 1);
					DMVAR_FAIL();
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
	}

	/*
	 * TEST    : dm_get_config_events - dir handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 8)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_path_to_handle(DummySubdir, &hanp, &hlen);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, hlen, DM_EVENT_MAX,
						  &eventset, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_MAX) {
					if (memcmp
					    (&eventset, &dmimpl_eventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    eventset,
							    dmimpl_eventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_DESTROY + 1);
					DMVAR_FAIL();
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
	}

	/*
	 * TEST    : dm_get_config_events - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 9)) {
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_get_config_events(DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					  DM_EVENT_MAX, &eventset, &nelem);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_config_events - invalidated handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_CONFIG_EVENTS_BASE + 10)) {
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyFile, DummyFile2);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile2, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile2);
		} else if ((rc = remove(DummyFile2)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated handle)\n",
				    szFuncName);
			rc = dm_get_config_events(hanp, hlen, DM_EVENT_MAX,
						  &eventset, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_set_disp";

	/*
	 * TEST    : dm_set_disp - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 1)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(DM_NO_SESSION, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 2)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(INVALID_ADDR, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 3)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(newsid, (void *)INVALID_ADDR, hlen,
					 DM_NO_TOKEN, &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 4)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(newsid, hanp, INVALID_ADDR,
					 DM_NO_TOKEN, &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 5)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(newsid, hanp, hlen, INVALID_ADDR,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid eventsetp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 6)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid eventsetp)\n",
				    szFuncName);
			rc = dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
					 (dm_eventset_t *) INVALID_ADDR,
					 DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid maxevent
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 7)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid maxevent)\n",
				    szFuncName);
			rc = dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX + 1);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 8)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 9)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummySubdir, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - invalid global handle event
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 10)) {
		dm_sessid_t newsid;
		dm_eventset_t eventset;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_POSTCREATE, eventset);
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(invalid global handle event)\n",
				    szFuncName);
			rc = dm_set_disp(newsid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					 DM_NO_TOKEN, &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

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
	 * TEST    : dm_set_disp - invalid fs handle event
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 11)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_MOUNT, eventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(invalid fs handle event)\n",
				    szFuncName);
			rc = dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_disp - valid global handle event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 12)) {
		dm_sessid_t newsid;
		dm_eventset_t eventset;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_MOUNT, eventset);
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(valid global handle event)\n",
				    szFuncName);
			rc = dm_set_disp(newsid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					 DM_NO_TOKEN, &eventset, DM_EVENT_MAX);
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
	 * TEST    : dm_set_disp - valid fs handle event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_DISP_BASE + 13)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_POSTCREATE, eventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid fs handle event)\n",
				    szFuncName);
			rc = dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
					 &eventset, DM_EVENT_MAX);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_getall_disp";

	/*
	 * TEST    : dm_getall_disp - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 1)) {
		char buf[64];
		size_t rlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n", szFuncName);
		rc = dm_getall_disp(DM_NO_SESSION, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_getall_disp - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 2)) {
		char buf[64];
		size_t rlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n", szFuncName);
		rc = dm_getall_disp(INVALID_ADDR, sizeof(buf), buf, &rlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_getall_disp - invalidated sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 3)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		} else if ((rc = dm_destroy_session(newsid)) == -1) {
			dm_handle_free(hanp, hlen);
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
			rc = dm_getall_disp(newsid, sizeof(buf), buf, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_disp - NULL bufp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 4)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL bufp)\n", szFuncName);
			rc = dm_getall_disp(newsid, sizeof(buf), NULL, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_disp - invalid bufp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 5)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_getall_disp(newsid, sizeof(buf),
					    (void *)INVALID_ADDR, &rlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_disp - NULL rlenp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 6)) {
		dm_sessid_t newsid;
		char buf[64];
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(NULL rlenp)\n",
				    szFuncName);
			rc = dm_getall_disp(newsid, sizeof(buf), buf, NULL);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_disp - invalid rlenp
	 * EXPECTED: rc = -1, errno EFAULT
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 7)) {
		dm_sessid_t newsid;
		char buf[64];
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_getall_disp(newsid, sizeof(buf), buf,
					    (size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_disp - zero buflen
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 8)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(buflen zero)\n",
				    szFuncName);
			rc = dm_getall_disp(newsid, 0, buf, &rlen);
			if (rc == -1 && errno == E2BIG) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
			}
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_getall_disp - valid
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GETALL_DISP_BASE + 9)) {
		dm_sessid_t newsid;
		char buf[64];
		size_t rlen;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_disp(newsid, hanp, hlen, DM_NO_TOKEN,
				     &maxEvents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
			rc = dm_getall_disp(newsid, sizeof(buf), buf, &rlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "rlen = %d\n", rlen);
				dm_dispinfo_t *di = (dm_dispinfo_t *) buf;
				if (rlen == sizeof(dm_dispinfo_t) + hlen) {
					if (dm_handle_cmp
					    (hanp, hlen,
					     DM_GET_VALUE(di, di_fshandle,
							  void *),
					     DM_GET_LEN(di,
							di_fshandle)) == 0) {
						if (di->di_eventset ==
						    maxEvents) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "%s passed with expected rc = %d\n",
								    szFuncName,
								    0);
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "%s failed with expected rc = %d but unexpected eventset (%llx vs %llx)\n",
								    szFuncName,
								    0,
								    di->
								    di_eventset,
								    maxEvents);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected fs handle (%llx vs %llx)\n",
							    szFuncName, 0,
							    *(__u64 *) hanp,
							    *(__u64 *)
							    DM_GET_VALUE(di,
									 di_fshandle,
									 void
									 *));
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected rlen (%d vs %d)\n",
						    szFuncName, 0, rlen,
						    sizeof(dm_dispinfo_t) +
						    hlen);
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
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_set_eventlist";

	/*
	 * TEST    : dm_set_eventlist - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 1)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_eventlist(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, &eventset,
					      DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 2)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_eventlist(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, &eventset,
					      DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 3)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_eventlist(newsid, (void *)INVALID_ADDR,
					      hlen, DM_NO_TOKEN, &eventset,
					      DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 4)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_eventlist(newsid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, &eventset,
					      DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 5)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
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
			rc = dm_set_eventlist(newsid, hanp, hlen, INVALID_ADDR,
					      &eventset, DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - invalid eventsetp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 6)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid eventsetp)\n",
				    szFuncName);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      (dm_eventset_t *) INVALID_ADDR,
					      DM_EVENT_MAX);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - invalid maxevent
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 7)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset = minEvents;

		/* Variation set up */
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid maxevent)\n",
				    szFuncName);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &eventset, DM_EVENT_MAX + 1);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - maxevent < high set event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 8)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, eventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &eventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventset = minEvents;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(maxevent < high set event)\n",
				    szFuncName);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &eventset, DM_EVENT_ATTRIBUTE);
			if (rc == 0) {
				if ((dm_get_eventlist
				     (newsid, hanp, hlen, DM_NO_TOKEN,
				      DM_EVENT_MAX, &eventset, &nelem) == 0)
				    && (nelem == DM_EVENT_ATTRIBUTE + 1)
				    &&
				    (DMEV_ISSET(DM_EVENT_ATTRIBUTE, eventset)))
				{
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but high set event cleared\n",
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_set_eventlist - maxevent > high set event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_EVENTLIST_BASE + 9)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t eventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(eventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, eventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &eventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventset = minEvents;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(maxevent > high set event)\n",
				    szFuncName);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &eventset, DM_EVENT_MAX);
			if (rc == 0) {
				if ((dm_get_eventlist
				     (newsid, hanp, hlen, DM_NO_TOKEN,
				      DM_EVENT_MAX, &eventset, &nelem) == 0)
				    && (nelem == DM_EVENT_UNMOUNT + 1)
				    &&
				    (!DMEV_ISSET(DM_EVENT_ATTRIBUTE, eventset)))
				{
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but high set event still set\n",
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * Global handle dm_set_eventlist variations
	 *
	 * This variation uncovered XFS BUG #15 (EBADF errno returned instead
	 * of EINVAL)
	 */
	for (i = 0, varNum = SET_EVENTLIST_BASE + 10; i < DM_EVENT_MAX;
	     i++, varNum++) {
		if (DMVAR_EXEC(varNum)) {
			dm_sessid_t newsid;
			dm_eventset_t eventset;

			/* Variation set up */
			DMEV_ZERO(eventset);
			DMEV_SET(i, eventset);
			rc = dm_create_session(DM_NO_SESSION, szSessionInfo,
					       &newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
				DMVAR_SKIP();
			} else {
				/* Variation */
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s(global handle, %s)\n",
					    szFuncName,
					    dmimpl_validEvents[i].name);
				rc = dm_set_eventlist(newsid, DM_GLOBAL_HANP,
						      DM_GLOBAL_HLEN,
						      DM_NO_TOKEN, &eventset,
						      DM_EVENT_MAX);
				DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

				/* Variation clean up */
				rc = dm_destroy_session(newsid);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "Unable to clean up variation! (errno = %d)\n",
						    errno);
				}
			}
		}
	}

	/*
	 * File system handle dm_set_eventlist variations
	 */
	for (i = 0; i < DM_EVENT_MAX; i++, varNum++) {
		if (DMVAR_EXEC(varNum)) {
			dm_sessid_t newsid;
			void *hanp;
			size_t hlen;
			dm_eventset_t eventset;

			/* Variation set up */
			DMEV_ZERO(eventset);
			DMEV_SET(i, eventset);
			if ((rc =
			     dm_create_session(DM_NO_SESSION, szSessionInfo,
					       &newsid)) == -1) {
				/* No clean up */
			} else
			    if ((rc =
				 dm_path_to_fshandle(DummyFile, &hanp,
						     &hlen)) == -1) {
				dm_destroy_session(newsid);
			}
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
				DMVAR_SKIP();
			} else {
				/* Variation */
				DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle, %s)\n",
					    szFuncName,
					    dmimpl_validEvents[i].name);
				rc = dm_set_eventlist(newsid, hanp, hlen,
						      DM_NO_TOKEN, &eventset,
						      DM_EVENT_MAX);
				if (dmimpl_validEvents[i].bFSHandle)
					DMVAR_ENDPASSEXP(szFuncName, 0, rc);
				else
					DMVAR_ENDFAILEXP(szFuncName, -1, rc,
							 EINVAL);

				/* Variation clean up */
				rc = dm_destroy_session(newsid);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "Unable to clean up variation! (errno = %d)\n",
						    errno);
				}
				dm_handle_free(hanp, hlen);
			}
		}
	}

	/*
	 * Directory handle dm_set_eventlist variations
	 */
	for (i = 0; i < DM_EVENT_MAX; i++, varNum++) {
		if (DMVAR_EXEC(varNum)) {
			dm_sessid_t newsid;
			void *hanp;
			size_t hlen;
			dm_eventset_t eventset;

			/* Variation set up */
			DMEV_ZERO(eventset);
			DMEV_SET(i, eventset);
			if ((rc =
			     dm_create_session(DM_NO_SESSION, szSessionInfo,
					       &newsid)) == -1) {
				/* No clean up */
			} else
			    if ((rc =
				 dm_path_to_handle(DummySubdir, &hanp,
						   &hlen)) == -1) {
				dm_destroy_session(newsid);
			}
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
				DMVAR_SKIP();
			} else {
				/* Variation */
				DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle, %s)\n",
					    szFuncName,
					    dmimpl_validEvents[i].name);
				rc = dm_set_eventlist(newsid, hanp, hlen,
						      DM_NO_TOKEN, &eventset,
						      DM_EVENT_MAX);
				if (dmimpl_validEvents[i].bDirHandle)
					DMVAR_ENDPASSEXP(szFuncName, 0, rc);
				else
					DMVAR_ENDFAILEXP(szFuncName, -1, rc,
							 EINVAL);

				/* Variation clean up */
				rc = dm_destroy_session(newsid);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "Unable to clean up variation! (errno = %d)\n",
						    errno);
				}
				dm_handle_free(hanp, hlen);
			}
		}
	}

	/*
	 * File handle dm_set_eventlist variations
	 */
	for (i = 0; i < DM_EVENT_MAX; i++, varNum++) {
		if (DMVAR_EXEC(varNum)) {
			dm_sessid_t newsid;
			void *hanp;
			size_t hlen;
			dm_eventset_t eventset;

			/* Variation set up */
			DMEV_ZERO(eventset);
			DMEV_SET(i, eventset);
			if ((rc =
			     dm_create_session(DM_NO_SESSION, szSessionInfo,
					       &newsid)) == -1) {
				/* No clean up */
			} else
			    if ((rc =
				 dm_path_to_handle(DummyFile, &hanp,
						   &hlen)) == -1) {
				dm_destroy_session(newsid);
			}
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to set up variation! (errno = %d)\n",
					    errno);
				DMVAR_SKIP();
			} else {
				/* Variation */
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s(file handle, %s)\n", szFuncName,
					    dmimpl_validEvents[i].name);
				rc = dm_set_eventlist(newsid, hanp, hlen,
						      DM_NO_TOKEN, &eventset,
						      DM_EVENT_MAX);
				if (dmimpl_validEvents[i].bFileHandle)
					DMVAR_ENDPASSEXP(szFuncName, 0, rc);
				else
					DMVAR_ENDFAILEXP(szFuncName, -1, rc,
							 EINVAL);

				/* Variation clean up */
				rc = dm_destroy_session(newsid);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "Unable to clean up variation! (errno = %d)\n",
						    errno);
				}
				dm_handle_free(hanp, hlen);
			}
		}
	}

	szFuncName = "dm_get_eventlist";

	/*
	 * TEST    : dm_get_eventlist - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 1)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_get_eventlist(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, DM_EVENT_MAX,
					      &outeventset, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 2)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_get_eventlist(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, DM_EVENT_MAX,
					      &outeventset, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 3)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_get_eventlist(newsid, (void *)INVALID_ADDR,
					      hlen, DM_NO_TOKEN, DM_EVENT_MAX,
					      &outeventset, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 4)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_get_eventlist(newsid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, DM_EVENT_MAX,
					      &outeventset, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 5)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_get_eventlist(newsid, hanp, hlen, INVALID_ADDR,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #16 (0 returned instead of -1 and
	 * errno EINVAL)
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 6)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummySubdir, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
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
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid nelem, fs handle
	 * EXPECTED: rc = -1, errno = E2BIG
	 *
	 * This variation uncovered XFS BUG #17 (EINVAL errno returned instead
	 * of E2BIG, nelemp not updated)
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 7)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(invalid nelem, fs handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      0, &outeventset, &nelem);
			if (rc == -1) {
				if (errno == E2BIG) {
					if (nelem == DM_EVENT_ATTRIBUTE + 1) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and expected errno = %d (nelem = %d)\n",
							    szFuncName, rc,
							    errno, nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
							    szFuncName, rc,
							    nelem,
							    DM_EVENT_ATTRIBUTE +
							    1);
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
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid nelem, file handle
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 8)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(invalid nelem, file handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      0, &outeventset, &nelem);
			if (rc == -1) {
				if (errno == E2BIG) {
					if (nelem == DM_EVENT_ATTRIBUTE + 1) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and expected errno = %d (nelem = %d)\n",
							    szFuncName, rc,
							    errno, nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
							    szFuncName, rc,
							    nelem,
							    DM_EVENT_ATTRIBUTE +
							    1);
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
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid eventsetp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 9)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid eventsetp)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX,
					      (dm_eventset_t *) INVALID_ADDR,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - invalid nelemp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 10)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelemp)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      (u_int *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - zero event, fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 11)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(zero event, fs handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == 0) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - zero event, file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 12)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(zero event, file handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == 0) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
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
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - one event, fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 13)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one event, fs handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_ATTRIBUTE + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_ATTRIBUTE + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - one event, file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 14)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(one event, file handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_ATTRIBUTE + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_ATTRIBUTE + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - multi event, fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 15)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_CLOSE, ineventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		DMEV_SET(DM_EVENT_DESTROY, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(multi event, fs handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_DESTROY + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_DESTROY + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - multi event, file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 16)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_CLOSE, ineventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		DMEV_SET(DM_EVENT_DESTROY, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					  &ineventset, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(multi event, file handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_DESTROY + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_DESTROY + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - read event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 17)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;
		dm_region_t region = { 0, 0, DM_REGION_READ };
		dm_boolean_t exactflag;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_READ, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_region(newsid, hanp, hlen, DM_NO_TOKEN, 1,
				       &region, &exactflag)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(read event)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_READ + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_READ + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - write event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 18)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;
		dm_region_t region = { 0, 10, DM_REGION_WRITE };
		dm_boolean_t exactflag;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_WRITE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_region(newsid, hanp, hlen, DM_NO_TOKEN, 1,
				       &region, &exactflag)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(write event)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_WRITE + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_WRITE + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - truncate event
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 19)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;
		dm_region_t region =
		    { TMP_FILELEN - 10, 0, DM_REGION_TRUNCATE };
		dm_boolean_t exactflag;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_TRUNCATE, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if ((rc =
			 dm_set_region(newsid, hanp, hlen, DM_NO_TOKEN, 1,
				       &region, &exactflag)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(truncate event)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_TRUNCATE + 1) {
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_TRUNCATE + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - event union, file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 20)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;
		dm_region_t region =
		    { TMP_FILELEN - 10, 0, DM_REGION_TRUNCATE };
		dm_boolean_t exactflag;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);
		DMEV_SET(DM_EVENT_CLOSE, ineventset);
		DMEV_SET(DM_EVENT_ATTRIBUTE, ineventset);
		DMEV_SET(DM_EVENT_DESTROY, ineventset);
		if ((rc =
		     dm_create_session(DM_NO_SESSION, szSessionInfo,
				       &newsid)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
		} else
		    if (((rc =
			  dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					   &ineventset, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(newsid, hanp, hlen, DM_NO_TOKEN, 1,
					&region, &exactflag)) == -1)) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(event union, file handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
				if (nelem == DM_EVENT_DESTROY + 1) {
					DMEV_SET(DM_EVENT_TRUNCATE, ineventset);
					if (memcmp
					    (&ineventset, &outeventset,
					     sizeof(dm_eventset_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but eventsets not same (%llx vs %llx)\n",
							    szFuncName, 0,
							    ineventset,
							    outeventset);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelem (%d vs %d)\n",
						    szFuncName, 0, nelem,
						    DM_EVENT_DESTROY + 1);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			DMEV_ZERO(ineventset);
			rc = dm_set_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      &ineventset, DM_EVENT_MAX);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_eventlist - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 21)) {
		dm_sessid_t newsid;
		dm_eventset_t outeventset;
		u_int nelem;

		/* Variation set up */
		rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &newsid);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */

			DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, DM_GLOBAL_HANP,
					      DM_GLOBAL_HLEN, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

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
	 * TEST    : dm_get_eventlist - invalidated file handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_EVENTLIST_BASE + 22)) {
		dm_sessid_t newsid;
		void *hanp;
		size_t hlen;
		dm_eventset_t ineventset, outeventset;
		u_int nelem;

		/* Variation set up */
		DMEV_ZERO(ineventset);
		DMEV_ZERO(outeventset);

		sprintf(command, "cp %s %s", DummyFile, DummyFile2);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_create_session(DM_NO_SESSION, szSessionInfo,
					   &newsid)) == -1) {
			remove(DummyFile2);
		} else if ((rc = dm_path_to_handle(DummyFile2, &hanp, &hlen)) ==
			   -1) {
			dm_destroy_session(newsid);
			remove(DummyFile2);
		} else if ((rc = remove(DummyFile2)) == -1) {
			dm_handle_free(hanp, hlen);
			dm_destroy_session(newsid);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(invalidated file handle)\n",
				    szFuncName);
			rc = dm_get_eventlist(newsid, hanp, hlen, DM_NO_TOKEN,
					      DM_EVENT_MAX, &outeventset,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/* We now need to repoint preunmount/unmount back to original session */
	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_PREUNMOUNT, events);
	DMEV_SET(DM_EVENT_UNMOUNT, events);
	rc = dm_path_to_fshandle(DummyFile, &fshanp, &fshlen);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_path_to_fshandle failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	rc = dm_set_disp(sid, fshanp, fshlen, DM_NO_TOKEN, &events,
			 DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_disp failed! (rc = %d, errno = %d)\n", rc,
			    errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	rc = dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	remove(DummyFile);
	rmdir(DummySubdir);
	dm_handle_free(fshanp, fshlen);

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
		} else {
			switch (type) {
			case DM_EVENT_PREUNMOUNT:
			case DM_EVENT_READ:
			case DM_EVENT_WRITE:
			case DM_EVENT_TRUNCATE:
			case DM_EVENT_CREATE:
			case DM_EVENT_REMOVE:
			case DM_EVENT_RENAME:
			case DM_EVENT_LINK:
			case DM_EVENT_SYMLINK:
				response = DM_RESP_CONTINUE;
				break;

			case DM_EVENT_POSTCREATE:
			case DM_EVENT_POSTREMOVE:
			case DM_EVENT_POSTRENAME:
			case DM_EVENT_POSTLINK:
			case DM_EVENT_POSTSYMLINK:
			case DM_EVENT_ATTRIBUTE:
			case DM_EVENT_CLOSE:
				response = DM_RESP_INVALID;
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
