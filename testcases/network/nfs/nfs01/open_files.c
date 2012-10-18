#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define TEMPLATE "ltpXXXXXX"

int write_something(int);
void delete_files(void);
void abortx(char *fmt, ...);

int main(argc,argv)
int argc;
char *argv[];
{
    int filedes[25500];
    int i, n, first, n_files;
    int cid, fork_number;
    int status;
    char filename[PATH_MAX];

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <number of files>\n", argv[0]);
	exit(1);
    }

    n = sscanf(argv[1], "%d", &n_files);
    if (n != 1) {
	fprintf(stderr, "Usage: %s <number of files>\n", argv[0]);
	exit(1);
    }

    first = 0;
    fork_number = 0;
    for (n=0;  n < n_files; n++) {
	strcpy(filename, TEMPLATE);
	filedes[n] = mkstemp(filename);
	if (filedes[n] == -1) {
	    if (errno != EMFILE)
		abortx("open() error: file = \"%s\", errno = %d",
		      filename, errno);
	    else {
		if ((cid = fork())) {
		    if (cid == -1)
			abortx("Error forking child");
		    else {
			waitpid(cid, &status, 0);
			for (i=first; i < n; i++)
			    if (!write_something(filedes[i]))
				abortx("Error writing to files");
			if (fork_number == 0)
			    delete_files();
			exit(WEXITSTATUS(status));
		    }
		}
		else {
		    fork_number++;
		    for (i=first ; i < n; i++)
			close(filedes[i]);
		    first = n;
		    n--;
		}
	    }
	}
    }

    for (i=first; i < n; i++)
	if (!write_something(filedes[i]))
	    abortx("Error writing to files");
    if (fork_number == 0)
	delete_files();
    exit(0);
}

int write_something(int fd)
{
    int rc;

    rc = write(fd, "I Love Linux!!!\n", 23);
    if (rc != 23)
	return(0);
    if (close(fd))
	return(0);
    return(1);
}

void delete_files(void)
{
    DIR *dirp;
    struct dirent *entp;
    struct stat stat_buffer;

    dirp = opendir(".");
    for (entp=readdir(dirp); entp; entp=readdir(dirp))
	if (!strncmp(entp->d_name, "apt", 3)) {
	    if (stat(entp->d_name, &stat_buffer))
		abortx("stat() failed for \"%s\", errno = %d",
		      entp->d_name, errno);
	    if (stat_buffer.st_size != 23)
		abortx("wrong file size for \"%s\"", entp->d_name);
	    if (unlink(entp->d_name))
		abortx("unlink failed for \"%s\"", entp->d_name);
	}
}

void abortx(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}
