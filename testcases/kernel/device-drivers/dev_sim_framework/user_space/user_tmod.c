/*
 * This is the main of your user space test program, 
 * which will open the correct kernel module, find the 
 * file descriptor value and use that value to make 
 * ioctl calls to the system
 *
 * Use the ki_generic and other ki_testname functions 
 * to abstract the calls from the main
 *
 * author: Sean Ruyle
 * date:   06/11/2003
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/kernel.h>

#include "user_tmod.h"
#include "../kernel_space/tmod.h"

static int tmod_fd = -1;		/* file descriptor */


int 
tmodopen() {
	struct stat     st;

	/* determine if there is a tmod device loaded */
        if (stat("/dev/tmod", &st)) {
                printf("\tERROR, Failed finding test mod kernel interface \n");
                return (1);
        }

	/* open tmod device */
        if ((tmod_fd = open("/dev/tmod", O_RDWR)) < 0) {
                printf("\tFailed opening test mod kernel interface \n");
                return (1);
        }

        return 0;
}

int 
tmodclose() {

	if (tmod_fd != -1) {
		close (tmod_fd);
		tmod_fd = -1;
	}
		
	return 0;
}


int main() {
	int rc;

	/* open the module */
	rc = tmodopen();
        if (rc ) {
                printf("Test MOD Driver may not be loaded\n");
                exit(1);
        }

	

	/* make test calls */
	if(ki_generic(tmod_fd, LTP_OPTION1))
		printf("Failed on option 1 test\n");
	else
		printf("Success on option 1 test\n");



	/* close the module */
	rc = tmodclose();
	if (rc ) {
                printf("Test MOD Driver may not be closed\n");
                exit(1);
        }

        return 0;
}
