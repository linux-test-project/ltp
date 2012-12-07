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
 * TEST CASE	: pmr_pre.c
 *
 * VARIATIONS	: 33
 *
 * API'S TESTED	: dm_set_region
 *
 * NOTES	: The last variation of this test case, when run before
 * 		  rebooting and pmr_post, verifies that persistent managed
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
#include <sys/xattr.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "dm_test.h"

#define ATTR_LISTLEN 1000
#define TMP_FILELEN 10000	/* must be > 2*PAGE_SIZE */

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char command[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummySubdir[FILENAME_MAX];
char *PMR_AttrName = PMR_ATTRNAME;

void *Thread(void *);

int main(int argc, char **argv)
{

	char *varstr;
	int i;
	int rc;
	char *szSessionInfo = "dm_test session info";
	char *szFuncName;
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
		int fd;

		sprintf(DummyFile, "%s/%s", mountPt, DUMMY_FILE);
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);

		remove(DummyFile);
		rmdir(DummySubdir);

		EVENT_DELIVERY_DELAY;
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

	DMLOG_PRINT(DMLVL_DEBUG,
		    "Starting DMAPI persistent managed regions test initialization\n");

	szFuncName = "dm_set_region";

	/*
	 * TEST    : dm_set_region - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_set_region(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, nelem, &regbuf,
					   &exactflag);
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
	 * TEST    : dm_set_region - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp\n",
				    szFuncName);
			rc = dm_set_region(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, nelem, &regbuf,
					   &exactflag);
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
	 * TEST    : dm_set_region - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   nelem, &regbuf, &exactflag);
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
	 * TEST    : dm_set_region - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, INVALID_ADDR, nelem,
					   &regbuf, &exactflag);
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
	 * TEST    : dm_set_region - invalid nelem
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_region_t regbuf;
		dm_boolean_t exactflag;
		dm_size_t retval;

		/* Variation set up */
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
			 dm_get_config(hanp, hlen,
				       DM_CONFIG_MAX_MANAGED_REGIONS,
				       &retval)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelem)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					   retval + 1, &regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

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
	 * TEST    : dm_set_region - invalid regbufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelem = 1;
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid regbufp)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   (dm_region_t *) INVALID_ADDR,
					   &exactflag);
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
	 * TEST    : dm_set_region - invalid exactflagp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;

		/* Variation set up */
		nelem = 1;
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
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid exactflagp)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   &regbuf,
					   (dm_boolean_t *) INVALID_ADDR);
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
	 * TEST    : dm_set_region - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 8)) {
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelem = 1;
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_set_region(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, nelem, &regbuf,
					   &exactflag);
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
	 * TEST    : dm_set_region - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 9)) {
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelem = 1;
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n",
				    szFuncName);
			rc = dm_set_region(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
					   DM_NO_TOKEN, nelem, &regbuf,
					   &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

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
	 * TEST    : dm_set_region - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 10)) {
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelem = 1;
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
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   &regbuf, &exactflag);
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
	 * TEST    : dm_set_region - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;

		/* Variation set up */
		nelem = 1;
		memset(&regbuf, 0, sizeof(regbuf));

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_fshandle(DummyFile, &hanp, &hlen))
			   == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   &regbuf, &exactflag);
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
	 * TEST    : dm_set_region - nelem 0
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_boolean_t exactflag;
		char value[ATTR_LISTLEN];

		/* Variation set up */
		nelem = 0;

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
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   NULL, &exactflag);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "exactflag = %d\n",
					    exactflag);
				if (((rc =
				      getxattr(DummyFile, PMR_AttrName, value,
					       sizeof(value))) == -1)
				    && (errno == ENODATA)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected getxattr(%s) rc (%d vs %d), errno %d\n",
						    szFuncName, 0, PMR_AttrName,
						    rc, -1, errno);
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
	 * TEST    : dm_set_region - nelem 1
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 13)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf;
		dm_boolean_t exactflag;
		char value[ATTR_LISTLEN];

		/* Variation set up */
		nelem = 1;
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
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(nelem 1)\n", szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   &regbuf, &exactflag);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "exactflag = %d\n",
					    exactflag);
				if ((rc =
				     getxattr(DummyFile, PMR_AttrName, value,
					      sizeof(value))) >=
				    sizeof(dm_region_t)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected getxattr(%s) rc (%d vs %d), errno %d\n",
						    szFuncName, 0, PMR_AttrName,
						    rc, sizeof(dm_region_t),
						    errno);
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
	 * TEST    : dm_set_region - nelem 2
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 14)) {
#ifdef MULTIPLE_REGIONS
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		char value[ATTR_LISTLEN];

		/* Variation set up */
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = 1000;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = 2000;
		regbuf[1].rg_size = 1000;
		regbuf[1].rg_flags = DM_REGION_WRITE;

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(nelem 2)\n", szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "exactflag = %d\n",
					    exactflag);
				if ((rc =
				     getxattr(DummyFile, PMR_AttrName, value,
					      sizeof(value))) >=
				    (2 * sizeof(dm_region_t))) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected getxattr(%s) rc (%d vs %d), errno %d\n",
						    szFuncName, 0, PMR_AttrName,
						    rc, 2 * sizeof(dm_region_t),
						    errno);
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
	 * TEST    : dm_set_region - clear
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 15)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		char value[ATTR_LISTLEN];

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = 1000;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = 2000;
		regbuf[1].rg_size = 1000;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = 1000;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					regbuf, &exactflag)) == -1)
			||
			((rc =
			  getxattr(DummyFile, PMR_AttrName, value,
				   sizeof(value))) <
			 (nelem * sizeof(dm_region_t)))) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
			rc = -1;	/* rc could be >= 0 from getxattr */
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(clear)\n", szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, 0,
					   NULL, &exactflag);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "exactflag = %d\n",
					    exactflag);
				if (((rc =
				      getxattr(DummyFile, PMR_AttrName, value,
					       sizeof(value))) == -1)
				    && (errno == ENODATA)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected getxattr(%s) rc (%d vs %d), errno %d\n",
						    szFuncName, 0, PMR_AttrName,
						    rc, -1, errno);
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
	 * TEST    : dm_set_region - replace
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 16)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		char value[ATTR_LISTLEN];
		ssize_t xattrlen;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = 1000;
		regbuf[0].rg_flags = DM_REGION_WRITE;
		regbuf[1].rg_offset = 2000;
		regbuf[1].rg_size = 1000;
		regbuf[1].rg_flags = DM_REGION_READ;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = 1000;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					regbuf, &exactflag)) == -1)
			||
			((rc =
			  getxattr(DummyFile, PMR_AttrName, value,
				   sizeof(value))) <
			 (nelem * sizeof(dm_region_t)))) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
			rc = -1;	/* rc could be >= 0 from getxattr */
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			xattrlen = rc;

			nelem = 1;
			regbuf[0].rg_offset = 0;
			regbuf[0].rg_size = 1000;
			regbuf[0].rg_flags = DM_REGION_READ;

			DMLOG_PRINT(DMLVL_DEBUG, "%s(replace)\n", szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "exactflag = %d\n",
					    exactflag);
#ifdef MULTIPLE_REGIONS
				if ((rc =
				     getxattr(DummyFile, PMR_AttrName, value,
					      sizeof(value))) < xattrlen) {
#else
				if ((rc =
				     getxattr(DummyFile, PMR_AttrName, value,
					      sizeof(value))) == xattrlen) {
#endif
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected getxattr(%s) rc (%d vs %d), errno %d\n",
						    szFuncName, 0, PMR_AttrName,
						    rc, xattrlen, errno);
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
	 * TEST    : dm_set_region - private read mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 17)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private read mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private write mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 18)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = PAGE_SIZE;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_PRIVATE, fd,
			      PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private write mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private exec mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 19)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = PAGE_SIZE;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_PRIVATE, fd,
			      PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private exec mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private read/write mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 20)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private r/w mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared read mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 21)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared read mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared write mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 22)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = PAGE_SIZE;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd,
			      PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared write mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared exec mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 23)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = PAGE_SIZE;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_SHARED, fd,
			      PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared exec mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared read/write mmap overlapping region
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 24)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_SHARED, fd, 0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared r/w mmap overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private read mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 25)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = PAGE_SIZE;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd,
			      PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private read mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private write mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 26)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private write mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private exec mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 27)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private exec mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - private r/w mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 28)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE, fd, 2 * PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private r/w mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared read mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 29)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = PAGE_SIZE;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_WRITE;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd,
			      PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared read mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared write mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 30)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared write mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared exec mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 31)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared exec mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - shared r/w mmap not overlapping region
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 32)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[2];
		dm_boolean_t exactflag;
		void *memmap;

		/* Variation set up */
#ifdef MULTIPLE_REGIONS
		nelem = 2;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
		regbuf[1].rg_offset = PAGE_SIZE;
		regbuf[1].rg_size = PAGE_SIZE / 2;
		regbuf[1].rg_flags = DM_REGION_WRITE;
#else
		nelem = 1;
		regbuf[0].rg_offset = 0;
		regbuf[0].rg_size = PAGE_SIZE / 2;
		regbuf[0].rg_flags = DM_REGION_READ;
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
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_SHARED, fd, 2 * PAGE_SIZE)) == MAP_FAILED) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1 || fd == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared r/w mmap not overlap region)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
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
	 * TEST    : dm_set_region - persistent, Part I
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_REGION_BASE + 33)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;
		dm_region_t regbuf[PMR_NUM_REGIONS];
		dm_boolean_t exactflag;
		char value[ATTR_LISTLEN];

		/* Variation set up */
		nelem = PMR_NUM_REGIONS;
		memcpy(regbuf, dm_PMR_regbuf, nelem * sizeof(dm_region_t));
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(persistent, Part I)\n",
				    szFuncName);
			rc = dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, nelem,
					   regbuf, &exactflag);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "exactflag = %d\n",
					    exactflag);
				if ((rc =
				     getxattr(DummyFile, PMR_AttrName, value,
					      sizeof(value))) >=
				    (PMR_NUM_REGIONS * sizeof(dm_region_t))) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "%s passed with expected rc = 0\n",
						    szFuncName);
					DMVAR_PASS();

					printf
					    ("********************************************************\n");
					printf
					    ("* PLEASE REBOOT AND RUN pmr_post TO COMPLETE VARIATION *\n");
					printf
					    ("********************************************************\n");

				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc = %d but unexpected getxattr(%s) rc (%d vs %d), errno %d\n",
						    szFuncName, 0, PMR_AttrName,
						    rc,
						    PMR_NUM_REGIONS *
						    sizeof(dm_region_t), errno);
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
			/* DO NOT REMOVE DummyFile, IT IS NEEDED BY pmr_post */
			/*rc |= remove(DummyFile); */
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
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

	remove(DUMMY_FILE);

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
			/* SPECIAL CASE: need to set bMounted and response */
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);

			if (nse->ne_retcode == 0) {
				bMounted = DM_FALSE;
			}

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
