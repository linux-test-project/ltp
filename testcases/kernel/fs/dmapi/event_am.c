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
 * TEST CASE	: event_am.c
 *
 * VARIATIONS	: 31
 *
 * EVENTS TESTED: DM_EVENT_ATTRIBUTE
 * 		  DM_EVENT_CLOSE
 * 		  DM_EVENT_DESTROY
 *
 * API'S TESTED	: dm_set_return_on_destroy
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

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummySubdir[FILENAME_MAX];

/* Variables for thread communications */
dm_eventtype_t eventExpected;
dm_eventtype_t eventReceived;
dm_response_t eventResponse;
void *hanp1, *hanp2, *ahanp1;
size_t hlen1, hlen2, ahlen1;
char name1[FILENAME_MAX];

void *Thread(void *);

int main(int argc, char **argv)
{

	char *varstr;
	int rc;
	int varStatus;
	char *szSessionInfo = "dm_test session info";
	char *szFuncName;
	dm_eventset_t events;
	dm_size_t maxAttrSize;
	void *fshanp;
	size_t fshlen;
	dm_eventset_t AMevents;

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

	DMLOG_PRINT(DMLVL_DEBUG,
		    "Starting DMAPI asynchronous metadata event tests\n");

	/*
	 *  First batch of tests will be with events enabled on file system,
	 *  so set up events on fs accordingly
	 */
	rc = dm_path_to_fshandle(mountPt, &fshanp, &fshlen);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_path_to_handle(fs) failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_PREUNMOUNT, events);
	DMEV_SET(DM_EVENT_UNMOUNT, events);
	DMEV_SET(DM_EVENT_POSTCREATE, events);
	DMEV_SET(DM_EVENT_ATTRIBUTE, events);
	DMEV_SET(DM_EVENT_CLOSE, events);
	rc = dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist(fs) failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	/*
	 * TEST    : chmod - enabled on fs
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 1)) {
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		rc = mkdir(DummySubdir, DUMMY_DIR_RO_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s)\n", DummySubdir);
			rc = chmod(DummySubdir, O_RDWR);
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummySubdir, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (!(statfs.st_mode & O_RDWR)) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object mode NOT correct! (%x vs %x)\n",
						    O_RDWR, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chown - enabled on fs
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 2)) {
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		rc = mkdir(DummySubdir, DUMMY_DIR_RO_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s)\n", DummySubdir);
			rc = chown(DummySubdir, DUMMY_UID, DUMMY_GID);
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummySubdir, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_uid != DUMMY_UID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object uid NOT correct! (%x vs %x)\n",
						    DUMMY_UID, statfs.st_uid);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_gid != DUMMY_GID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object gid NOT correct! (%x vs %x)\n",
						    DUMMY_GID, statfs.st_gid);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : close - enabled on fs
	 * EXPECTED: DM_EVENT_CLOSE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 3)) {
		int fd1, fd2;

		/* Variation set up */
		eventExpected = DM_EVENT_CLOSE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((fd1 = open(DummySubdir, O_DIRECTORY)) == -1) {
			rmdir(DummySubdir);
		} else if ((fd2 = open(DummySubdir, O_DIRECTORY)) == -1) {
			close(fd1);
			rmdir(DummySubdir);
		}
		if (fd1 == -1 || fd2 == -1 || fd1 == fd2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1\n", fd1);
			rc = close(fd1);
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1 returned %d\n",
				    fd1, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				eventExpected = DM_EVENT_CLOSE;
				EVENT_DELIVERY_DELAY;
				DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #2\n", fd2);
				rc = close(fd2);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "close(%d) #2 returned %d\n", fd2,
					    rc);
				EVENT_DELIVERY_DELAY;
				varStatus =
				    DMVAR_CHKPASSEXP(0, rc, eventExpected,
						     eventReceived);
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chmod - enabled on fs
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 1)) {
		int fd;
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = close(fd)) == -1) {
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s)\n", DummyFile);
			rc = chmod(DummyFile, O_RDWR);
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummyFile, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (!(statfs.st_mode & O_RDWR)) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object mode NOT correct! (%x vs %x)\n",
						    O_RDWR, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chown - enabled on fs
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 2)) {
		int fd;
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = close(fd)) == -1) {
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s)\n", DummyFile);
			rc = chown(DummyFile, DUMMY_UID, DUMMY_GID);
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummyFile, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_uid != DUMMY_UID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object uid NOT correct! (%x vs %x)\n",
						    DUMMY_UID, statfs.st_uid);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_gid != DUMMY_GID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object gid NOT correct! (%x vs %x)\n",
						    DUMMY_GID, statfs.st_gid);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : close - enabled on fs
	 * EXPECTED: DM_EVENT_CLOSE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 3)) {
		int fd1, fd2;

		/* Variation set up */
		eventExpected = DM_EVENT_CLOSE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd1 =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((fd2 = open(DummyFile, O_RDONLY)) == -1) {
			close(fd1);
			remove(DummyFile);
		}
		if (fd1 == -1 || fd2 == -1 || fd1 == fd2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1\n", fd1);
			rc = close(fd1);
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1 returned %d\n",
				    fd1, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				eventExpected = DM_EVENT_CLOSE;
				EVENT_DELIVERY_DELAY;
				DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #2\n", fd2);
				rc = close(fd2);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "close(%d) #2 returned %d\n", fd2,
					    rc);
				EVENT_DELIVERY_DELAY;
				varStatus =
				    DMVAR_CHKPASSEXP(0, rc, eventExpected,
						     eventReceived);
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/* Wait for all pending messages to be handled */
	EVENT_DELIVERY_DELAY_LOOP;

	/*
	 *  Next batch of tests will be with events enabled on object, so
	 *  clear events on fs
	 */
	rc = dm_path_to_fshandle(mountPt, &fshanp, &fshlen);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_path_to_handle failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_PREUNMOUNT, events);
	DMEV_SET(DM_EVENT_UNMOUNT, events);
	DMEV_SET(DM_EVENT_POSTCREATE, events);
	rc = dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist(fs) failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	DMEV_ZERO(AMevents);
	DMEV_SET(DM_EVENT_ATTRIBUTE, AMevents);
	DMEV_SET(DM_EVENT_CLOSE, AMevents);

	/*
	 * TEST    : chmod - enabled on directory
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 4)) {
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummySubdir, &hanp, &hlen))
			   == -1) {
			rmdir(DummySubdir);
		} else
		    if ((rc =
			 dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					  &AMevents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DummySubdir);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s)\n", DummySubdir);
			rc = chmod(DummySubdir, O_RDWR);
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummySubdir, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (!(statfs.st_mode & O_RDWR)) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object mode NOT correct! (%x vs %x)\n",
						    O_RDWR, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
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
	 * TEST    : chown - enabled on directory
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 5)) {
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummySubdir, &hanp, &hlen))
			   == -1) {
			rmdir(DummySubdir);
		} else
		    if ((rc =
			 dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					  &AMevents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			rmdir(DummySubdir);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s)\n", DummySubdir);
			rc = chown(DummySubdir, DUMMY_UID, DUMMY_GID);
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummySubdir, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_uid != DUMMY_UID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object uid NOT correct! (%x vs %x)\n",
						    DUMMY_UID, statfs.st_uid);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_gid != DUMMY_GID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object gid NOT correct! (%x vs %x)\n",
						    DUMMY_GID, statfs.st_gid);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
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
	 * TEST    : close - enabled on directory
	 * EXPECTED: DM_EVENT_CLOSE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 6)) {
		int fd1, fd2;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		eventExpected = DM_EVENT_CLOSE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((fd1 = open(DummySubdir, O_DIRECTORY)) == -1) {
			rmdir(DummySubdir);
		} else if ((fd2 = open(DummySubdir, O_DIRECTORY)) == -1) {
			close(fd1);
			rmdir(DummySubdir);
		} else if ((rc = dm_fd_to_handle(fd2, &hanp, &hlen)) == -1) {
			close(fd2);
			close(fd1);
			rmdir(DummySubdir);
		} else
		    if ((rc =
			 dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					  &AMevents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd2);
			close(fd1);
			rmdir(DummySubdir);
		}
		if (fd1 == -1 || fd2 == -1 || fd1 == fd2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1\n", fd1);
			rc = close(fd1);
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1 returned %d\n",
				    fd1, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				eventExpected = DM_EVENT_CLOSE;
				EVENT_DELIVERY_DELAY;
				DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #2\n", fd2);
				rc = close(fd2);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "close(%d) #2 returned %d\n", fd2,
					    rc);
				EVENT_DELIVERY_DELAY;
				varStatus =
				    DMVAR_CHKPASSEXP(0, rc, eventExpected,
						     eventReceived);
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
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
	 * TEST    : chmod - enabled on file
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else if (((rc = close(fd)) == -1) ||
			   ((rc =
			     dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					      &AMevents,
					      DM_EVENT_MAX)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s)\n", DummyFile);
			rc = chmod(DummyFile, O_RDWR);
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummyFile, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (!(statfs.st_mode & O_RDWR)) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object mode NOT correct! (%x vs %x)\n",
						    O_RDWR, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chown - enabled on file
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_ino_t ino;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else if (((rc = close(fd)) == -1) ||
			   ((rc =
			     dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					      &AMevents,
					      DM_EVENT_MAX)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s)\n", DummyFile);
			rc = chown(DummyFile, DUMMY_UID, DUMMY_GID);
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				rc |= stat(DummyFile, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object/entry handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object handle NOT correct! (%lld vs %d)\n",
						    ino, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_uid != DUMMY_UID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object uid NOT correct! (%x vs %x)\n",
						    DUMMY_UID, statfs.st_uid);
					varStatus = DMSTAT_FAIL;
				} else if (statfs.st_gid != DUMMY_GID) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Object gid NOT correct! (%x vs %x)\n",
						    DUMMY_GID, statfs.st_gid);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : close - enabled on file
	 * EXPECTED: DM_EVENT_CLOSE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 6)) {
		int fd1, fd2;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		eventExpected = DM_EVENT_CLOSE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd1 =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((fd2 = open(DummyFile, O_RDONLY)) == -1) {
			close(fd1);
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd2, &hanp, &hlen)) == -1) {
			close(fd2);
			close(fd1);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_eventlist(sid, hanp, hlen, DM_NO_TOKEN,
					  &AMevents, DM_EVENT_MAX)) == -1) {
			dm_handle_free(hanp, hlen);
			close(fd2);
			close(fd1);
			remove(DummyFile);
		}
		if (fd1 == -1 || fd2 == -1 || fd1 == fd2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1\n", fd1);
			rc = close(fd1);
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1 returned %d\n",
				    fd1, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				eventExpected = DM_EVENT_CLOSE;
				EVENT_DELIVERY_DELAY;
				DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #2\n", fd2);
				rc = close(fd2);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "close(%d) #2 returned %d\n", fd2,
					    rc);
				EVENT_DELIVERY_DELAY;
				varStatus =
				    DMVAR_CHKPASSEXP(0, rc, eventExpected,
						     eventReceived);
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/* Wait for all pending messages to be handled */
	EVENT_DELIVERY_DELAY_LOOP;

	/*
	 *  Last batch of tests will be with events disabled
	 */

	/*
	 * TEST    : chmod - disabled
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 7)) {

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		rc = mkdir(DummySubdir, DUMMY_DIR_RO_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s)\n", DummySubdir);
			rc = chmod(DummySubdir, O_RDWR);
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chown - disabled
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 8)) {

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		rc = mkdir(DummySubdir, DUMMY_DIR_RO_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s)\n", DummySubdir);
			rc = chown(DummySubdir, DUMMY_UID, DUMMY_GID);
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : close - disabled
	 * EXPECTED: DM_EVENT_CLOSE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_META_EVENT_BASE + 9)) {
		int fd1, fd2;

		/* Variation set up */
		eventExpected = DM_EVENT_CLOSE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((fd1 = open(DummySubdir, O_DIRECTORY)) == -1) {
			rmdir(DummySubdir);
		} else if ((fd2 = open(DummySubdir, O_DIRECTORY)) == -1) {
			close(fd1);
			rmdir(DummySubdir);
		}
		if (fd1 == -1 || fd2 == -1 || fd1 == fd2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1\n", fd1);
			rc = close(fd1);
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1 returned %d\n",
				    fd1, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				eventExpected = DM_EVENT_INVALID;
				eventReceived = DM_EVENT_INVALID;
				EVENT_DELIVERY_DELAY;
				DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #2\n", fd2);
				rc = close(fd2);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "close(%d) #2 returned %d\n", fd2,
					    rc);
				EVENT_DELIVERY_DELAY;
				varStatus =
				    DMVAR_CHKPASSEXP(0, rc, eventExpected,
						     eventReceived);
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chmod - disabled
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 7)) {
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = close(fd)) == -1) {
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s)\n", DummyFile);
			rc = chmod(DummyFile, O_RDWR);
			DMLOG_PRINT(DMLVL_DEBUG, "chmod(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : chown - disabled
	 * EXPECTED: DM_EVENT_ATTRIBUTE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 8)) {
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_ATTRIBUTE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = close(fd)) == -1) {
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s)\n", DummyFile);
			rc = chown(DummyFile, DUMMY_UID, DUMMY_GID);
			DMLOG_PRINT(DMLVL_DEBUG, "chown(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : close - disabled
	 * EXPECTED: DM_EVENT_CLOSE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_META_EVENT_BASE + 9)) {
		int fd1, fd2;

		/* Variation set up */
		eventExpected = DM_EVENT_CLOSE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		if ((fd1 =
		     open(DummyFile, O_RDONLY | O_CREAT,
			  DUMMY_FILE_RO_MODE)) == -1) {
			/* No clean up */
		} else if ((fd2 = open(DummyFile, O_RDONLY)) == -1) {
			close(fd1);
			remove(DummyFile);
		}
		if (fd1 == -1 || fd2 == -1 || fd1 == fd2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1\n", fd1);
			rc = close(fd1);
			DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #1 returned %d\n",
				    fd1, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				eventExpected = DM_EVENT_INVALID;
				eventReceived = DM_EVENT_INVALID;
				EVENT_DELIVERY_DELAY;
				DMLOG_PRINT(DMLVL_DEBUG, "close(%d) #2\n", fd2);
				rc = close(fd2);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "close(%d) #2 returned %d\n", fd2,
					    rc);
				EVENT_DELIVERY_DELAY;
				varStatus =
				    DMVAR_CHKPASSEXP(0, rc, eventExpected,
						     eventReceived);
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/* From here on out we're only interested in the destroy event */
	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_PREUNMOUNT, events);
	DMEV_SET(DM_EVENT_UNMOUNT, events);
	DMEV_SET(DM_EVENT_DESTROY, events);
	rc = dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	rc = dm_get_config(fshanp, fshlen, DM_CONFIG_MAX_ATTR_ON_DESTROY,
			   &maxAttrSize);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_get_config failed! (rc = %d, errno = %d)\n", rc,
			    errno);
		dm_destroy_session(sid);
		DM_EXIT();
	} else {
		DMLOG_PRINT(DMLVL_DEBUG, "DM_CONFIG_MAX_ATTR_ON_DESTROY %d\n",
			    maxAttrSize);
	}

	szFuncName = "dm_set_return_on_destroy";

	/*
	 * TEST    : dm_set_return_on_destroy - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 1)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid sid)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(INVALID_ADDR, fshanp,
						      fshlen, DM_NO_TOKEN,
						      &attrname, DM_TRUE);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 2)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, (void *)INVALID_ADDR,
						      fshlen, DM_NO_TOKEN,
						      &attrname, DM_TRUE);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 3)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, fshanp, INVALID_ADDR,
						      DM_NO_TOKEN, &attrname,
						      DM_TRUE);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 4)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid token)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, fshanp, fshlen,
						      INVALID_ADDR, &attrname,
						      DM_TRUE);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - invalid attrnamep
	 * EXPECTED: rc = -1, errno = EFAULT
	 *
	 * This variation uncovered XFS BUG #14 (non-0 return code from
	 * copy_from_user returned)
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 5)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid attrnamep)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, fshanp, fshlen,
						      DM_NO_TOKEN,
						      (dm_attrname_t *)
						      INVALID_ADDR, DM_TRUE);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 6)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(DM_NO_SESSION sid)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(DM_NO_SESSION, fshanp,
						      fshlen, DM_NO_TOKEN,
						      &attrname, DM_TRUE);
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
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - different sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 7)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];
		dm_sessid_t newsid;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		} else
		    if ((rc =
			 dm_create_session(DM_NO_SESSION, szSessionInfo,
					   &newsid)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
			dm_handle_free(fshanp, fshlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(different sid)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(newsid, fshanp, fshlen,
						      DM_NO_TOKEN, &attrname,
						      DM_FALSE);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			rc |= dm_destroy_session(newsid);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - file handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, hanp, hlen,
						      DM_NO_TOKEN, &attrname,
						      DM_TRUE);
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
	 * TEST    : dm_set_return_on_destroy - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 9)) {
		void *hanp;
		size_t hlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DummySubdir, &hanp, &hlen))
			   == -1) {
			rmdir(DummySubdir);
		} else
		    if ((rc =
			 dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
				       0, sizeof(buf), buf)) == -1) {
			rmdir(DummySubdir);
			dm_handle_free(hanp, hlen);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, hanp, hlen,
						      DM_NO_TOKEN, &attrname,
						      DM_TRUE);
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
	 * TEST    : dm_set_return_on_destroy - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 10)) {
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_set_return_on_destroy(sid, DM_GLOBAL_HANP,
					      DM_GLOBAL_HLEN, DM_NO_TOKEN,
					      &attrname, DM_TRUE);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_set_return_on_destroy - valid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 11)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char buf[ATTR_VALUELEN];

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		memcpy(buf, ATTR_VALUE, ATTR_VALUELEN);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, sizeof(buf), buf)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(valid)\n", szFuncName);
			rc = dm_set_return_on_destroy(sid, fshanp, fshlen,
						      DM_NO_TOKEN, &attrname,
						      DM_TRUE);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - zero length attribute
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 12)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN, &attrname,
					0, 0, NULL)) == -1)
			||
			((rc =
			  dm_path_to_fshandle(DummyFile, &fshanp,
					      &fshlen)) == -1)) {
			close(fd);
			remove(DummyFile);
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(zero len attr)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, fshanp, fshlen,
						      DM_NO_TOKEN, &attrname,
						      DM_TRUE);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
			dm_handle_free(fshanp, fshlen);
		}
	}

	/*
	 * TEST    : dm_set_return_on_destroy - attribute too big
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(SET_RETURN_ON_DESTROY_BASE + 13)) {
		int fd;
		void *hanp, *fshanp;
		size_t hlen, fshlen;
		dm_attrname_t attrname;
		char *buf;

		/* Variation set up */
		memset(attrname.an_chars, 0, DM_ATTR_NAME_SIZE);
		memcpy(attrname.an_chars, ATTR_NAME, DM_ATTR_NAME_SIZE);
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else if ((buf = malloc(maxAttrSize + 1)) == NULL) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		} else if ((memset(buf, '4', maxAttrSize + 1) == NULL) ||
			   ((rc =
			     dm_set_dmattr(sid, hanp, hlen, DM_NO_TOKEN,
					   &attrname, 0, maxAttrSize + 1,
					   buf)) == -1)
			   ||
			   ((rc =
			     dm_path_to_fshandle(DummyFile, &fshanp,
						 &fshlen)) == -1)) {
			free(buf);
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DummyFile);
		}
		if (fd == -1 || rc == -1 || buf == NULL) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(attr too big)\n",
				    szFuncName);
			rc = dm_set_return_on_destroy(sid, fshanp, fshlen,
						      DM_NO_TOKEN, &attrname,
						      DM_TRUE);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			free(buf);
			dm_handle_free(hanp, hlen);
			dm_handle_free(fshanp, fshlen);
		}
	}

	dm_handle_free(fshanp, fshlen);

	/* Wait for all pending messages to be handled */
	EVENT_DELIVERY_DELAY_LOOP;

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

			/*rc = dm_request_right(sid, lhanp, lhlen, token, DM_RR_WAIT, DM_RIGHT_EXCL);
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
			rc = dm_set_disp(sid, lhanp, lhlen, DM_NO_TOKEN,
					 &events, DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			/*rc = dm_release_right(sid, lhanp, lhlen, token);
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
			case DM_EVENT_ATTRIBUTE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_ATTRIBUTE\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Object handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Object handle length: %d\n",
						    ahlen1);

					response = DM_RESP_INVALID;
					break;
				}

			case DM_EVENT_CLOSE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_CLOSE\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Object handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Object handle length: %d\n",
						    ahlen1);

					response = DM_RESP_INVALID;
					break;
				}

			case DM_EVENT_DESTROY:
				{
					dm_destroy_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_destroy_event_t *);
					ahanp1 =
					    DM_GET_VALUE(de, ds_handle, void *);
					ahlen1 = DM_GET_LEN(de, ds_handle);

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_DESTROY\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Object handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Object handle length: %d\n",
						    ahlen1);
					if (de->ds_attrname.an_chars[0]) {
						int attrlen =
						    DM_GET_LEN(de, ds_attrcopy);

						DMLOG_PRINT(DMLVL_DEBUG,
							    "  Attribute name: %.*s\n",
							    8,
							    de->ds_attrname.
							    an_chars);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "  Attribute value length: %d\n",
							    attrlen);
						if (attrlen) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "  Attribute value: %s\n",
								    DM_GET_VALUE
								    (de,
								     ds_attrcopy,
								     char *));
						}
					}

					response = DM_RESP_INVALID;
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
