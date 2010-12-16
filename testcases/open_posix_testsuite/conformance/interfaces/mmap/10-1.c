/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * When the implementation selects a
 * value for pa, it never places a mapping at address 0.
 *
 * Test step:
 * This is not a good test. Cannot make sure (pa == 0) never happens.
 * Repeat LOOP_NUM times mmap() and mnumap(),
 * make sure pa will not equal 0.
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

#define TNAME "mmap/10-1.c"

#define LOOP_NUM 100000

int main()
{
  int rc;
  unsigned long cnt;

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

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_10_1_%d",
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
  prot = PROT_READ | PROT_WRITE;
  for (cnt = 0; cnt < LOOP_NUM; cnt++)
  {
    pa = mmap(addr, size, prot, flag, fd, off);
    if (pa == MAP_FAILED)
    {
  	  printf ("Test Fail: " TNAME " Error at mmap: %s\n",
              strerror(errno));
      exit(PTS_FAIL);
    }

    if (pa == 0)
    {
  	  printf("Test Fail " TNAME " mmap() map the file to 0 address "
  		       "without setting MAP_FIXED\n");
  	  exit(PTS_FAIL);
    }
    rc = munmap (pa, size);
    if (rc != 0)
    {
      printf(TNAME "Error at mnumap(): %s\n",
            strerror(errno));
      exit(PTS_UNRESOLVED);
    }
  }

  close (fd);
  printf ("Test PASS\n");
  return PTS_PASS;
}