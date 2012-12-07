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
 * TEST CASE	: pmr_post.c
 *
 * VARIATIONS	: 15
 *
 * API'S TESTED	: dm_get_region
 *
 * NOTES	: The first variation of this test case, when run after
 * 		  pmr_pre and rebooting, verifies that persistent managed
 * 		  regions work
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
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

#define TMP_FILELEN 10000

void *Thread(void *);

void LogRegions(dm_region_t * rgns, u_int nelem)
{
	int i;

	DMLOG_PRINT(DMLVL_DEBUG, "Regions:\n");
	for (i = 0; i < nelem; i++) {
		DMLOG_PRINT(DMLVL_DEBUG,
			    "  region %d: offset %lld, size %lld, flags %d\n",
			    i, rgns[i].rg_offset, rgns[i].rg_size,
			    rgns[i].rg_flags);
	}
}

int main(int argc, char **argv)
{

	char *varstr;
	int i;
	int rc;
	char *szSessionInfo = "dm_test session info";
	dm_eventset_t events;
	char buf[MSG_DATALEN];

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_MOUNT, events);

	/* CANNOT DO ANYTHING WITHOUT SUCCESSFUL INITIALIZATION */
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
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);

		/* DO NOT REMOVE DummyFile, IT HAS THE REGIONS FOR FIRST VAR */
		remove(DummySubdir);

		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
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
	}

	/* This is what kicks off the test case, variations done in thread */
	memcpy(buf, MSG_DATA, MSG_DATALEN);
	rc = dm_send_msg(sid, DM_MSGTYPE_SYNC, MSG_DATALEN, buf);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_send_msg failed! (rc = %d, errno = %d)\n", rc,
			    errno);
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

	remove(DUMMY_FILE);

	DMLOG_STOP();

	tst_exit();
}

void DoTest()
{

	int rc;
	char *szFuncName;

	DMLOG_PRINT(DMLVL_DEBUG,
		    "Starting DMAPI persistent managed regions test\n");

	szFuncName = "dm_get_region";

	/*
	 * TEST    : dm_get_region - persistent, Part II
	 * EXPECTED: rc = 0, nelem = 5
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[PMR_NUM_REGIONS];
		int varStatus;
		int i;

		/* Variation set up */
		if ((fd = open(DummyFile, O_RDWR)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(persistent, Part II)\n",
				    szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   sizeof(regbuf) / sizeof(dm_region_t),
					   regbuf, &nelem);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", nelem);
			}
			varStatus = DMSTAT_PASS;
			if (rc != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s returned unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errno);
				varStatus = DMSTAT_FAIL;
			} else if (nelem != PMR_NUM_REGIONS) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Number of regions NOT correct! (%d vs %d)\n",
					    nelem, PMR_NUM_REGIONS);
				varStatus = DMSTAT_FAIL;
			} else {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "%s returned expected rc = %d\n",
					    szFuncName, rc, errno);
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n", rc,
					    errno);
				LogRegions(regbuf, nelem);

				for (i = 0; i < PMR_NUM_REGIONS; i++) {
					if (regbuf[i].rg_offset !=
					    dm_PMR_regbuf[i].rg_offset) {
						DMLOG_PRINT(DMLVL_ERR,
							    "region %d offset NOT correct! (%lld vs %d)\n",
							    i,
							    regbuf[i].rg_offset,
							    dm_PMR_regbuf[i].
							    rg_offset);
						varStatus = DMSTAT_FAIL;
					}
					if (regbuf[i].rg_size !=
					    dm_PMR_regbuf[i].rg_size) {
						DMLOG_PRINT(DMLVL_ERR,
							    "region %d size NOT correct! (%lld vs %d)\n",
							    i,
							    regbuf[i].rg_size,
							    dm_PMR_regbuf[i].
							    rg_size);
						varStatus = DMSTAT_FAIL;
					}
					if (regbuf[i].rg_flags !=
					    dm_PMR_regbuf[i].rg_flags) {
						DMLOG_PRINT(DMLVL_ERR,
							    "region %d flags NOT correct! (%lld vs %d)\n",
							    i,
							    regbuf[i].rg_flags,
							    dm_PMR_regbuf[i].
							    rg_flags);
						varStatus = DMSTAT_FAIL;
					}
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
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
	 * TEST    : dm_get_region - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_get_region(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, nelemin, &regbuf,
					   &nelemout);
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
	 * TEST    : dm_get_region - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_get_region(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, nelemin, &regbuf,
					   &nelemout);
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
	 * TEST    : dm_get_region - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_get_region(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   nelemin, &regbuf, &nelemout);
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
	 * TEST    : dm_get_region - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_get_region(sid, hanp, hlen, INVALID_ADDR,
					   nelemin, &regbuf, &nelemout);
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
	 * TEST    : dm_get_region - invalid regbufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid regbufp)\n",
				    szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin,
					   (dm_region_t *) INVALID_ADDR,
					   &nelemout);
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
	 * TEST    : dm_get_region - invalid nelemp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelemp)\n",
				    szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin, &regbuf,
					   (u_int *) INVALID_ADDR);
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
	 * TEST    : dm_get_region - DM_SO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_get_region(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, nelemin, &regbuf,
					   &nelemout);
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
	 * TEST    : dm_get_region - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbuf, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n",
				    szFuncName);
			rc = dm_get_region(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					   DM_NO_TOKEN, nelemin, &regbuf,
					   &nelemout);
			if (rc == -1 && errno == EBADF) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n",
					    nelemout);
			}
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
	 * TEST    : dm_get_region - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 10)) {
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

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
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin, &regbuf, &nelemout);
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

	/*
	 * TEST    : dm_get_region - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 11)) {
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;

		/* Variation set up */
		nelemin = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DummySubdir, &hanp, &hlen))
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin, &regbuf, &nelemout);
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

	/*
	 * TEST    : dm_get_region - nelem 0
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbuf;

		/* Variation set up */
		nelemin = 0;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(nelem 0)\n", szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin, &regbuf, &nelemout);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n",
					    nelemout);
				if (nelemin == nelemout) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = %d\n",
						    szFuncName, 0);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelemp (%d vs %d)\n",
						    szFuncName, 0, nelemin,
						    nelemout);
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
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_region - nelem 1
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbufin, regbufout;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 1;
		memset(&regbufin, 0, sizeof(regbufin));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       &regbufin, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(nelem 1)\n", szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin, &regbufout, &nelemout);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n",
					    nelemout);
				if (nelemin == nelemout) {
					if (memcmp
					    (&regbufin, &regbufout,
					     sizeof(dm_region_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected regions\n",
							    szFuncName, 0);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "Region in:\n");
						LogRegions(&regbufin, nelemin);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "Region out:\n");
						LogRegions(&regbufout,
							   nelemout);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelemp (%d vs %d)\n",
						    szFuncName, 0, nelemin,
						    nelemout);
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
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_region - nelem 2
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 13)) {
#ifdef MULTIPLE_REGIONS
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbufin[2], regbufout[2];
		dm_boolean_t exactflag;

		/* Variation set up */
		nelemin = 2;
		regbufin[0].rg_offset = 0;
		regbufin[0].rg_size = 1000;
		regbufin[0].rg_flags = DM_REGION_READ;
		regbufin[1].rg_offset = 2000;
		regbufin[1].rg_size = 1000;
		regbufin[1].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       regbufin, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(nelem 2)\n", szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin, regbufout, &nelemout);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n",
					    nelemout);
				if (nelemin == nelemout) {
					if (memcmp
					    (&regbufin, &regbufout,
					     sizeof(dm_region_t)) == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected regions\n",
							    szFuncName, 0);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "Region in:\n");
						LogRegions(regbufin, nelemin);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "Region out:\n");
						LogRegions(regbufout, nelemout);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected nelemp (%d vs %d)\n",
						    szFuncName, 0, nelemin,
						    nelemout);
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
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with MULTIPLE_REGIONS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_get_region - regbuf too small
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(GET_REGION_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelemin, nelemout;
		dm_region_t regbufin[2], regbufout[1];
		dm_boolean_t exactflag;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelemin = 2;
		regbufin[0].rg_offset = 0;
		regbufin[0].rg_size = 1000;
		regbufin[0].rg_flags = DM_REGION_READ;
		regbufin[1].rg_offset = 2000;
		regbufin[1].rg_size = 1000;
		regbufin[1].rg_flags = DM_REGION_WRITE;
#else
		nelemin = 1;
		regbufin[0].rg_offset = 0;
		regbufin[0].rg_size = 1000;
		regbufin[0].rg_flags = DM_REGION_READ;
#endif

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelemin,
				       regbufin, &exactflag)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(regbuf too small)\n",
				    szFuncName);
			rc = dm_get_region(sid, hanp, hlen, DM_NO_TOKEN,
					   nelemin - 1, regbufout, &nelemout);
			if (rc == -1) {
				if (errno == E2BIG) {
					DMLOG_PRINT(DMLVL_DEBUG, "nelem = %d\n",
						    nelemout);
					if (nelemout == nelemin) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d and expected errno = %d\n",
							    szFuncName, rc,
							    errno);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d and expected errno = %d but unexpected nelemp (%d vs %d)\n",
							    szFuncName, rc,
							    errno, nelemout,
							    nelemin);
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
			/* SPECIAL CASE: need to set bMounted and response */
			dm_mount_event_t *me =
			    DM_GET_VALUE(dmMsg, ev_data, dm_mount_event_t *);
			void *lhanp = DM_GET_VALUE(me, me_handle1, void *);
			size_t lhlen = DM_GET_LEN(me, me_handle1);

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
			DMEV_SET(DM_EVENT_UNMOUNT, events);
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
			/* SPECIAL CASE: need to set response and bMounted */
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);

			if (nse->ne_retcode == 0) {
				bMounted = DM_FALSE;
			}

			response = DM_RESP_CONTINUE;
		} else if (type == DM_EVENT_USER) {
			DMLOG_PRINT(DMLVL_DEBUG, "Message is DM_EVENT_USER\n");

			DoTest();

			response = DM_RESP_CONTINUE;
		} else {
			DMLOG_PRINT(DMLVL_ERR, "Message is unexpected!\n");
			response = DM_RESP_ABORT;
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
