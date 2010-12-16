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
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include <sys/mman.h>
#include<selinux/selinux.h>

/*
 * Test the mmap file operation on a file whose name is given as the first
 * argument. The second argument must be the SID we are to use to test
 * the actual mmap() operation by changing the SID of the file we are
 * given.
 */

int main(int argc, char **argv) {

  int fd;
  void *ptr;
  int rc;

  if (argc != 3) {
    printf("usage: %s filename context\n", argv[0]);
    exit(2);
  }

  fd = open(argv[1], O_RDWR, 0);

  if (fd == -1) {
    perror("selinux_mmap:open");
    exit(2);
  }

  rc = fsetfilecon(fd, argv[2]);
  if (rc < 0) {
    perror("selinux_mmap:fsetfilecon");
    exit(2);
  }

  ptr = mmap(0, 1024, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, getpagesize());
  if (ptr == MAP_FAILED) {
    perror("selinux_mmap:mmap");
    close(fd);
    exit(1);
  }

  close(fd);
  exit(0);

}