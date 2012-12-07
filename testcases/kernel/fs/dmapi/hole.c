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
 * TEST CASE	: hole.c
 *
 * VARIATIONS	: 77
 *
 * API'S TESTED	: dm_get_allocinfo
 * 		  dm_probe_hole
 * 		  dm_punch_hole
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "dm_test.h"

#define TMP_FILELEN 500000
#define NUM_EXTENTS 8

char command[4096];
dm_sessid_t sid;
dm_extent_t Extents[NUM_EXTENTS];
dm_extent_t bigExtents[20];

void LogExtents(dm_extent_t * pext, u_int nelem)
{

	int i;

	DMLOG_PRINT(DMLVL_DEBUG, "Extents:\n");
	for (i = 0; i < nelem; i++, pext++) {
		DMLOG_PRINT(DMLVL_DEBUG,
			    "  extent %d: type %d, offset %lld, length %lld\n",
			    i + 1, pext->ex_type, pext->ex_offset,
			    pext->ex_length);
	}

}

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int i;
	int rc;
	char *szSessionInfo = "dm_test session info";

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	/* CANNOT DO ANYTHING WITHOUT SUCCESSFUL INITIALIZATION!!! */
	if ((rc = dm_init_service(&varstr)) != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_init_service failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else if ((rc = dm_create_session(DM_NO_SESSION, szSessionInfo, &sid))
		   != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_create_session failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	} else {
		int fd;

		fd = open(DUMMY_TMP, O_RDWR | O_CREAT | O_TRUNC,
			  DUMMY_FILE_RW_MODE);
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

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI file hole tests\n");

	szFuncName = "dm_get_allocinfo";

	/*
	 * TEST    : dm_get_allocinfo - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_get_allocinfo(INVALID_ADDR, hanp, hlen,
					      DM_NO_TOKEN, &off, NUM_EXTENTS,
					      Extents, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_get_allocinfo(sid, (void *)INVALID_ADDR, hlen,
					      DM_NO_TOKEN, &off, NUM_EXTENTS,
					      Extents, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 *
	 * This variation uncovered XFS BUG #7 (EOPNOTSUPP errno returned
	 * instead of EINVAL)
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
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
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_get_allocinfo(sid, hanp, INVALID_ADDR,
					      DM_NO_TOKEN, &off, NUM_EXTENTS,
					      Extents, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_get_allocinfo(sid, hanp, hlen, INVALID_ADDR,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - invalid offp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid offp)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      (dm_off_t *) INVALID_ADDR,
					      NUM_EXTENTS, Extents, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - unaligned offp
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = UNALIGNED_BLK_OFF;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(unaligned offp)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - off past EOF
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = TMP_FILELEN + 1;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(off past EOF)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - zero nelem
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(nelem zero)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, 0, Extents, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - invalid extentp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid extentp)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS,
					      (dm_extent_t *) INVALID_ADDR,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - invalid nelemp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid nelemp)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      (u_int *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - zero-length file
	 * EXPECTED: rc = 0, nelem = 0
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(zero-length file)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "dm_get_allocinfo returned %d\n", rc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);
				if (nelem == 0) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					DMVAR_PASS();
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 1);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - file all resident
	 * EXPECTED: rc = 0, nelem = 1
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 13)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file all resident)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "dm_get_allocinfo returned %d\n", rc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);
				if (nelem == 1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					LogExtents(Extents, nelem);
					if ((Extents[0].ex_length ==
					     TMP_FILELEN)
					    && (Extents[0].ex_offset == 0)
					    && (Extents[0].ex_type ==
						DM_EXTENT_RES)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s extent information correct\n",
							    szFuncName);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s extent information NOT correct!\n",
							    szFuncName);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 1);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - file all hole
	 * EXPECTED: rc = 0, nelem = 1
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 14)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if (((off =
			  lseek(fd, TMP_FILELEN - DUMMY_STRLEN,
				SEEK_SET)) != TMP_FILELEN - DUMMY_STRLEN)
			||
			((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN) ? -1 : 0) == -1)
			||
			((rc =
			  ftruncate(fd,
				    ((TMP_FILELEN / 2) & (~(BLK_SIZE - 1))))) ==
			 -1)
			|| ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1)) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || off != (TMP_FILELEN - DUMMY_STRLEN) || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			off = 0;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file all hole)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG,
				    "dm_get_allocinfo returned %d\n", rc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);
				if (nelem == 1) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					LogExtents(Extents, nelem);
					if ((Extents[0].ex_length ==
					     ((TMP_FILELEN /
					       2) & (~(BLK_SIZE - 1))))
					    && (Extents[0].ex_offset == 0)
					    && (Extents[0].ex_type ==
						DM_EXTENT_HOLE)) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s extent information correct\n",
							    szFuncName);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s extent information NOT correct!\n",
							    szFuncName);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 1);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - hole, then resident
	 * EXPECTED: rc = 0, nelem = 2
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 15)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if (((off =
			  lseek(fd, TMP_FILELEN - DUMMY_STRLEN,
				SEEK_SET)) != TMP_FILELEN - DUMMY_STRLEN)
			||
			((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN) ? -1 : 0) == -1)
			|| ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1)) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || off != (TMP_FILELEN - DUMMY_STRLEN) || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			off = 0;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(hole, resident)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);
				if (nelem == 2) {
					int i;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					LogExtents(Extents, nelem);
					if ((i = 1)
					    && (Extents[0].ex_length +
						Extents[1].ex_length ==
						TMP_FILELEN) && (i = 2)
					    && (Extents[0].ex_offset == 0)
					    && (i = 3)
					    && (Extents[0].ex_length ==
						Extents[1].ex_offset) && (i = 4)
					    && (Extents[0].ex_type ==
						DM_EXTENT_HOLE) && (i = 5)
					    && (Extents[1].ex_type ==
						DM_EXTENT_RES)
					    ) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s extent information correct\n",
							    szFuncName);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s extent information NOT correct! (test %d failed)\n",
							    szFuncName, i);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 2);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - resident, then hole
	 * EXPECTED: rc = 0, nelem = 2
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 16)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if (((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN) ? -1 : 0) == -1)
			||
			((off =
			  lseek(fd, TMP_FILELEN - DUMMY_STRLEN,
				SEEK_SET)) != TMP_FILELEN - DUMMY_STRLEN)
			||
			((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN) ? -1 : 0) == -1)
			||
			((rc =
			  ftruncate(fd,
				    ((TMP_FILELEN / 2) & (~(BLK_SIZE - 1))))) ==
			 -1)
			|| ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1)) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || off != (TMP_FILELEN - DUMMY_STRLEN) || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			off = 0;
			DMLOG_PRINT(DMLVL_DEBUG, "%s(resident, hole)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);
				if (nelem == 2) {
					int i;

					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					LogExtents(Extents, nelem);
					if ((i = 1)
					    && (Extents[0].ex_length +
						Extents[1].ex_length ==
						((TMP_FILELEN /
						  2) & (~(BLK_SIZE - 1))))
					    && (i = 2)
					    && (Extents[0].ex_offset == 0)
					    && (i = 3)
					    && (Extents[0].ex_length ==
						Extents[1].ex_offset) && (i = 4)
					    && (Extents[0].ex_type ==
						DM_EXTENT_RES) && (i = 5)
					    && (Extents[1].ex_type ==
						DM_EXTENT_HOLE)
					    ) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s extent information correct\n",
							    szFuncName);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s extent information NOT correct! (test %d failed)\n",
							    szFuncName, i);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 2);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - resident, then hole, then resident
	 * EXPECTED: rc = 0, nelem = 3
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 17)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if (((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN) ? -1 : 0) == -1)
			||
			((off =
			  lseek(fd, TMP_FILELEN - DUMMY_STRLEN,
				SEEK_SET)) != TMP_FILELEN - DUMMY_STRLEN)
			||
			((rc =
			  (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			   DUMMY_STRLEN) ? -1 : 0) == -1)
			|| ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1)) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || off != (TMP_FILELEN - DUMMY_STRLEN) || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			off = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(resident, hole, resident)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				int i;
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);

				if (nelem == 3) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					LogExtents(Extents, nelem);
					if ((i = 1)
					    && (Extents[0].ex_length +
						Extents[1].ex_length +
						Extents[2].ex_length ==
						TMP_FILELEN) && (i = 2)
					    && (Extents[0].ex_offset == 0)
					    && (i = 3)
					    && (Extents[0].ex_length ==
						Extents[1].ex_offset) && (i = 4)
					    && (Extents[1].ex_length +
						Extents[1].ex_offset ==
						Extents[2].ex_offset) && (i = 5)
					    && (Extents[0].ex_type ==
						DM_EXTENT_RES) && (i = 6)
					    && (Extents[1].ex_type ==
						DM_EXTENT_HOLE) && (i = 7)
					    && (Extents[2].ex_type ==
						DM_EXTENT_RES)
					    ) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s extent information correct\n",
							    szFuncName);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s extent information NOT correct! (test %d failed)\n",
							    szFuncName, i);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 3);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - resident, then hole, then resident, then hole, etc.
	 * EXPECTED: rc = 1, nelem = 8
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 18)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;
		int i;

		/* Variation set up */
		fd = open(DUMMY_FILE, O_RDWR | O_CREAT, DUMMY_FILE_RW_MODE);
		for (i = 0, rc = 0; rc == 0 && i < TMP_FILELEN;
		     i += TMP_FILELEN / (NUM_EXTENTS + 2)) {
			if ((rc =
			     (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
			      DUMMY_STRLEN) ? -1 : 0) != -1) {
				if ((off = lseek(fd, i, SEEK_SET)) != off) {
					rc = -1;
				}
			}
		}
		if ((rc == -1) ||
		    ((off =
		      lseek(fd, TMP_FILELEN - DUMMY_STRLEN, SEEK_SET)) != off)
		    ||
		    ((rc =
		      (write(fd, DUMMY_STRING, DUMMY_STRLEN) !=
		       DUMMY_STRLEN) ? -1 : 0) == -1)
		    || ((rc = dm_fd_to_handle(fd, &hanp, &hlen)))) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || off != TMP_FILELEN - DUMMY_STRLEN || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			off = 0;
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(resident, hole, resident, hole, etc.)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 1) {
				int i;
				DMLOG_PRINT(DMLVL_DEBUG, "  off = %lld\n", off);

				if (nelem == NUM_EXTENTS) {
					DMLOG_PRINT(DMLVL_DEBUG,
						    "  nelem = %d\n", nelem);
					LogExtents(Extents, nelem);
					if ((i = 1)
					    && (Extents[0].ex_offset == 0)
					    && (i = 2)
					    && (Extents[0].ex_length ==
						Extents[1].ex_offset) && (i = 3)
					    && (Extents[1].ex_length +
						Extents[1].ex_offset ==
						Extents[2].ex_offset) && (i = 4)
					    && (Extents[2].ex_length +
						Extents[2].ex_offset ==
						Extents[3].ex_offset) && (i = 5)
					    && (Extents[3].ex_length +
						Extents[3].ex_offset ==
						Extents[4].ex_offset) && (i = 6)
					    && (Extents[4].ex_length +
						Extents[4].ex_offset ==
						Extents[5].ex_offset) && (i = 7)
					    && (Extents[5].ex_length +
						Extents[5].ex_offset ==
						Extents[6].ex_offset) && (i = 8)
					    && (Extents[6].ex_length +
						Extents[6].ex_offset ==
						Extents[7].ex_offset) && (i = 9)
					    && (Extents[7].ex_length +
						Extents[7].ex_offset == off)
					    && (i = 10)
					    && (Extents[0].ex_type ==
						DM_EXTENT_RES) && (i = 11)
					    && (Extents[1].ex_type ==
						DM_EXTENT_HOLE) && (i = 12)
					    && (Extents[2].ex_type ==
						DM_EXTENT_RES) && (i = 13)
					    && (Extents[3].ex_type ==
						DM_EXTENT_HOLE) && (i = 14)
					    && (Extents[4].ex_type ==
						DM_EXTENT_RES) && (i = 15)
					    && (Extents[5].ex_type ==
						DM_EXTENT_HOLE) && (i = 16)
					    && (Extents[6].ex_type ==
						DM_EXTENT_RES) && (i = 17)
					    && (Extents[7].ex_type ==
						DM_EXTENT_HOLE)
					    ) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s extent information correct\n",
							    szFuncName);

						rc = dm_get_allocinfo(sid, hanp,
								      hlen,
								      DM_NO_TOKEN,
								      &off,
								      sizeof
								      (bigExtents)
								      /
								      sizeof
								      (dm_extent_t),
								      bigExtents,
								      &nelem);
						DMLOG_PRINT(DMLVL_DEBUG,
							    "second %s returned %d\n",
							    szFuncName, rc);
						if (rc == 0) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "  nelem = %d\n",
								    nelem);
							LogExtents(bigExtents,
								   nelem);
							if (bigExtents
							    [nelem -
							     1].ex_offset +
							    bigExtents[nelem -
								       1].
							    ex_length ==
							    TMP_FILELEN) {
								DMLOG_PRINT
								    (DMLVL_DEBUG,
								     "second %s extent information correct\n",
								     szFuncName);
								DMVAR_PASS();
							} else {
								DMLOG_PRINT
								    (DMLVL_ERR,
								     "second %s extent information NOT correct!\n",
								     szFuncName);
								DMVAR_FAIL();
							}
						} else {
							DMLOG_PRINT(DMLVL_ERR,
								    "second %s failed with unexpected rc = %d (errno = %d)\n",
								    szFuncName,
								    rc, errno);
							DMVAR_FAIL();
						}
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s extent information NOT correct! (test %d failed)\n",
							    szFuncName, i);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s nelem NOT correct! (%d vs %d)\n",
						    szFuncName, nelem, 3);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 19)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_get_allocinfo(DM_NO_SESSION, hanp, hlen,
					      DM_NO_TOKEN, &off, NUM_EXTENTS,
					      Extents, &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 20)) {
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_SUBDIR, &hanp,
					     &hlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_allocinfo - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 21)) {
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_get_allocinfo(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				      DM_NO_TOKEN, &off, NUM_EXTENTS, Extents,
				      &nelem);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_allocinfo - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_ALLOCINFO_BASE + 22)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_get_allocinfo(sid, hanp, hlen, DM_NO_TOKEN,
					      &off, NUM_EXTENTS, Extents,
					      &nelem);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_probe_hole";

	/*
	 * TEST    : dm_probe_hole - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_probe_hole(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, inoff, inlen, &outoff,
					   &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_probe_hole(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, inoff, inlen, &outoff,
					   &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
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
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_probe_hole(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   inoff, inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_probe_hole(sid, hanp, hlen, INVALID_ADDR, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid off
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = TMP_FILELEN + 1, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid off)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid len
	 * EXPECTED: rc = -1, errno = E2BIG
	 *
	 * This variation uncovered XFS BUG #8 (0 returned instead of -1 and
	 * errno E2BIG)
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = TMP_FILELEN + 1, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid len)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid roffp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid roffp)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, (dm_off_t *) INVALID_ADDR,
					   &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - invalid rlenp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff,
					   (dm_size_t *) INVALID_ADDR);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - entire file
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(entire file)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "In -> offset %lld, length %lld\n",
					    inoff, inlen);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Out <- offset %lld, length %lld\n",
					    outoff, outlen);
				if (outoff == inoff) {
					if (outlen == inlen) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc %d\n",
							    szFuncName, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc %d but unexpected outlen (%d vs %d)\n",
							    szFuncName, rc,
							    outlen, inlen);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc %d but unexpected outoff (%d vs %d)\n",
						    szFuncName, rc, outoff,
						    inoff);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - end of file without rounding
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = BLKALIGN(UNALIGNED_BLK_OFF), outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(end of file without rounding)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "In -> offset %lld, length %lld\n",
					    inoff, inlen);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Out <- offset %lld, length %lld\n",
					    outoff, outlen);
				if (outoff == inoff) {
					if (outlen == inlen) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc %d\n",
							    szFuncName, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc %d but unexpected outlen (%d vs %d)\n",
							    szFuncName, rc,
							    outlen, inlen);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc %d but unexpected outoff (%d vs %d)\n",
						    szFuncName, rc, outoff,
						    inoff);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - end of file with rounding
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = UNALIGNED_BLK_OFF, outoff;
		dm_size_t inlen = TMP_FILELEN - UNALIGNED_BLK_OFF, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(end of file with rounding)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "In -> offset %lld, length %lld\n",
					    inoff, inlen);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Out <- offset %lld, length %lld\n",
					    outoff, outlen);
				if ((outoff >= inoff)
				    && (!(outoff & (BLK_SIZE - 1)))) {
					if (outlen == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc %d\n",
							    szFuncName, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc %d but unexpected outlen (%d vs %d)\n",
							    szFuncName, rc,
							    outlen, 0);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc %d but unexpected outoff %d\n",
						    szFuncName, rc, outoff);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - middle of file without rounding
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 13)) {
#ifdef INTERIOR_HOLES
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = BLKALIGN(UNALIGNED_BLK_OFF), outoff;
		dm_size_t inlen =
		    BLKALIGN(TMP_FILELEN - BLK_SIZE - UNALIGNED_BLK_OFF),
		    outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(middle of file without rounding)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "In -> offset %lld, length %lld\n",
					    inoff, inlen);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Out <- offset %lld, length %lld\n",
					    outoff, outlen);
				if (outoff == inoff) {
					if (outlen == inlen) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc %d\n",
							    szFuncName, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc %d but unexpected outlen (%d vs %d)\n",
							    szFuncName, rc,
							    outlen, inlen);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc %d but unexpected outoff (%d vs %d)\n",
						    szFuncName, rc, outoff,
						    inoff);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with INTERIOR_HOLES defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_probe_hole - middle of file with rounding, large
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 14)) {
#ifdef INTERIOR_HOLES
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = UNALIGNED_BLK_OFF, outoff;
		dm_size_t inlen =
		    TMP_FILELEN - BLK_SIZE - UNALIGNED_BLK_OFF, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(middle of file with rounding, large)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "In -> offset %lld, length %lld\n",
					    inoff, inlen);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Out <- offset %lld, length %lld\n",
					    outoff, outlen);
				if ((outoff >= inoff)
				    && (!(outoff & (BLK_SIZE - 1)))) {
					if ((outlen <= inlen)
					    && (!(outlen & (BLK_SIZE - 1)))) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc %d\n",
							    szFuncName, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc %d but unexpected outlen %d\n",
							    szFuncName, rc,
							    outlen);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc %d but unexpected outoff %d\n",
						    szFuncName, rc, outoff);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with INTERIOR_HOLES defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_probe_hole - middle of file with rounding, small
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 15)) {
#ifdef INTERIOR_HOLES
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = TMP_FILELEN / 2 - BLK_SIZE, outoff;
		dm_size_t inlen = 5 * BLK_SIZE, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(middle of file with rounding, small)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			if (rc == 0) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "In -> offset %lld, length %lld\n",
					    inoff, inlen);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Out <- offset %lld, length %lld\n",
					    outoff, outlen);
				if ((outoff >= inoff)
				    && (!(outoff & (BLK_SIZE - 1)))) {
					if ((outlen <= inlen)
					    && (!(outlen & (BLK_SIZE - 1)))) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc %d\n",
							    szFuncName, rc);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc %d but unexpected outlen %d\n",
							    szFuncName, rc,
							    outlen);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "%s failed with expected rc %d but unexpected outoff %d\n",
						    szFuncName, rc, outoff);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with INTERIOR_HOLES defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_probe_hole - middle of file with rounding, no hole
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 16)) {
#ifdef INTERIOR_HOLES
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff =
		    ((TMP_FILELEN / 2) & ~(BLK_SIZE - 1)) + 1, outoff;
		dm_size_t inlen = BLK_SIZE, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(middle of file with rounding, no hole)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with INTERIOR_HOLES defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_probe_hole - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 17)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_probe_hole(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, inoff, inlen, &outoff,
					   &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 18)) {
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_SUBDIR, &hanp,
					     &hlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_probe_hole - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 19)) {
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_probe_hole(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_NO_TOKEN, inoff, inlen, &outoff, &outlen);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_probe_hole - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(PROBE_HOLE_BASE + 20)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t inoff = 0, outoff;
		dm_size_t inlen = 0, outlen;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_probe_hole(sid, hanp, hlen, DM_NO_TOKEN, inoff,
					   inlen, &outoff, &outlen);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	szFuncName = "dm_punch_hole";

	/*
	 * TEST    : dm_punch_hole - invalid sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_punch_hole(INVALID_ADDR, hanp, hlen,
					   DM_NO_TOKEN, off, len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_punch_hole(sid, (void *)INVALID_ADDR, hlen,
					   DM_NO_TOKEN, off, len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EFAULT);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - directory handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 3)) {
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
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
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_punch_hole(sid, hanp, INVALID_ADDR, DM_NO_TOKEN,
					   off, len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - invalid token
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 5)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_punch_hole(sid, hanp, hlen, INVALID_ADDR, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - invalid off
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = TMP_FILELEN + 1;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid off)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - invalid len
	 * EXPECTED: rc = -1, errno = E2BIG
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = TMP_FILELEN + 1;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid len)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, E2BIG);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - unaligned off
	 * EXPECTED: rc = -1, errno = EAGAIN
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 1;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(unaligned off)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EAGAIN);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - unaligned len
	 * EXPECTED: rc = -1, errno = EAGAIN
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 1;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(unaligned len)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EAGAIN);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - unaligned off and len
	 * EXPECTED: rc = -1, errno = EAGAIN
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 10)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = UNALIGNED_BLK_OFF;
		dm_size_t len = TMP_FILELEN - UNALIGNED_BLK_OFF;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(unaligned off and len)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EAGAIN);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - truncate entire file
	 * EXPECTED: rc = 0, nelem = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 11)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(truncate entire file)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				off = 0;
				rc = dm_get_allocinfo(sid, hanp, hlen,
						      DM_NO_TOKEN, &off,
						      NUM_EXTENTS, Extents,
						      &nelem);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "dm_get_allocinfo returned %d\n",
					    rc);
				if (rc == 0) {
					if (nelem == 0) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "  nelem = %d\n",
							    nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s nelem NOT correct! (%d vs %d)\n",
							    szFuncName, nelem,
							    0);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_get_allocinfo failed with unexpected rc = %d (errno = %d)\n",
						    rc, errno);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - truncate part of file, len 0
	 * EXPECTED: rc = 0, nelem = 1
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 12)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLK_SIZE;
		dm_size_t len = 0;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(truncate part of file, len 0)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				off = 0;
				rc = dm_get_allocinfo(sid, hanp, hlen,
						      DM_NO_TOKEN, &off,
						      NUM_EXTENTS, Extents,
						      &nelem);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "dm_get_allocinfo returned %d\n",
					    rc);
				if (rc == 0) {
					if (nelem == 1) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "  nelem = %d\n",
							    nelem);
						LogExtents(Extents, nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s nelem NOT correct! (%d vs %d)\n",
							    szFuncName, nelem,
							    1);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_get_allocinfo failed with unexpected rc = %d (errno = %d)\n",
						    rc, errno);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - truncate part of file, len non-zero
	 * EXPECTED: rc = 0, nelem = 1
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 13)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLK_SIZE;
		dm_size_t len = TMP_FILELEN - BLK_SIZE;
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(truncate part of file, len non-0)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				off = 0;
				rc = dm_get_allocinfo(sid, hanp, hlen,
						      DM_NO_TOKEN, &off,
						      NUM_EXTENTS, Extents,
						      &nelem);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "dm_get_allocinfo returned %d\n",
					    rc);
				if (rc == 0) {
					if (nelem == 1) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "  nelem = %d\n",
							    nelem);
						LogExtents(Extents, nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s nelem NOT correct! (%d vs %d)\n",
							    szFuncName, nelem,
							    1);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_get_allocinfo failed with unexpected rc = %d (errno = %d)\n",
						    rc, errno);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - small hole
	 * EXPECTED: rc = 0, nelem = 3
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 14)) {
#ifdef INTERIOR_HOLES
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = (TMP_FILELEN / 2) & (~(BLK_SIZE - 1));
		dm_size_t len = 2 * BLK_SIZE;
		u_int nelem;
		char buf[DUMMY_STRLEN];

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(small hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				off = 0;
				rc = dm_get_allocinfo(sid, hanp, hlen,
						      DM_NO_TOKEN, &off,
						      NUM_EXTENTS, Extents,
						      &nelem);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "dm_get_allocinfo returned %d\n",
					    rc);
				if (rc == 0) {
					if (nelem == 3) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "  nelem = %d\n",
							    nelem);
						LogExtents(Extents, nelem);
						if ((lseek
						     (fd, TMP_FILELEN / 2,
						      SEEK_SET) ==
						     (TMP_FILELEN / 2))
						    &&
						    (read(fd, buf, DUMMY_STRLEN)
						     == DUMMY_STRLEN)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "buffer from hole: [%s]\n",
								    buf);
						}
						if ((lseek
						     (fd, TMP_FILELEN - 10,
						      SEEK_SET) ==
						     (TMP_FILELEN - 10))
						    &&
						    (read(fd, buf, DUMMY_STRLEN)
						     == DUMMY_STRLEN)) {
							DMLOG_PRINT(DMLVL_DEBUG,
								    "buffer from resident extent: [%s]\n",
								    buf);
						}
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s nelem NOT correct! (%d vs %d)\n",
							    szFuncName, nelem,
							    3);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_get_allocinfo failed with unexpected rc = %d (errno = %d)\n",
						    rc, errno);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with INTERIOR_HOLES defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_punch_hole - large hole
	 * EXPECTED: rc = 0, nelem = 3
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 15)) {
#ifdef INTERIOR_HOLES
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLK_SIZE;
		dm_size_t len =
		    (TMP_FILELEN - (2 * BLK_SIZE)) & (~(BLK_SIZE - 1));
		u_int nelem;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(large hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMLOG_PRINT(DMLVL_DEBUG, "%s returned %d\n", szFuncName,
				    rc);
			if (rc == 0) {
				off = 0;
				rc = dm_get_allocinfo(sid, hanp, hlen,
						      DM_NO_TOKEN, &off,
						      NUM_EXTENTS, Extents,
						      &nelem);
				DMLOG_PRINT(DMLVL_DEBUG,
					    "dm_get_allocinfo returned %d\n",
					    rc);
				if (rc == 0) {
					if (nelem == 3) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "  nelem = %d\n",
							    nelem);
						LogExtents(Extents, nelem);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s nelem NOT correct! (%d vs %d)\n",
							    szFuncName, nelem,
							    3);
						DMVAR_FAIL();
					}
				} else {
					DMLOG_PRINT(DMLVL_ERR,
						    "dm_get_allocinfo failed with unexpected rc = %d (errno = %d)\n",
						    rc, errno);
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
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
#else
		DMLOG_PRINT(DMLVL_WARN,
			    "Test case not built with INTERIOR_HOLES defined\n");
		DMVAR_SKIP();
#endif
	}

	/*
	 * TEST    : dm_punch_hole - DM_NO_SESSION sid
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 16)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
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
			rc = dm_punch_hole(DM_NO_SESSION, hanp, hlen,
					   DM_NO_TOKEN, off, len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - fs handle
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 17)) {
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else
		    if ((rc =
			 dm_path_to_fshandle(DUMMY_SUBDIR, &hanp,
					     &hlen)) == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EINVAL);

			/* Variation clean up */
			rc = rmdir(DUMMY_SUBDIR);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 18)) {
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_punch_hole(sid, DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_NO_TOKEN, off, len);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_punch_hole - invalidated hanp
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 19)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else if ((rc = close(fd)) == -1) {
			dm_handle_free(hanp, hlen);
			remove(DUMMY_FILE);
		} else if ((rc = remove(DUMMY_FILE)) == -1) {
			dm_handle_free(hanp, hlen);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated hanp)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private read mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 20)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private read mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private write mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 21)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private write mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private exec mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 22)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private exec mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private r/w mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 23)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private r/w mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared read mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 24)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared read mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared write mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 25)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared write mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared exec mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 26)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared exec mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared r/w mmap overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 27)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = 0;
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_SHARED, fd, 0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared r/w mmap overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private read mmap not overlapping hole
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 28)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private read mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private write mmap not overlapping hole
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 29)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private write mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private exec mmap not overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 30)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_PRIVATE, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private exec mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - private r/w mmap not overlapping hole
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 31)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(private r/w mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared read mmap not overlapping hole
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 32)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared read mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared write mmap not overlapping hole
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 33)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared write mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared exec mmap not overlapping hole
	 * EXPECTED: rc = -1, errno = EBUSY
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 34)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_EXEC, MAP_SHARED, fd,
			      0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared exec mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBUSY);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_punch_hole - shared r/w mmap not overlapping hole
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(PUNCH_HOLE_BASE + 35)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_off_t off = BLKALIGN(TMP_FILELEN / 2);
		dm_size_t len = 0;
		void *memmap;

		/* Variation set up */
		sprintf(command, "cp %s %s", DUMMY_TMP, DUMMY_FILE);
		if ((rc = system(command)) == -1) {
			/* No clean up */
		} else if ((fd = open(DUMMY_FILE, O_RDWR)) == -1) {
			remove(DUMMY_FILE);
		} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) == -1) {
			close(fd);
			remove(DUMMY_FILE);
		} else
		    if ((memmap =
			 mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			      MAP_SHARED, fd, 0)) == MAP_FAILED) {
			dm_handle_free(hanp, hlen);
			close(fd);
			remove(DUMMY_FILE);
		}
		if (fd == -1 || rc == -1 || memmap == MAP_FAILED) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG,
				    "%s(shared r/w mmap not overlap hole)\n",
				    szFuncName);
			rc = dm_punch_hole(sid, hanp, hlen, DM_NO_TOKEN, off,
					   len);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

			/* Variation clean up */
			munmap(memmap, PAGE_SIZE);
			rc = close(fd);
			rc |= remove(DUMMY_FILE);
			if (rc == -1) {
				DMLOG_PRINT(DMLVL_DEBUG,
					    "Unable to clean up variation! (errno = %d)\n",
					    errno);
			}
			dm_handle_free(hanp, hlen);
		}
	}

	rc = dm_destroy_session(sid);
	if (rc == -1) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_destroy_session failed! (rc = %d, errno = %d)\n",
			    rc, errno);
	}

	remove(DUMMY_TMP);

	DMLOG_STOP();

	tst_exit();
}
