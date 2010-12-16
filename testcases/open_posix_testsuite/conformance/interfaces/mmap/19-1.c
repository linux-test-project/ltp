/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [EBADF] The fildes argument is not a valid open file descriptor.
 *
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define TNAME "mmap/19-1.c"

int main()
{
  void *pa = NULL;
  void *addr = NULL;
  size_t size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  size = 1024;

  fd = -1;
  flag = MAP_SHARED;
  prot = PROT_READ | PROT_WRITE;

  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EBADF)
  {
  	printf ("Test Pass: " TNAME " Get EBADF when fd is invalid\n");
    exit(PTS_PASS);
  }

  if (pa == MAP_FAILED)
    perror("mmap() error");
  printf ("Test FAIL: Did not get EBADF when fd is invalid\n");
  return PTS_FAIL;
}