/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<linux/fs.h>
#include<linux/ext2_fs.h>

/*
 * Test the ioctl() calls on a file whose name is given as the first 
 * argument. This version of the program expects some of the ioctl()
 * calls to fail, so if one does succeed, we exit with a bad return code.
 * This program expects the domain it is running as to have only read
 * acess to the given file.
 */
int main(int argc, char **argv) {

  int fd;
  int rc;
  int val;

  fd = open(argv[1], O_RDONLY, 0);
  
  if(fd == -1) {
    perror("test_noioctl:open");
    exit(1);
  }

  /* This one should hit the normal file descriptor use test; expect success */
  rc = ioctl(fd, FIONBIO, &val);
  if( rc != 0 ) {
    perror("test_noioctl:FIONBIO");
    exit(1);
  }

  /* This one should hit the FILE__GETATTR test; expect failure */
  rc = ioctl(fd, FIGETBSZ, &val);
  if( rc == 0 ) {
    exit(1);
  }

  /* This one should hit the FILE__IOCTL test */
  rc = ioctl(fd, FIOCLEX);
  if( rc == 0 ) {
    exit(1);
  }

  val = 0;
  /* This one should hit the FILE__SETATTR test; expect failure */
  rc = ioctl(fd, EXT2_IOC_SETVERSION, &val);
  if( rc == 0 ) {
    exit(1);
  }

  close(fd);
  exit(0);

}
