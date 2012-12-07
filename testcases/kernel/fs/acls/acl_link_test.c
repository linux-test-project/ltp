#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/xattr.h>

int main(int argc, char *argv[])
{
	ssize_t s;
	char *tok;
	char value[1024];
	char list[1024];
	int rc = 0;

	if (argc < 2) {
		printf("Please enter a file name as argument.\n");
		return -1;
	}

	if (-1 == (s = listxattr(argv[1], list, 1024))) {
		perror("listxattr");
		return 1;
	}
	if (s == 0) {
		printf("No xattrs defined for %s, further testcase useless\n",
		       argv[1]);
		return 1;
	}
	tok = strtok(list, "\0");
	s = getxattr(argv[1], tok, (void *)value, 1024);
	if (-1 == s) {
		perror("getxattr");
		return -1;
	}

	s = lsetxattr(argv[1], tok, (void *)value, s, 0);

	if (s == -1) {
		printf("User unable to change extended attributes %s !\n",
		       argv[1]);
		printf("errno = %i\n", errno);
		rc = 1;
	}

	s = lremovexattr(argv[1], tok);
	if (s == -1) {
		printf("User unable to remove extended attributes %s !\n",
		       argv[1]);
		printf("errno = %i\n", errno);
		rc = 1;
	}

	return rc;
}
