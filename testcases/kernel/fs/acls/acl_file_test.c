#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "unistd.h"

int main(int argc, char *argv[]){
	ssize_t s;
	char * tok;
	char value[1024];
	char list[1024];
	char delim = '\0';
	int rc = 0;
	char * file;
	int fd;

	if ( argc < 2) {
		printf ("Please enter a file name as argument.\n");
		return -1;
	}

	file = argv[1];

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf ("Unable to open file %s !", file);
		return -1;
	} 
	
	//syscall(232, file, &list, 1024); //listxattr
        syscall(__NR_listxattr, file, &list, 1024); //listxattr
	tok = strtok((char *)&list, &delim);
	//s = syscall(229, file,tok, (void *)&value, 1024); //getxattr
        s = syscall(__NR_getxattr, file,tok, (void *)&value, 1024); //getxattr

	//s = syscall(228, fd,tok,(void *)&value,s, 0); //fsetxattr
        s = syscall(__NR_fsetxattr, fd,tok,(void *)&value,s, 0); //fsetxattr

	if (s == -1) {
		printf ("User unable to change extended attributes on file %s !\n", argv[1]);
		printf("errno = %i\n", errno);
		rc = 1;
	}

	//s = syscall(237, fd,tok); //fremovexattr
        s = syscall(__NR_fremovexattr, fd,tok); //fremovexattr
	if (s == -1) {
                printf ("User unable to remove extended attributes file %s !\n", argv[1]);
                printf("errno = %i\n", errno);
                rc = 1;
        }
	
	close (fd);	
	return  rc;		
}
