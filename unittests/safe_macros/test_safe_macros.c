#include "test.h"
#include "safe_macros.h"

char *TCID = "test_safe_macros";
int TST_TOTAL = 1;

int
main(int argc, char **argv)
{
	char buf[10];
	int fd, fds[2];

	buf[9] = '\0';

	system("cp " __FILE__ " " __FILE__ "~");
	printf("%s\n", SAFE_BASENAME(NULL, *argv));
	printf("%s\n", SAFE_DIRNAME(NULL, *argv));
	fd = SAFE_OPEN(NULL, __FILE__ "~", O_RDWR);
	SAFE_READ(NULL, 0, fd, buf, 9);
	printf("buf: %s\n", buf);
	SAFE_READ(NULL, 1, fd, buf, 9);
	printf("buf: %s\n", buf);
	SAFE_WRITE(NULL, 0, fd, buf, 9);
	SAFE_WRITE(NULL, 1, fd, buf, 9);
	SAFE_CLOSE(NULL, fd);
	SAFE_PIPE(NULL, fds);
	SAFE_UNLINK(NULL, __FILE__ "~");

	return 0;
}
