/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	The function will mark for update the last data modification and last file
 *	status change timestamps of the file associated with the standard error
 *	stream at some time between its successful completion and exit( ),
 *	abort( ), or the completion of fflush( ) or fclose( ) on stderr.
 *
 * method:
 *	-Redirect error from stderr to a file using freopen.
 *	-Note down time stamps for data modification and file status change of the
 *	file to which we have redirected stderr.
 *	-Set errno to ERANGE.
 *	-Call perror().
 *	-Check time stamps for data modification and file status change of the
 *	file to which we have redirected stderr and it should be greater than
 *	previously noted values, that is before call of perror.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"
#include <features.h>
#include <wchar.h>
#include <time.h>
#include <sys/stat.h>

#define TNAME "perror/3-1.c"

int main(void)
{
	struct stat buf;
	time_t data_modification_freopen, data_modification_perror;
	time_t file_status_freopen, file_status_perror;
	FILE *fp;

	fp = freopen("perror.txt", "w+", stderr);
	if (fp == NULL) {
		printf(TNAME " Error at freopen(), errno: %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	stat("perror.txt", &buf);
	data_modification_freopen = buf.st_mtime;
	file_status_freopen = buf.st_ctime;
	sleep(2);

	errno = ERANGE;
	perror("strtol");
	stat("perror.txt", &buf);
	data_modification_perror = buf.st_mtime;
	file_status_perror = buf.st_ctime;

	if (data_modification_freopen < data_modification_perror &&
			file_status_freopen < file_status_perror) {
		printf(TNAME " Test Passed, last data modification and last file status"
				" change timestamps of the file associated with stderr stream"
				" are updated.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, failed to update last data modification and"
				" last file status change timestamps of the file associated"
				" with stderr stream.\n");
		exit(PTS_FAIL);
	}
}
