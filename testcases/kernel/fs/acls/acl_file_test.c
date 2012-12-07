#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/xattr.h>
#include <string.h>

int main(int argc, char *argv[])
{
	ssize_t s;
	char *tok;
	char value[1024];
	char list[1024];
	int rc = 0;
	char *file;
	int fd;

	if (argc < 2) {
		printf("Please enter a file name as argument.\n");
		return -1;
	}

	file = argv[1];

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf("Unable to open file %s !", file);
		return -1;
	}

	if (-1 == (s = flistxattr(fd, list, 1024))) {
		perror("flistxattr");
		return 1;
	}
	if (s == 0) {
		printf("No xattrs defined for %s, further testcase useless\n",
		       file);
		return 1;
	}
	tok = strtok(list, "\0");
	s = fgetxattr(fd, tok, (void *)value, 1024);
	if (s == -1) {
		perror("fgetxattr");
		return 1;
	}
	s = fsetxattr(fd, tok, (void *)value, s, 0);

	if (s == -1) {
		printf
		    ("User unable to change extended attributes on file %s !\n",
		     argv[1]);
		printf("errno = %i\n", errno);
		rc = 1;
	}
	//s = syscall(237, fd,tok); //fremovexattr
#ifdef __NR_fremovexattr
	s = syscall(__NR_fremovexattr, fd, tok);	//fremovexattr
#else
	s = -1;
	errno = ENOSYS;
#endif
	if (s == -1) {
		printf("User unable to remove extended attributes file %s !\n",
		       argv[1]);
		printf("errno = %i\n", errno);
		rc = 1;
	}

	close(fd);
	return rc;
}
