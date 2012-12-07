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
 * TEST CASE	: config.c
 *
 * VARIATIONS	: 28
 *
 * API'S TESTED	: dm_get_config
 */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "dm_test.h"

int main(int argc, char **argv)
{

	char *szFuncName;
	char *varstr;
	int rc;
	int i;

	DMOPT_PARSE(argc, argv);
	DMLOG_START();

	/* CANNOT DO ANYTHING WITHOUT SUCCESSFUL INITIALIZATION AND NO PREEXISTING FILES!!! */
	if ((rc = dm_init_service(&varstr)) != 0) {
		DMLOG_PRINT(DMLVL_ERR,
			    "dm_init_service failed! (rc = %d, errno = %d)\n",
			    rc, errno);
		DM_EXIT();
	}

	DMLOG_PRINT(DMLVL_DEBUG, "Starting DMAPI configuration tests\n");

	szFuncName = "dm_get_config";

	/*
	 * TEST    : dm_get_config - invalid hanp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 1)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hanp)\n",
				    szFuncName);
			rc = dm_get_config((void *)INVALID_ADDR, hlen,
					   DM_CONFIG_BULKALL, &retval);
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
	 * TEST    : dm_get_config - invalid hlen
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 2)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid hlen)\n",
				    szFuncName);
			rc = dm_get_config(hanp, INVALID_ADDR,
					   DM_CONFIG_BULKALL, &retval);
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
	 * TEST    : dm_get_config - invalid flagname
	 * EXPECTED: rc = -1, errno = EINVAL
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 3)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid flagname)\n",
				    szFuncName);
			rc = dm_get_config(hanp, hlen, INVALID_ADDR, &retval);
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
	 * TEST    : dm_get_config - invalid retvalp
	 * EXPECTED: rc = -1, errno = EFAULT
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 4)) {
		int fd;
		void *hanp;
		size_t hlen;

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalid retvalp)\n",
				    szFuncName);
			rc = dm_get_config(hanp, hlen, DM_CONFIG_BULKALL,
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
	 * TEST    : dm_get_config - global handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 5)) {
		dm_size_t retval;

		/* Variation set up */

		/* Variation */
		DMLOG_PRINT(DMLVL_DEBUG, "%s(global handle)\n", szFuncName);
		rc = dm_get_config(DM_GLOBAL_HANP, DM_GLOBAL_HLEN,
				   DM_CONFIG_BULKALL, &retval);
		DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

		/* Variation clean up */
	}

	/*
	 * TEST    : dm_get_config - file handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 6)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(file handle)\n",
				    szFuncName);
			rc = dm_get_config(hanp, hlen, DM_CONFIG_BULKALL,
					   &retval);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

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
	 * TEST    : dm_get_config - directory handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 7)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

		/* Variation set up */
		if ((rc = mkdir(DUMMY_SUBDIR, DUMMY_DIR_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_handle(DUMMY_SUBDIR, &hanp, &hlen))
			   == -1) {
			rmdir(DUMMY_SUBDIR);
		}
		if (fd == -1 || rc == -1) {
			DMLOG_PRINT(DMLVL_DEBUG,
				    "Unable to set up variation! (errno = %d)\n",
				    errno);
			DMVAR_SKIP();
		} else {
			/* Variation */
			DMLOG_PRINT(DMLVL_DEBUG, "%s(dir handle)\n",
				    szFuncName);
			rc = dm_get_config(hanp, hlen, DM_CONFIG_BULKALL,
					   &retval);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

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
	 * TEST    : dm_get_config - fs handle
	 * EXPECTED: rc = 0
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 8)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
		} else if ((rc = dm_path_to_fshandle(DUMMY_FILE, &hanp, &hlen))
			   == -1) {
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(fs handle)\n", szFuncName);
			rc = dm_get_config(hanp, hlen, DM_CONFIG_BULKALL,
					   &retval);
			DMVAR_ENDPASSEXP(szFuncName, 0, rc);

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
	 * TEST    : dm_get_config - invalidated handle
	 * EXPECTED: rc = -1, errno = EBADF
	 */
	if (DMVAR_EXEC(GET_CONFIG_BASE + 9)) {
		int fd;
		void *hanp;
		size_t hlen;
		dm_size_t retval;

		/* Variation set up */
		if ((fd =
		     open(DUMMY_FILE, O_RDWR | O_CREAT,
			  DUMMY_FILE_RW_MODE)) == -1) {
			/* No clean up */
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
			DMLOG_PRINT(DMLVL_DEBUG, "%s(invalidated handle)\n",
				    szFuncName);
			rc = dm_get_config(hanp, hlen, DM_CONFIG_BULKALL,
					   &retval);
			DMVAR_ENDFAILEXP(szFuncName, -1, rc, EBADF);

			/* Variation clean up */
			dm_handle_free(hanp, hlen);
		}
	}

	/*
	 * TEST    : dm_get_config
	 * EXPECTED: rc = 0
	 */
	for (i = 1; i < CONFIG_MAX; i++) {
		if (DMVAR_EXEC(GET_CONFIG_BASE + 9 + i)) {
			int fd;
			void *hanp;
			size_t hlen;
			dm_size_t retval;

			/* Variation set up */
			if ((fd =
			     open(DUMMY_FILE, O_RDWR | O_CREAT,
				  DUMMY_FILE_RW_MODE)) == -1) {
				/* No clean up */
			} else if ((rc = dm_fd_to_handle(fd, &hanp, &hlen)) ==
				   -1) {
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
				DMLOG_PRINT(DMLVL_DEBUG, "%s(%s)\n", szFuncName,
					    dmimpl_expectedResults[i].name);
				rc = dm_get_config(hanp, hlen, i, &retval);
				if (rc == 0) {
					if (retval ==
					    dmimpl_expectedResults[i].result) {
						DMLOG_PRINT(DMLVL_DEBUG,
							    "%s passed with expected rc = %d\n",
							    szFuncName, 0);
						DMVAR_PASS();
					} else {
						DMLOG_PRINT(DMLVL_ERR,
							    "%s failed with expected rc = %d but unexpected retval (%lld vs %lld)\n",
							    szFuncName, 0,
							    retval,
							    dmimpl_expectedResults
							    [i].result);
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
	}

	DMLOG_STOP();

	tst_exit();

}
