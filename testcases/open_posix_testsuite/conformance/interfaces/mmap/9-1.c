/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * If MAP_FIXED is set,
 * mmap() may return MAP_FAILED and set errno to [EINVAL].
 *
 * [EINVAL] The addr argument (if MAP_FIXED was specified) or off is not a multiple of
 * the page size as returned by sysconf(), or is considered invalid by the
 * implementation.
 *
 * Test step:
 * 1. Set 'addr' as an illegal address, which is not a multiple of page size;
 * 2. Call mmap() and get EINVAL;
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

#define TNAME "mmap/9-1.c"

int main()
{
  char tmpfname[256];
  long  page_size;
  long total_size;

  void *illegal_addr;
  void *pa = NULL;
  void *addr = NULL;
  size_t size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  page_size = sysconf(_SC_PAGE_SIZE);
  total_size = page_size;
  size = total_size;

  /* Create tmp file */

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_9_1_%d",
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

  /* Map the file for the first time, to get a legal address, pa */

  flag = MAP_SHARED;
  prot = PROT_READ | PROT_WRITE;
  pa = mmap(addr, size, prot, flag, fd, off);

  if ((unsigned long)pa % page_size)
  {
    printf("pa is not multiple of page_size\n");
    illegal_addr = pa;
  }
  else
  {
    printf("pa is a multiple of page_size\n");
    illegal_addr = pa + 1;
  }

  munmap (pa, size);

  /* Mmap again using the illegal address, setting MAP_FIXED */
  prot = PROT_READ | PROT_WRITE;
  flag = MAP_FIXED;
  addr = illegal_addr;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EINVAL)
  {
    printf("Test Pass: " TNAME " Set MAP_FIXED and get "
    	   "EINVAL when 'addr' is illegal\n");
    exit(PTS_PASS);
  }

  close (fd);
  munmap (pa, size);
  printf ("Test FAIL, Set MAP_FIXED but did not get EINVAL"
          " when 'addr' is illegal\n");
  return PTS_FAIL;
}