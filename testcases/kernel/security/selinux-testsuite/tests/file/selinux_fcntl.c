/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * Test the fcntl file operation on a file whose name is given as the first
 * argument.
 */
int main(int argc, char **argv) {

  int fd;
  int rc;
  struct flock my_lock;

  if( argc != 2 ) {
    printf("usage: %s filename\n", argv[0]);
    exit(2);
  }

  fd = open(argv[1], O_RDONLY | O_APPEND, 0);
 
  if(fd == -1) {
    perror("selinux_fcntl:open");
    exit(2);
  }

  rc = fcntl(fd, F_SETFL, 0);
  if( rc == -1 ) {
    perror("selinux_fcntl:F_SETFL");
    exit(1);
  }

  rc = fcntl(fd, F_GETFL);
  if( rc == -1 ) {
    perror("selinux_fcntl:F_GETFL");
    exit(1);
  }

  my_lock.l_type = F_RDLCK;
  my_lock.l_start = 0;
  my_lock.l_whence = SEEK_CUR;
  my_lock.l_len = 0;

  rc = fcntl(fd, F_GETLK, &my_lock);
  if( rc == -1 ) {
    perror("selinux_fcntl:F_GETLK");
    exit(1);
  }

  close(fd);
  exit(0);

}
