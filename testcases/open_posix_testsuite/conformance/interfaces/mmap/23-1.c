/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [ENODEV] The fildes argument refers to a file whose type is not supported by mmap().
 *
 * Test Step:
 * 1. Create pipe;
 * 2. mmap the pipe fd to memory, should get ENODEV;
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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define TNAME "mmap/23-1.c"

int main()
{
  int pipe_fd[2];

  void *pa = NULL;
  void *addr = NULL;
  size_t len = 1024;
  int prot = PROT_READ;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;

  if (pipe(pipe_fd) == -1)
  {
    printf("Test Unresolved: " TNAME " Error at pipe(): %s\n",
            strerror(errno));
    exit(PTS_UNRESOLVED);
  }

  fd = pipe_fd[0];
  pa = mmap(addr, len, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == ENODEV)
  {
    printf("Test Pass: " TNAME " Get ENODEV when mmap a pipe fd\n");
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    exit(PTS_PASS);
  }
  else
  {
    printf ("Test Fail: " TNAME
            " Expect ENODEV, get: %s\n", strerror(errno));
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    exit(PTS_FAIL);
  }
}