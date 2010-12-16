/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * When the implementation selects a
 * The mmap() function shall fail if:
 * [EINVAL] The value of flags is invalid (neither MAP_PRIVATE nor MAP_SHARED is
 * set).
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

#define TNAME "mmap/21-1.c"

int main()
{
  char tmpfname[256];
  long total_size;

  void *pa = NULL;
  void *addr = NULL;
  size_t size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  total_size = 1024;
  size = total_size;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_21_1_%d",
           getpid());
  unlink(tmpfname);
  fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    printf(TNAME " Error at open(): %s\n",
           strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  unlink(tmpfname);

  if (ftruncate(fd, total_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n",
            strerror(errno));
    exit(PTS_UNRESOLVED);
  }

  flag = MAP_SHARED;
  while (flag == MAP_SHARED || flag == MAP_PRIVATE
         || flag == MAP_FIXED)
  	flag++;

  prot = PROT_READ | PROT_WRITE;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EINVAL)
  {
  	printf ("Test PASS: " TNAME " Error at mmap: %s\n",
            strerror(errno));
    exit(PTS_PASS);
  }

  close (fd);
  munmap (pa, size);
  printf ("Test FAIL\n");
  return PTS_FAIL;
}