/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
  char buf[1];
  int rc;

  buf[0] = 0;
  rc = write(1, buf, sizeof buf);
  if (rc < 0) {
    perror("write");
    exit(-1);
  }
  rc = read(0, buf, sizeof buf);
  if (rc < 0) {
    perror("read");
    exit(-1);
  } 
  exit(0);
}

