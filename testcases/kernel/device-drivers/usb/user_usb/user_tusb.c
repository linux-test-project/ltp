#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/kernel.h>
#include "../tusb/tusb.h"

static int tusb_fd = -1;		//file descriptor

int 
tusbopen() {
	struct stat st;			//for test exist tusb driver
	
	/*
	 * Make sure the tusb driver is up
	 */
	if (stat("/dev/tusb", &st)) {
		printf("\tFailed to find tusb driver\n");
		return 1;
	}

	if ((tusb_fd = open("/dev/tusb", O_RDWR)) < 0) {
		printf("\tFailed opening tusb driver\n");
		return 1;
	}

	return 0;
}

int 
tusbclose() {
	/*
	 * Close the tusb driver 
	 */
	if (tusb_fd != -1) {
		close (tusb_fd);
		tusb_fd = -1;
	}
	
	return 0;
}


int main() {
	int 	rc = 0;

	rc = tusbopen();
	if( rc ) {
		printf("tusb driver may not be loaded\n");
		exit(1);
	}

	/* test find device pointer */
	if(ki_generic(tusb_fd, FIND_DEV))
		printf("Failed to find usb device pointer\n");
	else
		printf("Found usb device pointer\n");
	
	/* test find usb hostcontroller */
	if(ki_generic(tusb_fd, TEST_FIND_HCD))
                printf("Failed to find usb hcd pointer\n");
        else
                printf("Found usb hcd pointer\n");

	/* test hcd probe */
	if(ki_generic(tusb_fd, TEST_HCD_PROBE))
                printf("Failed on hcd probe call\n");
        else
                printf("Success hcd probe\n");

        /* test hcd suspend */
        if(ki_generic(tusb_fd, TEST_HCD_SUSPEND))
                printf("Failed on hcd suspend call\n");
        else
                printf("Success hcd suspend\n");

        /* test hcd resume */
        if(ki_generic(tusb_fd, TEST_HCD_RESUME))
                printf("Failed on hcd resume call\n");
        else
                printf("Success hcd resume\n");

#if 0 
	/* test hcd remove */
	if(ki_generic(tusb_fd, TEST_HCD_REMOVE))
		printf("Failed on hcd remove call\n");
	else
		printf("Success hcd remove\n");
#endif

	tusbclose();

	return 0;
}

