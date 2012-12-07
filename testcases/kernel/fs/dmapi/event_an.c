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
 * TEST CASE	: event_an.c
 *
 * VARIATIONS	: 34
 *
 * EVENTS TESTED: DM_EVENT_POSTCREATE
 * 		  DM_EVENT_POSTREMOVE
 * 		  DM_EVENT_POSTRENAME
 * 		  DM_EVENT_POSTSYMLINK
 * 		  DM_EVENT_POSTLINK
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
char command[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummyFile2[FILENAME_MAX];
char DummySubdir[FILENAME_MAX];
char DummySubdir2[FILENAME_MAX];
char DummyLink[FILENAME_MAX];
char DummySubdir2File[FILENAME_MAX];
char DummySubdir2Subdir[FILENAME_MAX];

/* Variables for thread communications */
dm_eventtype_t eventExpected;
dm_eventtype_t eventReceived;
dm_response_t eventResponse;
void *hanp1, *hanp2, *ahanp1, *ahanp2;
size_t hlen1, hlen2, ahlen1, ahlen2;
char name1[FILENAME_MAX], name2[FILENAME_MAX], aname1[FILENAME_MAX],
    aname2[FILENAME_MAX];
dm_mode_t mode, amode;
int aretcode;

void *Thread(void *);

int main(int argc, char **argv)
{

	char *varstr;
	int rc;
	int varStatus;
	char *szSessionInfo = "dm_test session info";
	dm_eventset_t events;
	void *fshanp, *dhanp;
	size_t fshlen, dhlen;

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
		sprintf(DummyFile2, "%s/%s", mountPt, DUMMY_FILE2);
		sprintf(DummySubdir, "%s/%s", mountPt, DUMMY_SUBDIR);
		sprintf(DummySubdir2, "%s/%s", mountPt, DUMMY_SUBDIR2);
		sprintf(DummyLink, "%s/%s", mountPt, DUMMY_LINK);
		sprintf(DummySubdir2File, "%s/%s", mountPt, DUMMY_SUBDIR2_FILE);
		sprintf(DummySubdir2Subdir, "%s/%s", mountPt,
			DUMMY_SUBDIR2_SUBDIR);

		remove(DummySubdir2File);
		rmdir(DummySubdir2Subdir);
		remove(DummyFile);
		remove(DummyFile2);
		unlink(DummyLink);
		rmdir(DummySubdir);
		rmdir(DummySubdir2);
	}

	DMLOG_PRINT(DMLVL_DEBUG,
		    "Starting DMAPI asynchronous namespace event tests\n");

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
	DMEV_SET(DM_EVENT_CREATE, events);
	DMEV_SET(DM_EVENT_POSTCREATE, events);
	DMEV_SET(DM_EVENT_REMOVE, events);
	DMEV_SET(DM_EVENT_POSTREMOVE, events);
	DMEV_SET(DM_EVENT_RENAME, events);
	DMEV_SET(DM_EVENT_POSTRENAME, events);
	DMEV_SET(DM_EVENT_SYMLINK, events);
	DMEV_SET(DM_EVENT_POSTSYMLINK, events);
	DMEV_SET(DM_EVENT_LINK, events);
	DMEV_SET(DM_EVENT_POSTLINK, events);
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
	 * TEST    : mkdir - enabled on fs
	 * EXPECTED: DM_EVENT_POSTCREATE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 1)) {
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTCREATE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "mkdir(%s)\n", DummySubdir);
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		DMLOG_PRINT(DMLVL_DEBUG, "mkdir(%s) returned %d\n", DummySubdir,
			    rc);
		EVENT_DELIVERY_DELAY;
		if ((varStatus =
		     DMVAR_CHKPASSEXP(0, rc, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			struct stat statfs;

			rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
			rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
			rc |= stat(DummySubdir, &statfs);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Unable to obtain inode!\n");
				varStatus = DMSTAT_FAIL;
			} else if (ino1 != ROOT_INODE) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handle NOT root! (%lld vs %d)\n",
					    ino1, ROOT_INODE);
				varStatus = DMSTAT_FAIL;
			} else if (ino2 != statfs.st_ino) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry handle NOT correct! (%lld vs %d)\n",
					    ino2, statfs.st_ino);
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry name NOT correct! (%s vs %s)\n",
					    aname1, DUMMY_SUBDIR);
				varStatus = DMSTAT_FAIL;
			} else if (dm_handle_cmp(hanp1, hlen1, ahanp1, ahlen1)
				   != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handles NOT same!\n");
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(name1, aname1) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry names NOT same! (%s vs %s)\n",
					    name1, aname1);
				varStatus = DMSTAT_FAIL;
			} else if (amode != statfs.st_mode) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Modes NOT same! (%d vs %d)\n",
					    amode, statfs.st_mode);
				varStatus = DMSTAT_FAIL;
			} else if (aretcode != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Return codes NOT same! (%d vs %d)\n",
					    mode, amode);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
		EVENT_DELIVERY_DELAY;
		rc = rmdir(DummySubdir);
		EVENT_DELIVERY_DELAY;
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : rmdir - enabled on fs
	 * EXPECTED: DM_EVENT_POSTREMOVE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 2)) {
		dm_ino_t ino;
		struct stat statfs;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTREMOVE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = stat(DummySubdir, &statfs)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "rmdir(%s)\n", DummySubdir);
			rc = rmdir(DummySubdir);
			DMLOG_PRINT(DMLVL_DEBUG, "rmdir(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (amode != statfs.st_mode) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Modes NOT same! (%d vs %d)\n",
						    amode, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : mv - enabled on fs
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 3)) {
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummySubdir, DummySubdir2);
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummySubdir, DummySubdir2);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n",
				    DummySubdir, DummySubdir2, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT root! (%lld vs %d)\n",
						    ino2, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(ahanp1, ahlen1, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT equal to new parent handle!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_SUBDIR2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_SUBDIR2);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : symlink - enabled on fs
	 * EXPECTED: DM_EVENT_POSTSYMLINK
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 4)) {
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTSYMLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "symlink(%s, %s)\n",
				    DummySubdir, DummySubdir2);
			rc = symlink(DummySubdir, DummySubdir2);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "symlink(%s, %s) returned %d\n",
				    DummySubdir, DummySubdir2, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				rc |= lstat(DummySubdir2, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry handle NOT correct! (%lld vs %d)\n",
						    ino2, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_SUBDIR2);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DummySubdir) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT correct! (%s vs %s)\n",
						    aname2, DummySubdir);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = unlink(DummySubdir2);
			rc |= rmdir(DummySubdir);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : link - enabled on fs
	 * EXPECTED: DM_EVENT_POSTLINK
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 5)) {
#ifdef DIRECTORY_LINKS
		dm_ino_t ino, ino1, ino2;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s)\n", DummySubdir,
				    DummyLink);
			rc = link(DummySubdir, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s) returned %d\n",
				    DummySubdir, DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(hanp1, hlen1, &ino1);
				rc |= dm_handle_to_ino(hanp2, hlen2, &ino2);
				rc |= dm_handle_to_ino(hanp, hlen, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%d vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Source link handle NOT correct! (%d vs %d)\n",
						    ino2, ino);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_LINK) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Target entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_LINK);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			rc |= unlink(DummyLink);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with DIRECTORY_LINKS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : open - enabled on fs
	 * EXPECTED: DM_EVENT_POSTCREATE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 1)) {
		int fd;
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTCREATE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "open(%s)\n", DummyFile);
		fd = open(DummyFile, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		DMLOG_PRINT(DMLVL_DEBUG, "open(%s) returned %d\n", DummyFile,
			    rc);
		rc = (fd == -1) ? -1 : 0;
		EVENT_DELIVERY_DELAY;
		if ((varStatus =
		     DMVAR_CHKPASSEXP(0, rc, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			struct stat statfs;

			rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
			rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
			rc |= stat(DummyFile, &statfs);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Unable to obtain inode!\n");
				varStatus = DMSTAT_FAIL;
			} else if (ino1 != ROOT_INODE) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handle NOT root! (%lld vs %d)\n",
					    ino1, ROOT_INODE);
				varStatus = DMSTAT_FAIL;
			} else if (ino2 != statfs.st_ino) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry handle NOT correct! (%lld vs %d)\n",
					    ino2, statfs.st_ino);
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(aname1, DUMMY_FILE) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry name NOT correct! (%s vs %s)\n",
					    aname1, DUMMY_FILE);
				varStatus = DMSTAT_FAIL;
			} else if (dm_handle_cmp(hanp1, hlen1, ahanp1, ahlen1)
				   != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handles NOT same!\n");
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(name1, aname1) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry names NOT same! (%s vs %s)\n",
					    name1, aname1);
				varStatus = DMSTAT_FAIL;
			} else if (amode != statfs.st_mode) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Modes NOT same! (%d vs %d)\n",
					    amode, statfs.st_mode);
				varStatus = DMSTAT_FAIL;
			} else if (aretcode != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Return codes NOT same! (%d vs %d)\n",
					    mode, amode);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
		EVENT_DELIVERY_DELAY;
		rc = close(fd);
		rc |= remove(DummyFile);
		EVENT_DELIVERY_DELAY;
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : remove - enabled on fs
	 * EXPECTED: DM_EVENT_POSTREMOVE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 2)) {
		int fd;
		dm_ino_t ino;
		struct stat statfs;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTREMOVE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if (((rc = close(fd)) == -1) ||
			   ((rc = stat(DummyFile, &statfs)) == -1)) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "remove(%s)\n", DummyFile);
			rc = remove(DummyFile);
			DMLOG_PRINT(DMLVL_DEBUG, "remove(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (amode != statfs.st_mode) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Modes NOT same! (%d vs %d)\n",
						    amode, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : mv - enabled on fs
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 3)) {
		int fd;
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummyFile, DummyFile2);
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummyFile, DummyFile2);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n", DummyFile,
				    DummyFile2, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT root! (%lld vs %d)\n",
						    ino2, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(ahanp1, ahlen1, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT equal to new parent handle!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_FILE2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_FILE2);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : symlink - enabled on fs
	 * EXPECTED: DM_EVENT_POSTSYMLINK
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 4)) {
		int fd;
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTSYMLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "symlink(%s, %s)\n", DummyFile,
				    DummyLink);
			rc = symlink(DummyFile, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "symlink(%s, %s) returned %d\n", DummyFile,
				    DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				rc |= lstat(DummyLink, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry handle NOT correct! (%lld vs %d)\n",
						    ino2, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_LINK) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_LINK);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DummyFile) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT correct! (%s vs %s)\n",
						    aname2, DummyFile);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = unlink(DummyLink);
			rc |= remove(DummyFile);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : link - enabled on fs
	 * EXPECTED: DM_EVENT_POSTLINK
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 5)) {
		dm_ino_t ino, ino1, ino2;
		void *hanp;
		size_t hlen;
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s)\n", DummyFile,
				    DummyLink);
			rc = link(DummyFile, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s) returned %d\n",
				    DummyFile, DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(hanp1, hlen1, &ino1);
				rc |= dm_handle_to_ino(hanp2, hlen2, &ino2);
				rc |= dm_handle_to_ino(hanp, hlen, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%d vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Source link handle NOT correct! (%d vs %d)\n",
						    ino2, ino);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_LINK) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Target entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_LINK);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = close(fd);
			rc |= remove(DummyFile);
			rc |= remove(DummyLink);
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
	 *  Next batch of tests will be with events enabled on directory, so
	 *  clear events on fs and set up events on dir accordingly
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
	rc = dm_set_eventlist(sid, fshanp, fshlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist(fs) failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	rc = dm_path_to_handle(mountPt, &dhanp, &dhlen);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_path_to_handle failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	DMEV_ZERO(events);
	DMEV_SET(DM_EVENT_CREATE, events);
	DMEV_SET(DM_EVENT_POSTCREATE, events);
	DMEV_SET(DM_EVENT_REMOVE, events);
	DMEV_SET(DM_EVENT_POSTREMOVE, events);
	DMEV_SET(DM_EVENT_RENAME, events);
	DMEV_SET(DM_EVENT_POSTRENAME, events);
	DMEV_SET(DM_EVENT_SYMLINK, events);
	DMEV_SET(DM_EVENT_POSTSYMLINK, events);
	DMEV_SET(DM_EVENT_LINK, events);
	DMEV_SET(DM_EVENT_POSTLINK, events);
	rc = dm_set_eventlist(sid, dhanp, dhlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist(dir) failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	/*
	 * TEST    : mkdir - enabled on directory
	 * EXPECTED: DM_EVENT_POSTCREATE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 6)) {
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTCREATE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "mkdir(%s)\n", DummySubdir);
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		DMLOG_PRINT(DMLVL_DEBUG, "mkdir(%s) returned %d\n", DummySubdir,
			    rc);
		EVENT_DELIVERY_DELAY;
		if ((varStatus =
		     DMVAR_CHKPASSEXP(0, rc, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			struct stat statfs;

			rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
			rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
			rc |= stat(DummySubdir, &statfs);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Unable to obtain inode!\n");
				varStatus = DMSTAT_FAIL;
			} else if (ino1 != ROOT_INODE) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handle NOT root! (%lld vs %d)\n",
					    ino1, ROOT_INODE);
				varStatus = DMSTAT_FAIL;
			} else if (ino2 != statfs.st_ino) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry handle NOT correct! (%lld vs %d)\n",
					    ino2, statfs.st_ino);
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry name NOT correct! (%s vs %s)\n",
					    aname1, DUMMY_SUBDIR);
				varStatus = DMSTAT_FAIL;
			} else if (dm_handle_cmp(hanp1, hlen1, ahanp1, ahlen1)
				   != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handles NOT same!\n");
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(name1, aname1) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry names NOT same! (%s vs %s)\n",
					    name1, aname1);
				varStatus = DMSTAT_FAIL;
			} else if (amode != statfs.st_mode) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Modes NOT same! (%d vs %d)\n",
					    amode, statfs.st_mode);
				varStatus = DMSTAT_FAIL;
			} else if (aretcode != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Return codes NOT same! (%d vs %d)\n",
					    mode, amode);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
		EVENT_DELIVERY_DELAY;
		rc = rmdir(DummySubdir);
		EVENT_DELIVERY_DELAY;
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : rmdir - enabled on directory
	 * EXPECTED: DM_EVENT_POSTREMOVE
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 7)) {
		dm_ino_t ino;
		struct stat statfs;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTREMOVE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = stat(DummySubdir, &statfs)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "rmdir(%s)\n", DummySubdir);
			rc = rmdir(DummySubdir);
			DMLOG_PRINT(DMLVL_DEBUG, "rmdir(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (amode != statfs.st_mode) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Modes NOT same! (%d vs %d)\n",
						    amode, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : mv - enabled on source and destination directory
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 8)) {
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummySubdir, DummySubdir2);
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummySubdir, DummySubdir2);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n",
				    DummySubdir, DummySubdir2, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT root! (%lld vs %d)\n",
						    ino2, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(ahanp1, ahlen1, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT equal to new parent handle!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_SUBDIR2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_SUBDIR2);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : mv - enabled on source directory
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 9)) {
		void *dhanp;
		size_t dhlen;
		dm_ino_t ino1, ino2, dino;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummySubdir, DummySubdir2Subdir);
		EVENT_DELIVERY_DELAY;
		if ((rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DummySubdir2, DUMMY_DIR_RW_MODE)) == -1) {
			rmdir(DummySubdir);
		} else
		    if ((rc =
			 dm_path_to_handle(DummySubdir2, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DummySubdir2);
			rmdir(DummySubdir);
		} else if ((rc = dm_handle_to_ino(dhanp, dhlen, &dino)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DummySubdir2);
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
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummySubdir, DummySubdir2Subdir);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n",
				    DummySubdir, DummySubdir2Subdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != dino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT correct! (%lld vs %d)\n",
						    ino2, dino);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir2Subdir);
			rc |= rmdir(DummySubdir2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : mv - enabled on destination directory
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 10)) {
		void *dhanp;
		size_t dhlen;
		dm_ino_t ino1, ino2, dino;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummySubdir2Subdir, DummySubdir);
		EVENT_DELIVERY_DELAY;
		if ((rc = mkdir(DummySubdir2, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = mkdir(DummySubdir2Subdir, DUMMY_DIR_RW_MODE))
			   == -1) {
			rmdir(DummySubdir2);
		} else
		    if ((rc =
			 dm_path_to_handle(DummySubdir2, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DummySubdir2Subdir);
			rmdir(DummySubdir2);
		} else if ((rc = dm_handle_to_ino(dhanp, dhlen, &dino)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DummySubdir2Subdir);
			rmdir(DummySubdir2);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummySubdir2Subdir, DummySubdir);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n",
				    DummySubdir2Subdir, DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != dino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT correct! (%lld vs %d)\n",
						    ino1, dino);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT root! (%lld vs %d)\n",
						    ino2, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_SUBDIR) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_SUBDIR);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir2);
			rc |= rmdir(DummySubdir);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(dhanp, dhlen);
		}
	}

	/*
	 * TEST    : symlink - enabled on directory
	 * EXPECTED: DM_EVENT_POSTSYMLINK
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 11)) {
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTSYMLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "symlink(%s, %s)\n",
				    DummySubdir, DummySubdir2);
			rc = symlink(DummySubdir, DummySubdir2);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "symlink(%s, %s) returned %d\n",
				    DummySubdir, DummySubdir2, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				rc |= lstat(DummySubdir2, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry handle NOT correct! (%lld vs %d)\n",
						    ino2, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_SUBDIR2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_SUBDIR2);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DummySubdir) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT correct! (%s vs %s)\n",
						    aname2, DummySubdir);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = unlink(DummySubdir2);
			rc |= rmdir(DummySubdir);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : link - enabled on directory
	 * EXPECTED: DM_EVENT_POSTLINK
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 12)) {
#ifdef DIRECTORY_LINKS
		dm_ino_t ino, ino1, ino2;
		void *hanp;
		size_t hlen;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s)\n", DummySubdir,
				    DummyLink);
			rc = link(DummySubdir, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s) returned %d\n",
				    DummySubdir, DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(hanp1, hlen1, &ino1);
				rc |= dm_handle_to_ino(hanp2, hlen2, &ino2);
				rc |= dm_handle_to_ino(hanp, hlen, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%d vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Source link handle NOT correct! (%d vs %d)\n",
						    ino2, ino);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_LINK) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Target entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_LINK);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			rc |= unlink(DummyLink);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with DIRECTORY_LINKS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : open - enabled on directory
	 * EXPECTED: DM_EVENT_POSTCREATE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 6)) {
		int fd;
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTCREATE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "open(%s)\n", DummyFile);
		fd = open(DummyFile, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		DMLOG_PRINT(DMLVL_DEBUG, "open(%s) returned %d\n", DummyFile,
			    rc);
		rc = (fd == -1) ? -1 : 0;
		EVENT_DELIVERY_DELAY;
		if ((varStatus =
		     DMVAR_CHKPASSEXP(0, rc, eventExpected,
				      eventReceived)) == DMSTAT_PASS) {
			struct stat statfs;

			rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
			rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
			rc |= stat(DummyFile, &statfs);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Unable to obtain inode!\n");
				varStatus = DMSTAT_FAIL;
			} else if (ino1 != ROOT_INODE) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handle NOT root! (%lld vs %d)\n",
					    ino1, ROOT_INODE);
				varStatus = DMSTAT_FAIL;
			} else if (ino2 != statfs.st_ino) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry handle NOT correct! (%lld vs %d)\n",
					    ino2, statfs.st_ino);
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(aname1, DUMMY_FILE) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry name NOT correct! (%s vs %s)\n",
					    aname1, DUMMY_FILE);
				varStatus = DMSTAT_FAIL;
			} else if (dm_handle_cmp(hanp1, hlen1, ahanp1, ahlen1)
				   != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Parent handles NOT same!\n");
				varStatus = DMSTAT_FAIL;
			} else if (strcmp(name1, aname1) != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Entry names NOT same! (%s vs %s)\n",
					    name1, aname1);
				varStatus = DMSTAT_FAIL;
			} else if (amode != statfs.st_mode) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Modes NOT same! (%d vs %d)\n",
					    amode, statfs.st_mode);
				varStatus = DMSTAT_FAIL;
			} else if (aretcode != 0) {
				DMLOG_PRINT(DMLVL_ERR,
					    "Return codes NOT same! (%d vs %d)\n",
					    mode, amode);
				varStatus = DMSTAT_FAIL;
			}
		}
		DMVAR_END(varStatus);

		/* Variation clean up */
		EVENT_DELIVERY_DELAY;
		rc = close(fd);
		rc |= remove(DummyFile);
		EVENT_DELIVERY_DELAY;
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : remove - enabled on directory
	 * EXPECTED: DM_EVENT_POSTREMOVE
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 7)) {
		int fd;
		dm_ino_t ino;
		struct stat statfs;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTREMOVE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if (((rc = close(fd)) == -1) ||
			   ((rc = stat(DummyFile, &statfs)) == -1)) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "remove(%s)\n", DummyFile);
			rc = remove(DummyFile);
			DMLOG_PRINT(DMLVL_DEBUG, "remove(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (amode != statfs.st_mode) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Modes NOT same! (%d vs %d)\n",
						    amode, statfs.st_mode);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : mv - enabled on source and destination directory
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 8)) {
		int fd;
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummyFile, DummyFile2);
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummyFile, DummyFile2);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n", DummyFile,
				    DummyFile2, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT root! (%lld vs %d)\n",
						    ino2, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(ahanp1, ahlen1, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT equal to new parent handle!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_FILE2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_FILE2);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : mv - enabled on source directory
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 9)) {
		int fd;
		void *dhanp;
		size_t dhlen;
		dm_ino_t ino1, ino2, dino;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummyFile, DummySubdir2File);
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if (((rc = close(fd)) == -1) ||
			   ((rc =
			     mkdir(DummySubdir2, DUMMY_DIR_RW_MODE)) == -1)) {
			remove(DummyFile);
		} else
		    if ((rc =
			 dm_path_to_handle(DummySubdir2, &dhanp,
					   &dhlen)) == -1) {
			rmdir(DummySubdir2);
			remove(DummyFile);
		} else if ((rc = dm_handle_to_ino(dhanp, dhlen, &dino)) == -1) {
			dm_handle_free(dhanp, dhlen);
			rmdir(DummySubdir2);
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
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummyFile, DummySubdir2File);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n", DummyFile,
				    DummySubdir2File, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != dino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT correct! (%lld vs %d)\n",
						    ino2, dino);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummySubdir2File);
			rc |= rmdir(DummySubdir2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : mv - enabled on destination directory
	 * EXPECTED: DM_EVENT_POSTRENAME
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 10)) {
		int fd;
		void *dhanp;
		size_t dhlen;
		dm_ino_t ino1, ino2, dino;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTRENAME;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummySubdir2File, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = mkdir(DummySubdir2, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummySubdir2File, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			rmdir(DummySubdir2);
		} else if ((rc = close(fd)) == -1) {
			remove(DummySubdir2File);
			rmdir(DummySubdir2);
		} else
		    if ((rc =
			 dm_path_to_handle(DummySubdir2, &dhanp,
					   &dhlen)) == -1) {
			remove(DummySubdir2File);
			rmdir(DummySubdir2);
		} else if ((rc = dm_handle_to_ino(dhanp, dhlen, &dino)) == -1) {
			dm_handle_free(dhanp, dhlen);
			remove(DummySubdir2File);
			rmdir(DummySubdir2);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummySubdir2File, DummyFile);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n",
				    DummySubdir2File, DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != dino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handle NOT correct! (%lld vs %d)\n",
						    ino1, dino);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handle NOT root! (%lld vs %d)\n",
						    ino2, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp2, hlen2, ahanp2, ahlen2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT correct! (%s vs %s)\n",
						    name2, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Old entry name NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "New entry name NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile);
			rc |= rmdir(DummySubdir2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : symlink - enabled on directory
	 * EXPECTED: DM_EVENT_POSTSYMLINK
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 11)) {
		int fd;
		dm_ino_t ino1, ino2;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTSYMLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "symlink(%s, %s)\n", DummyFile,
				    DummyLink);
			rc = symlink(DummyFile, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "symlink(%s, %s) returned %d\n", DummyFile,
				    DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				struct stat statfs;

				rc = dm_handle_to_ino(ahanp1, ahlen1, &ino1);
				rc |= dm_handle_to_ino(ahanp2, ahlen2, &ino2);
				rc |= lstat(DummyLink, &statfs);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%lld vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != statfs.st_ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry handle NOT correct! (%lld vs %d)\n",
						    ino2, statfs.st_ino);
					varStatus = DMSTAT_FAIL;
				} else
				    if (dm_handle_cmp
					(hanp1, hlen1, ahanp1, ahlen1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname1, DUMMY_LINK) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry name NOT correct! (%s vs %s)\n",
						    aname1, DUMMY_LINK);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(aname2, DummyFile) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT correct! (%s vs %s)\n",
						    aname2, DummyFile);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, aname1) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink entry names NOT same! (%s vs %s)\n",
						    name1, aname1);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name2, aname2) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Symlink contents NOT same! (%s vs %s)\n",
						    name2, aname2);
					varStatus = DMSTAT_FAIL;
				} else if (aretcode != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Return codes NOT same! (%d vs %d)\n",
						    mode, amode);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = unlink(DummyLink);
			rc |= remove(DummyFile);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : link - enabled on directory
	 * EXPECTED: DM_EVENT_POSTLINK
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 12)) {
		dm_ino_t ino, ino1, ino2;
		void *hanp;
		size_t hlen;
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_POSTLINK;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s)\n", DummyFile,
				    DummyLink);
			rc = link(DummyFile, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s) returned %d\n",
				    DummyFile, DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_to_ino(hanp1, hlen1, &ino1);
				rc |= dm_handle_to_ino(hanp2, hlen2, &ino2);
				rc |= dm_handle_to_ino(hanp, hlen, &ino);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Unable to obtain inode!\n");
					varStatus = DMSTAT_FAIL;
				} else if (ino1 != ROOT_INODE) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Parent handle NOT root! (%d vs %d)\n",
						    ino1, ROOT_INODE);
					varStatus = DMSTAT_FAIL;
				} else if (ino2 != ino) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Source link handle NOT correct! (%d vs %d)\n",
						    ino2, ino);
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_LINK) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Target entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_LINK);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = close(fd);
			rc |= remove(DummyFile);
			rc |= remove(DummyLink);
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
	 *  Last batch of tests will be with events disabled, so clear events
	 *  on dir
	 */
	DMEV_ZERO(events);
	rc = dm_set_eventlist(sid, dhanp, dhlen, DM_NO_TOKEN, &events,
			      DM_EVENT_MAX);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_set_eventlist(dir) failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		dm_destroy_session(sid);
		DM_EXIT();
	}

	/*
	 * TEST    : mkdir - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 13)) {

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "mkdir(%s)\n", DummySubdir);
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		DMLOG_PRINT(DMLVL_DEBUG, "mkdir(%s) returned %d\n", DummySubdir,
			    rc);
		EVENT_DELIVERY_DELAY;
		DMVAR_END(DMVAR_CHKPASSEXP
			  (0, rc, eventExpected, eventReceived));

		/* Variation clean up */
		EVENT_DELIVERY_DELAY;
		rc = rmdir(DummySubdir);
		EVENT_DELIVERY_DELAY;
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : rmdir - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 14)) {

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "rmdir(%s)\n", DummySubdir);
			rc = rmdir(DummySubdir);
			DMLOG_PRINT(DMLVL_DEBUG, "rmdir(%s) returned %d\n",
				    DummySubdir, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : mv - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 15)) {

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummySubdir, DummySubdir2);
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummySubdir, DummySubdir2);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n",
				    DummySubdir, DummySubdir2, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : symlink - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 16)) {

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "symlink(%s, %s)\n",
				    DummySubdir, DummySubdir2);
			rc = symlink(DummySubdir, DummySubdir2);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "symlink(%s, %s) returned %d\n",
				    DummySubdir, DummySubdir2, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = unlink(DummySubdir2);
			rc |= rmdir(DummySubdir);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : link - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(DIR_ASYNC_NAMESP_EVENT_BASE + 17)) {
#ifdef DIRECTORY_LINKS
		void *hanp;
		size_t hlen;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		rc = mkdir(DummySubdir, DUMMY_DIR_RW_MODE);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s)\n", DummySubdir,
				    DummyLink);
			rc = link(DummySubdir, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s) returned %d\n",
				    DummySubdir, DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = rmdir(DummySubdir);
			rc |= unlink(DummyLink);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with DIRECTORY_LINKS defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : open - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 13)) {
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;

		/* Variation */
		EVENT_DELIVERY_DELAY;
		DMLOG_PRINT(DMLVL_DEBUG, "open(%s)\n", DummyFile);
		fd = open(DummyFile, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		DMLOG_PRINT(DMLVL_DEBUG, "open(%s) returned %d\n", DummyFile,
			    rc);
		rc = (fd == -1) ? -1 : 0;
		EVENT_DELIVERY_DELAY;
		DMVAR_END(DMVAR_CHKPASSEXP
			  (0, rc, eventExpected, eventReceived));

		/* Variation clean up */
		EVENT_DELIVERY_DELAY;
		rc = close(fd);
		rc |= remove(DummyFile);
		EVENT_DELIVERY_DELAY;
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to clean up variation! (errno = %d)\n",
				    errno);
		}
	}

	/*
	 * TEST    : remove - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 14)) {
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "remove(%s)\n", DummyFile);
			rc = remove(DummyFile);
			DMLOG_PRINT(DMLVL_DEBUG, "remove(%s) returned %d\n",
				    DummyFile, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
		}
	}

	/*
	 * TEST    : mv - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 15)) {
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		sprintf(command, "mv %s %s", DummyFile, DummyFile2);
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "system(mv %s %s)\n",
				    DummyFile, DummyFile2);
			rc = system(command);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "system(mv %s %s) returned %d\n", DummyFile,
				    DummyFile2, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = remove(DummyFile2);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : symlink - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 16)) {
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "symlink(%s, %s)\n", DummyFile,
				    DummyLink);
			rc = symlink(DummyFile, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "symlink(%s, %s) returned %d\n", DummyFile,
				    DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = unlink(DummyLink);
			rc |= remove(DummyFile);
			EVENT_DELIVERY_DELAY;
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : link - disabled
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_ASYNC_NAMESP_EVENT_BASE + 17)) {
		void *hanp;
		size_t hlen;
		int fd;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		EVENT_DELIVERY_DELAY;
		if ((fd =
		     open(DummyFile, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
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
			EVENT_DELIVERY_DELAY;
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s)\n", DummyFile,
				    DummyLink);
			rc = link(DummyFile, DummyLink);
			DMLOG_PRINT(DMLVL_DEBUG, "link(%s, %s) returned %d\n",
				    DummyFile, DummyLink, rc);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = close(fd);
			rc |= remove(DummyFile);
			rc |= remove(DummyLink);
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
			DMEV_SET(DM_EVENT_POSTCREATE, events);
			DMEV_SET(DM_EVENT_REMOVE, events);
			DMEV_SET(DM_EVENT_POSTREMOVE, events);
			DMEV_SET(DM_EVENT_RENAME, events);
			DMEV_SET(DM_EVENT_POSTRENAME, events);
			DMEV_SET(DM_EVENT_SYMLINK, events);
			DMEV_SET(DM_EVENT_POSTSYMLINK, events);
			DMEV_SET(DM_EVENT_LINK, events);
			DMEV_SET(DM_EVENT_POSTLINK, events);
			rc = dm_set_disp(sid, lhanp, lhlen, token, &events,
					 DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
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
			response = eventResponse;

			switch (type) {
			case DM_EVENT_CREATE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					mode = nse->ne_mode;
					hanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					hlen1 = DM_GET_LEN(nse, ne_handle1);
					strcpy(name1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_CREATE\n");
					DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n",
						    mode);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry name: %s\n",
						    name1);

					break;
				}

			case DM_EVENT_POSTCREATE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					amode = nse->ne_mode;
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);
					ahanp2 =
					    DM_GET_VALUE(nse, ne_handle2,
							 void *);
					ahlen2 = DM_GET_LEN(nse, ne_handle2);
					strcpy(aname1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					aretcode = nse->ne_retcode;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_POSTCREATE\n");
					DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n",
						    amode);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    ahlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry handle: %p\n",
						    ahanp2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry handle length: %d\n",
						    ahlen2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry name: %s\n",
						    aname1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Return code: %x\n",
						    aretcode);

					response = DM_RESP_INVALID;
					break;
				}

			case DM_EVENT_REMOVE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					mode = nse->ne_mode;
					hanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					hlen1 = DM_GET_LEN(nse, ne_handle1);
					strcpy(name1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_REMOVE\n");
					DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n",
						    mode);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry name: %s\n",
						    name1);

					break;
				}

			case DM_EVENT_POSTREMOVE:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					amode = nse->ne_mode;
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);
					strcpy(aname1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					aretcode = nse->ne_retcode;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_POSTREMOVE\n");
					DMLOG_PRINT(DMLVL_DEBUG, "  Mode: %x\n",
						    amode);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    ahlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry name: %s\n",
						    aname1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Return code: %x\n",
						    aretcode);

					response = DM_RESP_INVALID;
					break;
				}

			case DM_EVENT_RENAME:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					hanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					hlen1 = DM_GET_LEN(nse, ne_handle1);
					hanp2 =
					    DM_GET_VALUE(nse, ne_handle2,
							 void *);
					hlen2 = DM_GET_LEN(nse, ne_handle2);
					strcpy(name1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					strcpy(name2,
					       DM_GET_VALUE(nse, ne_name2,
							    char *));

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_RENAME\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Old parent handle: %p\n",
						    hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Old parent handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  New parent handle: %p\n",
						    hanp2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  New parent handle length: %d\n",
						    hlen2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Old entry name: %s\n",
						    name1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  New entry name: %s\n",
						    name2);

					break;
				}

			case DM_EVENT_POSTRENAME:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);
					ahanp2 =
					    DM_GET_VALUE(nse, ne_handle2,
							 void *);
					ahlen2 = DM_GET_LEN(nse, ne_handle2);
					strcpy(aname1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					strcpy(aname2,
					       DM_GET_VALUE(nse, ne_name2,
							    char *));
					aretcode = nse->ne_retcode;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_POSTRENAME\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Old parent handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Old parent handle length: %d\n",
						    ahlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  New parent handle: %p\n",
						    ahanp2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  New parent handle length: %d\n",
						    ahlen2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Old entry name: %s\n",
						    aname1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  New entry name: %s\n",
						    aname2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Return code: %x\n",
						    aretcode);

					response = DM_RESP_INVALID;
					break;
				}

			case DM_EVENT_SYMLINK:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					hanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					hlen1 = DM_GET_LEN(nse, ne_handle1);
					strcpy(name1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					strcpy(name2,
					       DM_GET_VALUE(nse, ne_name2,
							    char *));

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_SYMLINK\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Symlink entry name: %s\n",
						    name1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Symlink contents: %s\n",
						    name2);

					break;
				}

			case DM_EVENT_POSTSYMLINK:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);
					ahanp2 =
					    DM_GET_VALUE(nse, ne_handle2,
							 void *);
					ahlen2 = DM_GET_LEN(nse, ne_handle2);
					strcpy(aname1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					strcpy(aname2,
					       DM_GET_VALUE(nse, ne_name2,
							    char *));
					aretcode = nse->ne_retcode;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_POSTSYMLINK\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    ahlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry handle: %p\n",
						    ahanp2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Entry handle length: %d\n",
						    ahlen2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Symlink entry name: %s\n",
						    aname1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Symlink contents: %s\n",
						    aname2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Return code: %x\n",
						    aretcode);

					response = DM_RESP_INVALID;
					break;
				}

			case DM_EVENT_LINK:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					hanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					hlen1 = DM_GET_LEN(nse, ne_handle1);
					hanp2 =
					    DM_GET_VALUE(nse, ne_handle2,
							 void *);
					hlen2 = DM_GET_LEN(nse, ne_handle2);
					strcpy(name1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_LINK\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    hanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    hlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Source link handle: %p\n",
						    hanp2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Source link handle length: %d\n",
						    hlen2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Target entry name: %s\n",
						    name1);

					break;
				}

			case DM_EVENT_POSTLINK:
				{
					dm_namesp_event_t *nse =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_namesp_event_t *);
					ahanp1 =
					    DM_GET_VALUE(nse, ne_handle1,
							 void *);
					ahlen1 = DM_GET_LEN(nse, ne_handle1);
					ahanp2 =
					    DM_GET_VALUE(nse, ne_handle2,
							 void *);
					ahlen2 = DM_GET_LEN(nse, ne_handle2);
					strcpy(aname1,
					       DM_GET_VALUE(nse, ne_name1,
							    char *));
					aretcode = nse->ne_retcode;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "Message is DM_EVENT_POSTLINK\n");
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle: %p\n",
						    ahanp1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Parent handle length: %d\n",
						    ahlen1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Source link handle: %p\n",
						    ahanp2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Source link handle length: %d\n",
						    ahlen2);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Target entry name: %s\n",
						    aname1);
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  Return code: %x\n",
						    aretcode);

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
