
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/errno.h>
#include <linux/netdevice.h>

#include "LtpAcpi.h"

int LTP_acpi_open_block_device(void);

int ltp_block_dev_handle = 0;      /* handle to LTP Test block device */

int main(int argc, char **argv)
{

    ltpdev_cmd_t  cmd = {0,0};
    int rc;

    printf("[%s] - Running test program\n", argv[0]);

    rc = LTP_acpi_open_block_device();

    if (!rc) {

        ltp_block_dev_handle = open(LTP_ACPI_DEVICE_NAME, O_RDWR);

        if (ltp_block_dev_handle < 0) {
            printf("ERROR: Open of device %s failed %d errno = %d\n", LTP_ACPI_DEVICE_NAME,ltp_block_dev_handle, errno);
        }
        else {
            rc = ioctl (ltp_block_dev_handle, LTPDEV_CMD, &cmd);

            printf("return from ioctl %d \n", rc);
        }

    } else {
        printf("ERROR: Create/open block device failed\n");
    }

    return 0;
}

int LTP_acpi_open_block_device()
{
    dev_t devt;
    struct stat statbuf;
    int rc;


    if (ltp_block_dev_handle == 0) {

        /* check for the /dev/LTPACPITest subdir, and create if it does not exist.
         *
         * If devfs is running and mounted on /dev, these checks will all pass,
         * so a new node will not be created.
         */
        devt = makedev(LTPMAJOR, 0);

        rc = stat(LTP_ACPI_DEV_NODE_PATH, &statbuf);

        if (rc) {
            if (errno == ENOENT) {
                /* dev node does not exist. */
                rc = mkdir(LTP_ACPI_DEV_NODE_PATH, (S_IFDIR | S_IRWXU |
                                                    S_IRGRP | S_IXGRP |
                                                    S_IROTH | S_IXOTH));
            } else {
                printf("ERROR: Problem with LTP ACPI dev directory.  Error code from stat() is %d\n\n", errno);
            }

        } else {
            if (!(statbuf.st_mode & S_IFDIR)) {
                rc = unlink(LTP_ACPI_DEV_NODE_PATH);
                if (!rc) {
                    rc = mkdir(LTP_ACPI_DEV_NODE_PATH, (S_IFDIR | S_IRWXU |
                                                    S_IRGRP | S_IXGRP |
                                                    S_IROTH | S_IXOTH));
                }
            }
        }


        /*
         * Check for the /dev/ltp-acpi/block_device node, and create if it does not
         * exist.
         */
        rc = stat(LTP_ACPI_DEVICE_NAME, &statbuf);
        if (rc) {
            if (errno == ENOENT) {
                /* dev node does not exist */
                rc = mknod(LTP_ACPI_DEVICE_NAME, (S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP), devt);
            } else {
                printf("ERROR:Problem with LTP ACPI block device node directory.  Error code form stat() is %d\n\n", errno);
            }

        } else {
            /*
             * /dev/ltp-acpi/block_device exists.  Check to make sure it is for a
             * block device and that it has the right major and minor.
             */
            if ((!(statbuf.st_mode & S_IFBLK)) ||
                 (statbuf.st_rdev != devt)) {

                /* Recreate the dev node. */
                rc = unlink(LTP_ACPI_DEVICE_NAME);
                if (!rc) {
                    rc = mknod(LTP_ACPI_DEVICE_NAME, (S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP), devt);
                }
            }
        }

    }

    return rc;
}
