/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <linux/ext2_fs.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <string.h>

/*
 * Test the ioctl() calls on a file whose name is given as the first
 * argument. This version of the program expects some of the ioctl()
 * calls to fail, so if one does succeed, we exit with a bad return code.
 * This program expects the domain it is running as to have only read
 * acess to the given file.
 */
int main(int argc, char **argv) {
  struct utsname uts;
  int fd;
  int rc, oldkernel = 1;
  int val;

  if (uname(&uts) < 0) {
    perror("uname");
    exit(1);
  }

  if (strverscmp(uts.release, "2.6.27") >= 0)
    oldkernel = 0;

  fd = open(argv[1], O_RDONLY, 0);
 
  if(fd == -1) {
    perror("test_noioctl:open");
    exit(1);
  }

  /* This one should hit the FILE__IOCTL test and fail. */
  rc = ioctl(fd, FIGETBSZ, &val);
  if( rc == 0 ) {
      printf("test_noioctl:FIGETBSZ");
      exit(1);
  }

  /* This one should hit the FILE__IOCTL test and fail. */
  rc = ioctl(fd, FIOCLEX);
  if( rc == 0 ) {
    printf("test_noioctl:FIOCLEX");
    exit(1);
  }

  /*
   * This one depends on kernel version:
   * New:  Should hit the FILE__IOCTL test and fail.
   * Old:  Should only check FD__USE and succeed.
   */
  rc = ioctl(fd, FIONBIO, &val);
  if( !rc == !oldkernel ) {
    printf("test_noioctl:FIONBIO");
    exit(1);
  }

  /*
   * This one depends on kernel version:
   * New:  Should hit the FILE__READ test and succeed.
   * Old:  Should hit the FILE__GETATTR test and fail.
   */
  rc = ioctl(fd, EXT2_IOC_GETVERSION, &val);
  if( !rc != !oldkernel ) {
    perror("test_noioctl:EXT2_IOC_GETVERSION");
    exit(1);
  }

  /* This one should hit the FILE__WRITE test and fail. */
  val = 0;
  rc = ioctl(fd, EXT2_IOC_SETVERSION, &val);
  if( rc == 0 ) {
    perror("test_noioctl:EXT2_IOC_SETVERSION");
    exit(1);
  }

  close(fd);
  exit(0);

}
