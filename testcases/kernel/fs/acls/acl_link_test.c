#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "unistd.h"

int main(int argc, char *argv[]){
	ssize_t s;
	char * tok;
	char value[1024];
	char list[1024];
	char delim = '\0';
	int rc = 0;

	if ( argc < 2) {
		printf ("Please enter a file name as argument.\n");
		return -1;
	}
	
	syscall(__NR_listxattr, argv[1], &list, 1024); //listxattr
	tok = strtok((char *)&list, &delim);
	s = syscall(__NR_getxattr, argv[1],tok, (void *)&value, 1024); //getxattr

	s = syscall(__NR_lsetxattr,argv[1],tok,(void *)&value,s, 0); //lsetxattr

	if (s == -1) {
		printf ("User unable to change extended attributes %s !\n", argv[1]);
		printf("errno = %i\n", errno);
		rc = 1;
	}

	s = syscall(__NR_lremovexattr,argv[1],tok); //lremovexattr
	if (s == -1) {
                printf ("User unable to remove extended attributes %s !\n", argv[1]);
                printf("errno = %i\n", errno);
                rc = 1;
        }
	
	return  rc;		
}
