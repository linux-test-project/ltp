/*
 * This file will include user space functions that will drive
 * the kernel module to test various functions and kernel
 * calls. Each function will need to setup the tif structure
 * so that the in parameters and out parameters are correctly 
 * initialized
 * 
 * use tif structure for passing params between user
 * space and kernel space, in some tests it is really not 
 * needed, and if nothing is needed to pass in utilize 
 * the ki_generic function below. the tif structure makes 
 * it easy to maintain all the tests if they have the same 
 * process in kernel space to read in params in the kernel 
 * module no matter what the test is
 * 
 * author: Sean Ruyle
 * date:   06/11/2003
 *
 * tmod_ki.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../tbase/tbase.h"

int ki_generic(int fd, int flag) {
        int                     rc;
        tmod_interface_t        tif;

        /*
         * build interface structure
         */
        tif.in_len = 0;
        tif.in_data = 0;
        tif.out_len = 0;
        tif.out_data = 0;
        tif.out_rc = 0;

        /*
         * ioctl call for flag
         */
        rc = ioctl(fd, flag, &tif);
        if(rc) {
                printf("Ioctl error\n");
                return rc;
        }
        if(tif.out_rc) 
                return tif.out_rc;

        return rc;
}





