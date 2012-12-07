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
 * TEST CASE	: event_sd.c
 *
 * VARIATIONS	: 89
 *
 * EVENTS TESTED: DM_EVENT_READ
 * 		  DM_EVENT_WRITE
 * 		  DM_EVENT_TRUNCATE
 *
 * NOTES	: The EVENT_DELIVERY_DELAY_LOOP macro is needed prior to
 * 		  invoking the routine (i.e. read) that generates the event
 * 		  being tested because the system("cp DUMMY_FILE DummyFile")
 * 		  call will generate DM_EVENT_WRITE events that can interfere
 * 		  with the results of the test.
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

#define TMP_FILELEN 10000

pthread_t tid;
dm_sessid_t sid;
char dmMsgBuf[4096];
char command[4096];
char *mountPt;
char *deviceNm;
char DummyFile[FILENAME_MAX];

/* Variables for thread communications */
dm_eventtype_t eventExpected;
dm_eventtype_t eventReceived;
dm_response_t eventResponse;
void *hanp1, *hanp2;
size_t hlen1, hlen2;
char name1[FILENAME_MAX];
dm_off_t offset;
dm_size_t length;
int numRegions;
dm_region_t Regions[10];
int eventPending;

void *Thread(void *);

int main(int argc, char **argv)
{

	char *varstr;
	int i;
	int rc;
	int varStatus;
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

		remove(DummyFile);

		EVENT_DELIVERY_DELAY;
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
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
		    "Starting DMAPI synchronous data event tests\n");

	/*
	 * TEST    : read - no regions
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 1)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 0;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one READ region covering entire file, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 2)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one READ region covering entire file, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_READ (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 3)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "read(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one WRITE region covering entire file
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 4)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one TRUNCATE region covering entire file
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 5)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - two READ regions covering part of file, read from one, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 6)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2222, SEEK_SET)) != 2222) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2222) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2222) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2222);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - two READ regions covering part of file, read from one, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_READ (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 7)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2222, SEEK_SET)) != 2222) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2222) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "read(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2222) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2222);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - two non-READ regions covering part of file, read from one
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 8)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - two non-READ regions covering part of file, read from other
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 9)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2222, SEEK_SET)) != 2222) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2222) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - three different regions covering part of file, read from READ, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 10)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 4444, SEEK_SET)) != 4444) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 4444) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 4444) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 4444);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - three different regions covering part of file, read from READ, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_READ (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 11)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 4444, SEEK_SET)) != 4444) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 4444) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "read(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 4444) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 4444);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - three READ regions covering part of file, read from outside all
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 12)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_READ;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 3333, SEEK_SET)) != 3333) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 3333) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - three different regions covering part of file, read from READ overlapping start, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 13)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2997, SEEK_SET)) != 2997) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2997) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - three different regions covering part of file, read from READ overlapping start, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_READ (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 14)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2997, SEEK_SET)) != 2997) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2997) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "read(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte READ region one byte before start of read
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 15)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte READ region at start of read, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 16)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte READ region at end of read, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 17)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = DUMMY_STRLEN;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte READ region one byte beyond end of read
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 18)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = DUMMY_STRLEN + 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read one byte before start of multibyte READ region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 19)) {
		int fd;
		char buf[1];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read at start of multibyte READ region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 20)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read at end of multibyte READ region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 21)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, DUMMY_STRLEN, SEEK_SET)) !=
			   DUMMY_STRLEN) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != DUMMY_STRLEN) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read one byte beyond end of multibyte READ region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 22)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, DUMMY_STRLEN + 1, SEEK_SET)) !=
			   DUMMY_STRLEN + 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != DUMMY_STRLEN + 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read one byte before start of one-byte READ region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 23)) {
		int fd;
		char buf[1];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read at start/end of one-byte READ region
	 * EXPECTED: DM_EVENT_READ, DM_RESP_CONTINUE
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 24)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one-byte read one byte beyond end of one-byte READ region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 25)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2, SEEK_SET)) != 2) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, 1, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one R/W/T region, read from it, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 26)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN / 2;
		Regions[0].rg_flags =
		    DM_REGION_READ | DM_REGION_WRITE | DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - three overlapping R/W/T regions, read from them, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 27)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN / 2;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 0;
		Regions[1].rg_size = DUMMY_STRLEN / 2;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 0;
		Regions[2].rg_size = DUMMY_STRLEN / 2;
		Regions[2].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - one DM_REGION_NOEVENT region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 28)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN / 2;
		Regions[0].rg_flags = DM_REGION_NOEVENT;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			rc = read(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - read from different fd than initialized READ region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 29)) {
		int fd1, fd2;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd1 =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd1 == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			fd2 = open(DummyFile, O_RDWR);
			rc = fd2 == -1 ? -1 : read(fd2, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = close(fd1);
			rc |= close(fd2);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : read - dm_pending without O_NONBLOCK
	 * EXPECTED: DM_EVENT_READ
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 30)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			eventPending = 1;
			rc = read(fd, buf, DUMMY_STRLEN);
			eventPending = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "read(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : read - dm_pending with O_NONBLOCK
	 * EXPECTED: DM_EVENT_READ (rc = -1, errno = EAGAIN)
	 *
	 * This variation uncovered XFS BUG #40 (returned errno instead of
	 * -1 and errno)
	 */
	if (DMVAR_EXEC(FILE_READ_DATA_EVENT_BASE + 31)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT | O_NONBLOCK,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_READ;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "read(%s)\n", DummyFile);
			eventPending = 1;
			rc = read(fd, buf, DUMMY_STRLEN);
			eventPending = 0;
			DMLOG_PRINT(DMLVL_DEBUG, "read(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, EAGAIN, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - no regions
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 1)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 0;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one WRITE region covering entire file, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 2)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one WRITE region covering entire file, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_WRITE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 3)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one READ region covering entire file
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 4)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one TRUNCATE region covering entire file
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 5)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - two WRITE regions covering part of file, write to one, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 6)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2222, SEEK_SET)) != 2222) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2222) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2222) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2222);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - two WRITE regions covering part of file, write to one, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_WRITE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 7)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2222, SEEK_SET)) != 2222) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2222) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2222) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2222);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - two non-WRITE regions covering part of file, write to one
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 8)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - two non-WRITE regions covering part of file, write to other
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 9)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2222, SEEK_SET)) != 2222) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2222) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "write(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - three different regions covering part of file, write to WRITE, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 10)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 4444, SEEK_SET)) != 4444) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 4444) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 4444) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 4444);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - three different regions covering part of file, write to WRITE, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_WRITE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 11)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 4444, SEEK_SET)) != 4444) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 4444) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 4444) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 4444);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - three WRITE regions covering part of file, write to outside all
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 12)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_WRITE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 3333, SEEK_SET)) != 3333) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 3333) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - three different regions covering part of file, write to WRITE overlapping start, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 13)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2997, SEEK_SET)) != 2997) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2997) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - three different regions covering part of file, write to WRITE overlapping start, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_WRITE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 14)) {
		int fd;
		char buf[DUMMY_STRLEN];
		dm_off_t off;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2997, SEEK_SET)) != 2997) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2997) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte WRITE region one byte before start of write
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 15)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte WRITE region at start of write, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 16)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte WRITE region at end of write, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 17)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = DUMMY_STRLEN;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte WRITE region one byte beyond end of write
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 18)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = DUMMY_STRLEN + 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write one byte before start of multibyte WRITE region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 19)) {
		int fd;
		char buf[1];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '0';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write at start of multibyte WRITE region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 20)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '1';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write at end of multibyte WRITE region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 21)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, DUMMY_STRLEN, SEEK_SET)) !=
			   DUMMY_STRLEN) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != DUMMY_STRLEN) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '0';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write one byte beyond end of multibyte WRITE region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 22)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, DUMMY_STRLEN + 1, SEEK_SET)) !=
			   DUMMY_STRLEN + 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != DUMMY_STRLEN + 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '1';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write one byte before start of one-byte WRITE region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 23)) {
		int fd;
		char buf[1];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '0';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write at start/end of one-byte WRITE region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 24)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 1, SEEK_SET)) != 1) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '1';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one-byte write one byte beyond end of one-byte WRITE region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 25)) {
		int fd;
		char buf[1];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else if ((off = lseek(fd, 2, SEEK_SET)) != 2) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != 2) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			buf[0] = '2';
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(1, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one R/W/T region, write to it, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 26)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN / 2;
		Regions[0].rg_flags =
		    DM_REGION_READ | DM_REGION_WRITE | DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - three overlapping R/W/T regions, write to them, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 27)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN / 2;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 0;
		Regions[1].rg_size = DUMMY_STRLEN / 2;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 0;
		Regions[2].rg_size = DUMMY_STRLEN / 2;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one DM_REGION_NOEVENT region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 28)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN / 2;
		Regions[0].rg_flags = DM_REGION_NOEVENT;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one WRITE region covering entire file (size 0), write beyond EOF, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 29)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else
		    if ((off =
			 lseek(fd, TMP_FILELEN + DUMMY_STRLEN,
			       SEEK_SET)) != TMP_FILELEN + DUMMY_STRLEN) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != TMP_FILELEN + DUMMY_STRLEN) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != TMP_FILELEN + DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - one WRITE region covering entire file (size TMP_FILELEN), write beyond EOF, DM_RESP_CONTINUE
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 30)) {
		int fd;
		char buf[DUMMY_STRLEN];
		off_t off;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = TMP_FILELEN;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		} else
		    if ((off =
			 lseek(fd, TMP_FILELEN + DUMMY_STRLEN,
			       SEEK_SET)) != TMP_FILELEN + DUMMY_STRLEN) {
			close(fd);
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1 || off != TMP_FILELEN + DUMMY_STRLEN) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			rc = write(fd, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - write to different fd than initialized WRITE region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 31)) {
		int fd1, fd2;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd1 =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd1 == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			fd2 = open(DummyFile, O_RDWR);
			rc = fd2 == -1 ? -1 : write(fd2, buf, DUMMY_STRLEN);
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
				}
			}
			DMVAR_END(varStatus);

			/* Variation clean up */
			EVENT_DELIVERY_DELAY;
			rc = close(fd1);
			rc |= close(fd2);
			rc |= remove(DummyFile);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
		}
	}

	/*
	 * TEST    : write - dm_pending without O_NONBLOCK
	 * EXPECTED: DM_EVENT_WRITE
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 32)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			eventPending = 1;
			rc = write(fd, buf, DUMMY_STRLEN);
			eventPending = 0;
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(DUMMY_STRLEN, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : write - dm_pending with O_NONBLOCK
	 * EXPECTED: DM_EVENT_WRITE (rc = -1, errno = EAGAIN)
	 */
	if (DMVAR_EXEC(FILE_WRITE_DATA_EVENT_BASE + 33)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT | O_NONBLOCK,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_WRITE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "write(%s)\n", DummyFile);
			eventPending = 1;
			rc = write(fd, buf, DUMMY_STRLEN);
			eventPending = 0;
			DMLOG_PRINT(DMLVL_DEBUG, "write(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, EAGAIN, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
				} else if (length != DUMMY_STRLEN) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Length NOT correct! (%d vs %d)\n",
						    length, DUMMY_STRLEN);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - no regions
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 1)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 0;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 5000);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - one TRUNCATE region covering entire file, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 2)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 5000);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 5000) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 5000);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - one TRUNCATE region covering entire file, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_TRUNCATE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 3)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 5000);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 5000) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 5000);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - one READ region covering entire file
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 4)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 5000);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - one WRITE region covering entire file
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 5)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 5000);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - two TRUNCATE regions covering part of file, truncate in one, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 6)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 2222);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2222) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2222);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - two TRUNCATE regions covering part of file, truncate in one, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_TRUNCATE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 7)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 2222);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2222) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2222);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - two non-TRUNCATE regions covering part of file, truncate in one
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 8)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 0);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - two non-TRUNCATE regions covering part of file, truncate in other
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 9)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 2;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 2222);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "ftruncate(%s) returned %d, buffer contents [%.*s]\n",
				    DummyFile, rc, DUMMY_STRLEN, buf);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three different regions covering part of file, truncate in TRUNCATE, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 10)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 4444);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 4444) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 4444);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three different regions covering part of file, truncate in TRUNCATE, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_TRUNCATE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 11)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_WRITE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 4444);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 4444) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 4444);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three TRUNCATE regions covering part of file, truncate beyond all
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 12)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 4000;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 6000);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three different regions covering part of file, truncate in TRUNCATE, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 13)) {
		int fd;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			memcpy(buf, DUMMY_STRING, DUMMY_STRLEN);
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 2997);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three different regions covering part of file, truncate in TRUNCATE, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_TRUNCATE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 14)) {
		int fd;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 2997);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 2997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three different regions covering part of file, truncate before TRUNCATE, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 15)) {
		int fd;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 1997);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three different regions covering part of file, truncate before TRUNCATE, DM_RESP_ABORT
	 * EXPECTED: DM_EVENT_TRUNCATE (rc = -1, errno = ABORT_ERRNO)
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 16)) {
		int fd;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 1000;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 2000;
		Regions[1].rg_size = 1000;
		Regions[1].rg_flags = DM_REGION_TRUNCATE;
		Regions[2].rg_offset = 3005;
		Regions[2].rg_size = 1000;
		Regions[2].rg_flags = DM_REGION_WRITE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_ABORT;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 1997);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			eventResponse = DM_RESP_CONTINUE;
			if ((varStatus =
			     DMVAR_CHKFAILEXP(-1, rc, ABORT_ERRNO,
					      eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1997) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - truncate one byte before start of one-byte TRUNCATE region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 17)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 0);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 2997);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - truncate at start/end of one-byte TRUNCATE region, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 18)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 1);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - truncate one byte beyond end of one-byte TRUNCATE region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 19)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 1;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, 2);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - one R/W/T region, truncate in it, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 20)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags =
		    DM_REGION_READ | DM_REGION_WRITE | DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, DUMMY_STRLEN / 2);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != DUMMY_STRLEN / 2) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - three overlapping R/W/T regions, truncate in them, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 21)) {
		int fd;

		/* Variation set up */
		numRegions = 3;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_READ;
		Regions[1].rg_offset = 0;
		Regions[1].rg_size = DUMMY_STRLEN;
		Regions[1].rg_flags = DM_REGION_WRITE;
		Regions[2].rg_offset = 0;
		Regions[2].rg_size = DUMMY_STRLEN;
		Regions[2].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, DUMMY_STRLEN / 2);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != DUMMY_STRLEN / 2) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 0);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : ftruncate - one DM_REGION_NOEVENT region
	 * EXPECTED: no event
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 22)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = DUMMY_STRLEN;
		Regions[0].rg_flags = DM_REGION_NOEVENT;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else
		    if ((fd =
			 open(DummyFile, O_RDWR | O_CREAT,
			      DUMMY_FILE_RW_MODE)) == -1) {
			remove(DummyFile);
		}
		if (rc == -1 || fd == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_INVALID;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s)\n", DummyFile);
			rc = ftruncate(fd, DUMMY_STRLEN / 2);
			DMLOG_PRINT(DMLVL_DEBUG, "ftruncate(%s) returned %d\n",
				    DummyFile, rc);
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : open with O_TRUNC - one TRUNCATE region covering entire file, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 23)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		rc = system(command);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "open(%s, O_TRUNC)\n",
				    DummyFile);
			fd = open(DummyFile, O_RDWR | O_CREAT | O_TRUNC,
				  DUMMY_FILE_RW_MODE);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "open(%s, O_TRUNC) returned %d\n",
				    DummyFile, fd);
			rc = fd == -1 ? -1 : 0;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 5000);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : open with O_TRUNC - one-byte TRUNCATE region past EOF, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 24)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = TMP_FILELEN;
		Regions[0].rg_size = 1;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		rc = system(command);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "open(%s, O_TRUNC)\n",
				    DummyFile);
			fd = open(DummyFile, O_RDWR | O_CREAT | O_TRUNC,
				  DUMMY_FILE_RW_MODE);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "open(%s, O_TRUNC) returned %d\n",
				    DummyFile, fd);
			rc = fd == -1 ? -1 : 0;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 5000);
					varStatus = DMSTAT_FAIL;
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
		}
	}

	/*
	 * TEST    : creat - one TRUNCATE region covering entire file, DM_RESP_CONTINUE
	 * EXPECTED: DM_EVENT_TRUNCATE
	 */
	if (DMVAR_EXEC(FILE_TRUNC_DATA_EVENT_BASE + 25)) {
		int fd;

		/* Variation set up */
		numRegions = 1;
		Regions[0].rg_offset = 0;
		Regions[0].rg_size = 0;
		Regions[0].rg_flags = DM_REGION_TRUNCATE;

		sprintf(command, "cp %s %s", DUMMY_FILE, DummyFile);
		EVENT_DELIVERY_DELAY;
		rc = system(command);
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			EVENT_DELIVERY_DELAY_LOOP;
			eventExpected = DM_EVENT_TRUNCATE;
			eventReceived = DM_EVENT_INVALID;
			eventResponse = DM_RESP_CONTINUE;

			DMLOG_PRINT(DMLVL_DEBUG, "creat(%s)\n", DummyFile);
			fd = creat(DummyFile, S_IRWXU);
			DMLOG_PRINT(DMLVL_DEBUG, "creat(%s) returned %d\n",
				    DummyFile, fd);
			rc = fd == -1 ? -1 : 0;
			if ((varStatus =
			     DMVAR_CHKPASSEXP(0, rc, eventExpected,
					      eventReceived)) == DMSTAT_PASS) {
				rc = dm_handle_cmp(hanp1, hlen1, hanp2, hlen2);
				if (rc != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Handles NOT same!\n");
					varStatus = DMSTAT_FAIL;
				} else if (strcmp(name1, DUMMY_FILE) != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Entry name NOT correct! (%s vs %s)\n",
						    name1, DUMMY_FILE);
					varStatus = DMSTAT_FAIL;
				} else if (offset != 0) {
					DMLOG_PRINT(DMLVL_ERR,
						    "Offset NOT correct! (%d vs %d)\n",
						    offset, 5000);
					varStatus = DMSTAT_FAIL;
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
			DMEV_SET(DM_EVENT_POSTCREATE, events);
			DMEV_SET(DM_EVENT_READ, events);
			DMEV_SET(DM_EVENT_WRITE, events);
			DMEV_SET(DM_EVENT_TRUNCATE, events);
			rc = dm_set_disp(sid, lhanp, lhlen, token, &events,
					 DM_EVENT_MAX);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_ERR,
					    "dm_set_disp failed! (rc = %d, errno = %d)\n",
					    rc, errno);
				dm_destroy_session(sid);
				DM_EXIT();
			}

			DMEV_CLR(DM_EVENT_READ, events);
			DMEV_CLR(DM_EVENT_WRITE, events);
			DMEV_CLR(DM_EVENT_TRUNCATE, events);
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
		} else if (type == DM_EVENT_POSTCREATE) {
			/* SPECIAL CASE: need to set regions (if any) and response */
			dm_boolean_t exactflag;
			dm_namesp_event_t *nse =
			    DM_GET_VALUE(dmMsg, ev_data, dm_namesp_event_t *);
			int retcode = nse->ne_retcode;
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
				    retcode);

			if ((retcode == 0) && (numRegions > 0)) {
				rc = dm_set_region(sid, hanp2, hlen2,
						   DM_NO_TOKEN, numRegions,
						   Regions, &exactflag);
				if (rc == -1) {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_set_region failed! (rc = %d, errno = %d)\n",
						    rc, errno);
				} else {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "dm_set_regions succesfully set %d region(s)\n",
						    numRegions);
				}
			}

			/* No response needed */
			response = DM_RESP_INVALID;
		} else {
			eventReceived = type;
			response = eventResponse;

			switch (type) {
			case DM_EVENT_READ:
				{
					dm_data_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_data_event_t *);
					dm_timestruct_t delay;

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

					if (eventPending) {
						rc = dm_pending(sid, token,
								&delay);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "pending returned %d\n",
							    rc);
						EVENT_DELIVERY_DELAY;
						EVENT_DELIVERY_DELAY;
						EVENT_DELIVERY_DELAY;
					}

					break;
				}

			case DM_EVENT_WRITE:
				{
					dm_data_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_data_event_t *);
					dm_timestruct_t delay;

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

					if (eventPending) {
						rc = dm_pending(sid, token,
								&delay);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "pending returned %d\n",
							    rc);
						EVENT_DELIVERY_DELAY;
						EVENT_DELIVERY_DELAY;
						EVENT_DELIVERY_DELAY;
					}

					break;
				}

			case DM_EVENT_TRUNCATE:
				{
					dm_data_event_t *de =
					    DM_GET_VALUE(dmMsg, ev_data,
							 dm_data_event_t *);
					dm_timestruct_t delay;

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

					if (eventPending) {
						rc = dm_pending(sid, token,
								&delay);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "pending returned %d\n",
							    rc);
						EVENT_DELIVERY_DELAY;
						EVENT_DELIVERY_DELAY;
						EVENT_DELIVERY_DELAY;
					}

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
