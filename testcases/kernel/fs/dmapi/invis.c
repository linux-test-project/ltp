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
 * TEST CASE	: invis.c
 *
 * VARIATIONS	: 32
 *
 * API'S TESTED	: dm_read_invis
 * 		  dm_write_invis
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
char DummySubdir[FILENAME_MAX];
char DummyTmp[FILENAME_MAX];
char DummyString[DUMMY_STRLEN];

/* Variables for thread communications */
dm_eventtype_t eventReceived;
void *hanp1;
size_t hlen1;
dm_off_t offset;
dm_size_t length;
int numRegions;
dm_region_t maxRegions[1], minRegions[1];

void *Thread(void *);

void LogStat(struct stat *statfs)
{

	DMLOG_PRINT(DMLVL_DEBUG, "  st_dev %d\n", statfs->st_dev);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_ino %d\n", statfs->st_ino);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_mode 0x%x\n", statfs->st_mode);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_nlink %d\n", statfs->st_nlink);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_uid %d\n", statfs->st_uid);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_gid %d\n", statfs->st_gid);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_rdev %d\n", statfs->st_rdev);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_size %lld\n", statfs->st_size);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_blksize %d\n", statfs->st_blksize);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_blocks %d\n", statfs->st_blocks);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_atime %d\n", statfs->st_atime);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_mtime %d\n", statfs->st_mtime);
	DMLOG_PRINT(DMLVL_DEBUG, "  st_ctime %d\n", statfs->st_ctime);

}

/*
 * StatCmp is used instead of memcmp because some of the pad fields or unused
 * fields may not be the same even though the pertinent info may be the same
 */
int StatCmp(struct stat *stat1, struct stat *stat2)
{

	if ((stat1->st_dev != stat2->st_dev) ||
	    (stat1->st_ino != stat2->st_ino) ||
	    (stat1->st_mode != stat2->st_mode) ||
	    (stat1->st_nlink != stat2->st_nlink) ||
	    (stat1->st_uid != stat2->st_uid) ||
	    (stat1->st_gid != stat2->st_gid) ||
	    (stat1->st_rdev != stat2->st_rdev) ||
	    (stat1->st_size != stat2->st_size) ||
	    (stat1->st_blksize != stat2->st_blksize) ||
	    (stat1->st_blocks != stat2->st_blocks) ||
	    (stat1->st_atime != stat2->st_atime) ||
	    (stat1->st_mtime != stat2->st_mtime) ||
	    (stat1->st_ctime != stat2->st_ctime)) {
		return -1;
	} else {
		return 0;
	}

}

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int i;
	int rc;
	char *szSessionInfo = "dm_test session info";
	dm_eventset_t events, maxFileEvents, minFileEvents;

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
		sprintf(DummyTmp, "%s/%s", mountPt, DUMMY_TMP);

		remove(DummyFile);
		rmdir(DummySubdir);

		memcpy(DummyString, DUMMY_STRING, DUMMY_STRLEN);

		EVENT_DELIVERY_DELAY;
		fd = open(DummyTmp, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
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
	}

	numRegions = 1;
	maxRegions[0].rg_offset = 0;
	maxRegions[0].rg_size = 0;
	maxRegions[0].rg_flags =
	    DM_REGION_READ | DM_REGION_WRITE | DM_REGION_TRUNCATE;
	minRegions[0].rg_offset = 0;
	minRegions[0].rg_size = 0;
	minRegions[0].rg_flags = DM_REGION_NOEVENT;

	DMEV_ZERO(maxFileEvents);
	DMEV_SET(DM_EVENT_PREUNMOUNT, maxFileEvents);
	DMEV_SET(DM_EVENT_UNMOUNT, maxFileEvents);
	DMEV_SET(DM_EVENT_CREATE, maxFileEvents);
	DMEV_SET(DM_EVENT_CLOSE, maxFileEvents);
	DMEV_SET(DM_EVENT_POSTCREATE, maxFileEvents);
	DMEV_SET(DM_EVENT_REMOVE, maxFileEvents);
	DMEV_SET(DM_EVENT_POSTREMOVE, maxFileEvents);
	DMEV_SET(DM_EVENT_RENAME, maxFileEvents);
	DMEV_SET(DM_EVENT_POSTRENAME, maxFileEvents);
	DMEV_SET(DM_EVENT_LINK, maxFileEvents);
	DMEV_SET(DM_EVENT_POSTLINK, maxFileEvents);
	DMEV_SET(DM_EVENT_SYMLINK, maxFileEvents);
	DMEV_SET(DM_EVENT_POSTSYMLINK, maxFileEvents);
	DMEV_SET(DM_EVENT_ATTRIBUTE, maxFileEvents);

	DMEV_ZERO(minFileEvents);
	DMEV_SET(DM_EVENT_PREUNMOUNT, minFileEvents);
	DMEV_SET(DM_EVENT_UNMOUNT, minFileEvents);

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI invisible read/write tests\n");

	szFuncName = "dm_read_invis";

	/*
	 * TEST    : dm_read_invis - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 1)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_read_invis(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, inoff, inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 2)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_read_invis(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, inoff, inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_read_invis(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   inoff, inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_read_invis(sid, hanp, hlen, INVALID_ADDR, inoff,
					   inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - invalid off
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #9 (0 returned instead of -1 and
	 * errno EINVAL)
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 5)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = TMP_FILELEN + 1;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid off)\n",
				    szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 6)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, (void *)INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - file start
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 7)) {
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file start)\n",
				    szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == inlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "read %d bytes\n", rc);
				if (memcmp(buf, DUMMY_STRING, DUMMY_STRLEN) ==
				    0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if ((rc2 == 0)
						    &&
						    (StatCmp(&statfs1, &statfs2)
						     == 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - file middle
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 8)) {
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t inoff = (TMP_FILELEN / 2) - ((TMP_FILELEN / 2) % 10);
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file middle)\n",
				    szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == inlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "read %d bytes\n", rc);
				if (memcmp(buf, DUMMY_STRING, DUMMY_STRLEN) ==
				    0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if ((rc2 == 0)
						    &&
						    (StatCmp(&statfs1, &statfs2)
						     == 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - file end
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 9)) {
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t inoff = TMP_FILELEN - DUMMY_STRLEN;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file end)\n", szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == inlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "read %d bytes\n", rc);
				if (memcmp(buf, DUMMY_STRING, DUMMY_STRLEN) ==
				    0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if ((rc2 == 0)
						    &&
						    (StatCmp(&statfs1, &statfs2)
						     == 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - overlaps file end
	 * EXPECTED: rc = DUMMY_STRLEN/2
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 10)) {
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t inoff = TMP_FILELEN - (DUMMY_STRLEN / 2);
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(overlaps file end)\n",
				    szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == DUMMY_STRLEN / 2) {
				DMLOG_PRINT(DMLVL_DEBUG, "read %d bytes\n", rc);
				if (memcmp
				    (buf, DummyString + (DUMMY_STRLEN / 2),
				     DUMMY_STRLEN / 2) == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if ((rc2 == 0)
						    &&
						    (StatCmp(&statfs1, &statfs2)
						     == 0)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
				DMVAR_FAIL();
			}

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 11)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_read_invis(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, inoff, inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_read_invis - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 12)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(directory handle)\n",
				    szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
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
	 * TEST    : dm_read_invis - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 13)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
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
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
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
	 * TEST    : dm_read_invis - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 14)) {
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_read_invis(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_NO_TOKEN, inoff, inlen, buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_read_invis - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(READ_INVIS_BASE + 15)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		} else if ((rc = remove(DummyFile)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_read_invis(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, buf);
			DMLOG_PRINT(DMLVL_DEBUG, "GOT %d, %s\n", rc, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_write_invis";

	/*
	 * TEST    : dm_write_invis - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 1)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_write_invis(INVALID_ADDR, hanp, hlen,
					    DM_NO_TOKEN, 0, outoff, outlen,
					    buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 2)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_write_invis(sid, (void *)INVALID_ADDR, hlen,
					    DM_NO_TOKEN, 0, outoff, outlen,
					    buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_write_invis(sid, hanp, INVALID_ADDR,
					    DM_NO_TOKEN, 0, outoff, outlen,
					    buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_write_invis(sid, hanp, hlen, INVALID_ADDR, 0,
					    outoff, outlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - invalid off+len
	 * EXPECTED: rc = -1, errno = EFBIG
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 5)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = MAXFILESIZE - 5;
		dm_size_t outlen = TMP_FILELEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid off+len)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFBIG);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - invalid bufp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 6)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen,
					    (void *)INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - file start, async
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 7)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDWR)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file start, async)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen, DUMMY_STRING2);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == outlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "wrote %d bytes\n",
					    rc);
				if ((rc2 == 0)
				    && (lseek(fd, outoff, SEEK_SET) == outoff)
				    && (read(fd, buf, DUMMY_STRLEN) == outlen)
				    && (memcmp(buf, DUMMY_STRING2, DUMMY_STRLEN)
					== 0)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if (StatCmp(&statfs1, &statfs2)
						    == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING2,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
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
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - file middle, async
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 8)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t outoff = (TMP_FILELEN / 2) - ((TMP_FILELEN / 2) % 10);
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDWR)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file middle, async)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen, DUMMY_STRING2);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == outlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "wrote %d bytes\n",
					    rc);
				if ((rc2 == 0)
				    && (lseek(fd, outoff, SEEK_SET) == outoff)
				    && (read(fd, buf, DUMMY_STRLEN) == outlen)
				    && (memcmp(buf, DUMMY_STRING2, DUMMY_STRLEN)
					== 0)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if (StatCmp(&statfs1, &statfs2)
						    == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING2,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
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
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - file end, async
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 9)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t outoff = TMP_FILELEN - DUMMY_STRLEN;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDWR)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file end, async)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen, DUMMY_STRING2);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == outlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "wrote %d bytes\n",
					    rc);
				if ((rc2 == 0)
				    && (lseek(fd, outoff, SEEK_SET) == outoff)
				    && (read(fd, buf, DUMMY_STRLEN) == outlen)
				    && (memcmp(buf, DUMMY_STRING2, DUMMY_STRLEN)
					== 0)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if (StatCmp(&statfs1, &statfs2)
						    == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING2,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
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
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - file start, sync
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 10)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDWR)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file start, sync)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN,
					    DM_WRITE_SYNC, outoff, outlen,
					    DUMMY_STRING2);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == outlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "wrote %d bytes\n",
					    rc);
				if ((rc2 == 0)
				    && (lseek(fd, outoff, SEEK_SET) == outoff)
				    && (read(fd, buf, DUMMY_STRLEN) == outlen)
				    && (memcmp(buf, DUMMY_STRING2, DUMMY_STRLEN)
					== 0)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if (StatCmp(&statfs1, &statfs2)
						    == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING2,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
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
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - file middle, sync
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 11)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t outoff = (TMP_FILELEN / 2) - ((TMP_FILELEN / 2) % 10);
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDWR)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file middle, sync)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN,
					    DM_WRITE_SYNC, outoff, outlen,
					    DUMMY_STRING2);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == outlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "wrote %d bytes\n",
					    rc);
				if ((rc2 == 0)
				    && (lseek(fd, outoff, SEEK_SET) == outoff)
				    && (read(fd, buf, DUMMY_STRLEN) == outlen)
				    && (memcmp(buf, DUMMY_STRING2, DUMMY_STRLEN)
					== 0)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if (StatCmp(&statfs1, &statfs2)
						    == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING2,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
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
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - file end, sync
	 * EXPECTED: rc = DUMMY_STRLEN
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 12)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_off_t outoff = TMP_FILELEN - DUMMY_STRLEN;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];
		struct stat statfs1, statfs2;
		int errnoSaved = 0;
		dm_boolean_t exactflag;
		int rc2;

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDWR)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_handle_to_fshandle(hanp, hlen, &fshanp,
					       &fshlen)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					   &maxFileEvents, DM_EVENT_MAX)) == -1)
			||
			((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, maxRegions,
					&exactflag)) == -1)
			|| ((rc = stat(DummyFile, &statfs1)) == -1)) {
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! %d\n", errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			TIMESTAMP_DELAY;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file end, sync)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN,
					    DM_WRITE_SYNC, outoff, outlen,
					    DUMMY_STRING2);
			errnoSaved = errno;
			EVENT_DELIVERY_DELAY;
			rc2 = stat(DummyFile, &statfs2);
			dm_set_region(sid, hanp, hlen, DM_NO_TOKEN, numRegions,
				      minRegions, &exactflag);
			dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN,
					 &minFileEvents, DM_EVENT_MAX);
			if (rc == outlen) {
				DMLOG_PRINT(DMLVL_DEBUG, "wrote %d bytes\n",
					    rc);
				if ((rc2 == 0)
				    && (lseek(fd, outoff, SEEK_SET) == outoff)
				    && (read(fd, buf, DUMMY_STRLEN) == outlen)
				    && (memcmp(buf, DUMMY_STRING2, DUMMY_STRLEN)
					== 0)) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "buffer contents [%.*s]\n",
						    rc, buf);
					if (eventReceived == DM_EVENT_INVALID) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "no event received\n");
						if (StatCmp(&statfs1, &statfs2)
						    == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info same\n");
							DMVAR_PASS();
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "stat info NOT same!\n");
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info before:\n");
							LogStat(&statfs1);
							DMLOG_PRINT(DMLVL_DEBUG,
								    "stat info after:\n");
							LogStat(&statfs2);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "event %d received!\n",
							    eventReceived);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "buffer contents NOT correct! (%.*s vs %.*s)\n",
						    DUMMY_STRLEN, DUMMY_STRING2,
						    rc, buf);
					DMVAR_FAIL();
				}
			} else {
				DMLOG_PRINT(DMLVL_ERR,
					    "%s failed with unexpected rc = %d (errno = %d)\n",
					    szFuncName, rc, errnoSaved);
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
			dm_handle_free(fshanp, fshlen);
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 13)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
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
			rc = dm_write_invis(DM_NO_SESSION, hanp, hlen,
					    DM_NO_TOKEN, 0, outoff, outlen,
					    buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_write_invis - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 14)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(directory handle)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen, buf);
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
	 * TEST    : dm_write_invis - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 15)) {
		void *hanp;
		size_t hlen;
		dm_off_t outoff = 0;
		dm_size_t outlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
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
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    outoff, outlen, buf);
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
	 * TEST    : dm_write_invis - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 16)) {
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_write_invis(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				    DM_NO_TOKEN, 0, inoff, inlen, buf);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_write_invis - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(WRITE_INVIS_BASE + 17)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = DUMMY_STRLEN;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummyFile, &hanp, &hlen)) ==
			   -1) {
			remove(DummyFile);
		} else if ((rc = remove(DummyFile)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_write_invis(sid, hanp, hlen, DM_NO_TOKEN, 0,
					    inoff, inlen, buf);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	remove(DummyTmp);

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
			DMEV_SET(DM_EVENT_CREATE, events);
			DMEV_SET(DM_EVENT_CLOSE, events);
			DMEV_SET(DM_EVENT_POSTCREATE, events);
			DMEV_SET(DM_EVENT_REMOVE, events);
			DMEV_SET(DM_EVENT_POSTREMOVE, events);
			DMEV_SET(DM_EVENT_RENAME, events);
			DMEV_SET(DM_EVENT_POSTRENAME, events);
			DMEV_SET(DM_EVENT_LINK, events);
			DMEV_SET(DM_EVENT_POSTLINK, events);
			DMEV_SET(DM_EVENT_SYMLINK, events);
			DMEV_SET(DM_EVENT_POSTSYMLINK, events);
			DMEV_SET(DM_EVENT_READ, events);
			DMEV_SET(DM_EVENT_WRITE, events);
			DMEV_SET(DM_EVENT_TRUNCATE, events);
			DMEV_SET(DM_EVENT_ATTRIBUTE, events);
			rc = dm_set_disp(sid, lhanp, lhlen, token, &events,
					 DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
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
		} else {
			eventReceived = type;

			switch (type) {
			case DM_EVENT_READ:
				{
					dm_data_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_data_event_t *);
					hanp1 =
					    DM_GET_VALUE(de, de_handle, void *);
					hlen1 = DM_GET_LEN(de, de_handle);
					offset = de->de_offset;
					length = de->de_length;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_READ\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Handle: %p\n", hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Offset: %d\n", offset);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Length: %d\n", length);

					response = DM_RESP_CONTINUE;
					break;
				}

			case DM_EVENT_WRITE:
				{
					dm_data_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_data_event_t *);
					hanp1 =
					    DM_GET_VALUE(de, de_handle, void *);
					hlen1 = DM_GET_LEN(de, de_handle);
					offset = de->de_offset;
					length = de->de_length;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_WRITE\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Handle: %p\n", hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Offset: %d\n", offset);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Length: %d\n", length);

					response = DM_RESP_CONTINUE;
					break;
				}

			case DM_EVENT_TRUNCATE:
				{
					dm_data_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_data_event_t *);
					hanp1 =
					    DM_GET_VALUE(de, de_handle, void *);
					hlen1 = DM_GET_LEN(de, de_handle);
					offset = de->de_offset;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_TRUNCATE\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Handle: %p\n", hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Offset: %d\n", offset);

					response = DM_RESP_CONTINUE;
					break;
				}

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
