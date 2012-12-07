/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 *   Date:  31/05/2011
 *
 *   Test assertion 4 - EBUSY is returned for re-initializing a
 *   condition variable.  Note the 0 (zero) may be returned
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <posixtest.h>

#define ERR_MSG(f, rc)	printf("Failed: function: %s status: %s(%u)\n", \
						f, strerror(rc), rc)

int main(void)
{
	int status;
	pthread_cond_t cond;
	int rc;

	status = PTS_UNRESOLVED;

	rc = pthread_cond_init(&cond, NULL);
	if (rc) {
		ERR_MSG("pthread_cond_init()", rc);
		return status;
	}

	rc = pthread_cond_init(&cond, NULL);
	if (rc && rc != EBUSY) {
		ERR_MSG("pthread_cond_init() 2", rc);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
