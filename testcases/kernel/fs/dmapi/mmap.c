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
 * TEST CASE	: mmap.c
 *
 * VARIATIONS	: 18
 *
 * EVENTS TESTED: DM_EVENT_READ
 * 		  DM_EVENT_WRITE
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "dm_test.h"

#define TMP_FILELEN 50000
#define MMAPFILE_EXE "mmapfile"

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char command[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];
char DummyTmp[FILENAME_MAX];

/* Variables for thread communications */
dm_eventtype_t eventExpected;
dm_eventtype_t eventReceived;
dm_response_t eventResponse;
void *hanp1;
size_t hlen1;
dm_off_t offset;
dm_size_t length;

void *Thread(void *);

int main(int argc, char **argv)
{

	char *varstr;
	int rc;
	int i;
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
		int fd;

		sprintf(DummyFile, "%s/%s", mountPt, DUMMY_FILE);
		sprintf(DummyTmp, "%s/%s", mountPt, DUMMY_TMP);

		remove(DummyFile);

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

	DMLOG_PRINT(DMLVL_DEBUG,
		    "Starting DMAPI memory mapped file synchronous event data tests\n");

	/*
	 * TEST    : mmap - no regions
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = 0;
		size_t inlen = DUMMY_STRLEN;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_INVALID;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_REGION_WRITE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = 0;
		size_t inlen = DUMMY_STRLEN;
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_INVALID;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_REGION_TRUNCATE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = 0;
		size_t inlen = DUMMY_STRLEN;
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_INVALID;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_EVENT_READ, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 *
	 * This variation uncovered XFS BUG #33 (entire file returned instead
	 * of mapped region only)
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(0);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int varStatus;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_READ;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (inoff != offset) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    inoff, offset);
					varStatus = DMSTAT_FAIL;
				} else if (inlen != length) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    inlen, length);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

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
	 * TEST    : mmap - one region, DM_EVENT_READ, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(0);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int varStatus;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_READ;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_ABORT;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 0);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (inoff != offset) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    inoff, offset);
					varStatus = DMSTAT_FAIL;
				} else if (inlen != length) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    inlen, length);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

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
	 * TEST    : mmap - one region, DM_EVENT_READ
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(0);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN / 2;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_EVENT_READ
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(TMP_FILELEN / 4);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN / 2;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_EVENT_READ
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(TMP_FILELEN / 4);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int varStatus;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_READ;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN / 4;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (inoff != offset) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    inoff, offset);
					varStatus = DMSTAT_FAIL;
				} else if (inlen != length) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    inlen, length);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

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
	 * TEST    : mmap - one region, DM_EVENT_READ
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_READ_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(TMP_FILELEN / 4);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDONLY, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - no regions
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = 0;
		size_t inlen = DUMMY_STRLEN;

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_INVALID;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_REGION_READ
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = 0;
		size_t inlen = DUMMY_STRLEN;
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_INVALID;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_REGION_TRUNCATE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = 0;
		size_t inlen = DUMMY_STRLEN;
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_INVALID;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_EVENT_WRITE, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(0);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int varStatus;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_WRITE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (inoff != offset) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    inoff, offset);
					varStatus = DMSTAT_FAIL;
				} else if (inlen != length) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    inlen, length);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

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
	 * TEST    : mmap - one region, DM_EVENT_WRITE, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(0);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int varStatus;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_WRITE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_ABORT;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 0);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (inoff != offset) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    inoff, offset);
					varStatus = DMSTAT_FAIL;
				} else if (inlen != length) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    inlen, length);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

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
	 * TEST    : mmap - one region, DM_EVENT_WRITE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(0);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN / 2;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_EVENT_WRITE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(TMP_FILELEN / 4);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN / 2;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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
	 * TEST    : mmap - one region, DM_EVENT_WRITE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(TMP_FILELEN / 4);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int varStatus;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_WRITE;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN / 4;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (inoff != offset) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    inoff, offset);
					varStatus = DMSTAT_FAIL;
				} else if (inlen != length) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    inlen, length);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

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
	 * TEST    : mmap - one region, DM_EVENT_WRITE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(MMAP_WRITE_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		off_t inoff = PAGEALIGN(TMP_FILELEN / 4);
		size_t inlen = PAGEALIGN(DUMMY_STRLEN);
		dm_boolean_t exactflag;
		int numRegions;
		dm_region_t Regions[1];

		/* Variation set up */
		eventExpected = DM_EVENT_INVALID;
		eventReceived = DM_EVENT_INVALID;
		eventResponse = DM_RESP_CONTINUE;
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DummyTmp, DummyFile);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DummyFile, O_RDONLY)) == -1) {
			remove(DummyFile);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DummyFile);
		} else
		    if (((rc =
			  dm_set_region(sid, hanp, hlen, DM_NO_TOKEN,
					numRegions, Regions, &exactflag)) == -1)
			|| ((rc = close(fd)) == -1)) {
			dm_handle_free(hanp, hlen);
			remove(DummyFile);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			sprintf(command, "./%s %s %d %ld %ld %d", MMAPFILE_EXE,
				DummyFile, O_RDWR, inoff, (long)inlen, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "invoking %s\n", command);
			rc = system(command);
			EVENT_DELIVERY_DELAY;
			DMVAR_END(DMVAR_CHKPASSEXP
				  (0, rc, eventExpected, eventReceived));

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

	remove(DummyFile);
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
			DMEV_SET(DM_EVENT_READ, events);
			DMEV_SET(DM_EVENT_WRITE, events);
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
		} else if (type == DM_EVENT_UNMOUNT) {
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);
			if (nse->ne_retcode == 0) {
				bMounted = DM_FALSE;
			}

			response = DM_RESP_CONTINUE;
		} else {
			eventReceived = type;

			switch (type) {
			case DM_EVENT_PREUNMOUNT:
				response = DM_RESP_CONTINUE;
				break;

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

					response = eventResponse;
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

					response = eventResponse;
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
