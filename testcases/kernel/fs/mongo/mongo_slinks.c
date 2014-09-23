//
//  A simple symlink test
//

#define _GNU_SOURCE

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

//
// Creates symlink [new-path] to [old-path], checks it,
// returnes  0 - if everything looks fine and
//  1 - otherwise.
// mongo_slinks reads arguments from stdin.

int main(int argc, char *argv[])
{
	char *old_path;
	char *new_path;

	struct stat statbuf;

	int num;
	char *buffer = NULL;
	char *line_buffer = NULL;
	size_t line_buffer_size = 0;
	int size = 1;

	if ((buffer = malloc(size + 1)) == NULL) {
		perror("checklink: malloc failed");
		return 1;
	}

	while (getline(&line_buffer, &line_buffer_size, stdin) != -1) {

		old_path = strtok(line_buffer, "\t ");
		new_path = strtok(NULL, "\t\n ");

		if (!old_path || !new_path)	/* empty lines at the end of file */
			break;

		// Create symlink
		if (symlink(old_path, new_path) != 0) {
			perror("checklink : symlink failed ");
			return 1;
		}
		// stat data of symlink itself
		if (lstat(new_path, &statbuf) == -1) {
			perror("checklink: lstat failed");
			return 1;
		}

		if (!(S_ISLNK(statbuf.st_mode))) {
			printf("checklink : file %s is not a symbol link\n",
			       new_path);
			return 1;
		}
		// Test readlink
		//
		// Increase size of buffer to readlink untile whole symlink body will be read.
		// Check readlink result on every iteration.

		while (1) {
			memset(buffer, 0, size + 1);
			num = readlink(new_path, buffer, size);
			if (num < 1 || num > size) {
				perror("checklink: readlink failed");
				free(buffer);
				return 1;
			}
			// Make sure that readlink did not break things
			if (buffer[num] != 0) {
				printf
				    ("checklink : readlink corrupts memory\n");
				free(buffer);
				return 1;
			}
			// Whole expected symlink body is read
			if (num < size)
				break;

			// Only part of symlink body was read. So we  make a bigger buffer
			// and call `readlink' again.
			size *= 2;
			if ((buffer = realloc(buffer, size + 1)) == NULL) {
				perror("checklink: realloc failed");
				return 1;
			}
		}
	}
	free(buffer);
	free(line_buffer);
	return 0;
}
