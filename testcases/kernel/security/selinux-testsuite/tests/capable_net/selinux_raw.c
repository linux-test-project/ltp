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
#include <netdb.h>
#include <unistd.h>

/*
 * Test the bind() operation for a raw socket.
 */
int main(int argc, char **argv) {

  int fd;

  fd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
  if (fd == -1) {
    perror("test_raw:socket");
    exit(1);
  }

  close(fd);
  exit(0);

}