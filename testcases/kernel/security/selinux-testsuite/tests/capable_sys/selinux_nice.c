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
#include <errno.h>
#include <unistd.h>

/*
 * Test the nice() system call.
 * This call will result in a CAP_SYS_NICE capable check.
 */
int main(int argc, char **argv) {

  int rc;

  rc = nice(-10);
  if (rc == -1) {
    perror("test_nice:nice");
    exit(1);
  }

  exit(0);

}