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

#include "user_tbase.h"
#include "../tbase/tbase.h"

static int tbase_fd = -1;		/* file descriptor */


int 
tbaseopen() {
	struct stat     st;

	/* determine if there is a tbase device loaded */
        if (stat("/dev/tbase", &st)) {
                printf("\tERROR, Failed finding test base kernel interface \n");
                return (1);
        }

	/* open tbase device */
        if ((tbase_fd = open("/dev/tbase", O_RDWR)) < 0) {
                printf("\tFailed opening test base kernel interface \n");
                return (1);
        }

        return 0;
}

int 
tbaseclose() {

	if (tbase_fd != -1) {
		close (tbase_fd);
		tbase_fd = -1;
	}
		
	return 0;
}


int main() {
	int rc;

	/* open the module */
	rc = tbaseopen();
        if (rc ) {
                printf("Test MOD Driver may not be loaded\n");
                exit(1);
        }

	
	/* test bus rescan */
	if(ki_generic(tbase_fd, BUS_RESCAN))
                printf("Failed on bus rescan\n");
        else
                printf("Success on bus rescan\n");

	/* test get driver */
	if(ki_generic(tbase_fd, GET_DRV))
                printf("Failed on get driver\n");
        else
                printf("Success on get driver\n");

	/* test put driver */
	if(ki_generic(tbase_fd, PUT_DRV))
                printf("Failed on put driver\n");
        else
                printf("Success on put driver\n");

	/* test register firmware, should return not 0 */
	if(ki_generic(tbase_fd, REG_FIRM))
                printf("Failed on register firmware\n\tPossibly because parent nodes already set\n");
        else
                printf("Success on register firmware\n");	

	/* test create driver file sysfs */
        if(ki_generic(tbase_fd, CREATE_FILE))
                printf("Failed on creating driver file\n");
        else
                printf("Success on creating driver file\n");

	/* test device suspend and resume */
	if(ki_generic(tbase_fd, DEV_SUSPEND))
                printf("Failed on suspending device\n");
        else
                printf("Success on suspending device\n");
	
	/* test device create file sysfs */
	if(ki_generic(tbase_fd, DEV_FILE))
                printf("Failed on creating device file\n");
        else
                printf("Success on creating device file\n");

	/* test bus create file sysfs */
        if(ki_generic(tbase_fd, BUS_FILE))
                printf("Failed on creating bus file\n");
        else
                printf("Success on creating bus file\n");

	/* test register class */
        if(ki_generic(tbase_fd, CLASS_REG))
                printf("Failed on registering class\n");
        else
                printf("Success on registering class\n");
	
	/* test get class */
	if(ki_generic(tbase_fd, CLASS_GET))
                printf("Failed on get class\n");
        else
                printf("Success on get class\n");
	
	/* test class create file sysfs */
	if(ki_generic(tbase_fd, CLASS_FILE))
                printf("Failed on creating class file\n");
        else
                printf("Success on creating class file\n");

	/* test unregistering class */
        if(ki_generic(tbase_fd, CLASS_UNREG))
                printf("Failed on unregistering class\n");
        else
                printf("Success on unregistering class\n");

	/* test register class device */
	if(ki_generic(tbase_fd, CLASSDEV_REG))
                printf("Failed on registering class device and creating sysfs file\n");
        else
                printf("Success on registering class device and creating sysfs file\n");

	/* test register class interface */
        if(ki_generic(tbase_fd, CLASSINT_REG))
                printf("Failed on registering class interface\n");
        else
                printf("Success on registering class interface\n");

	/* test register sysdev_class */
        if(ki_generic(tbase_fd, SYSDEV_CLS_REG))
                printf("Failed on registering sysdev_class\n");
        else
                printf("Success on registering sysdev_class\n");

	/* test register sysdev */
        if(ki_generic(tbase_fd, SYSDEV_REG))
                printf("Failed on registering sysdev\n");
        else
                printf("Success on registering sysdev\n");

        /* test unregister sysdev */
        if(ki_generic(tbase_fd, SYSDEV_UNREG))
                printf("Failed on unregistering sysdev\n");
        else
                printf("Success on unregistering sysdev\n");


	/* test unregister sysdev_class */
        if(ki_generic(tbase_fd, SYSDEV_CLS_UNREG))
                printf("Failed on unregistering sysdev_class\n");
        else
                printf("Success on unregistering sysdev_class\n");	





	/* close the module */
	rc = tbaseclose();
	if (rc ) {
                printf("Test MOD Driver may not be closed\n");
                exit(1);
        }

        return 0;
}
