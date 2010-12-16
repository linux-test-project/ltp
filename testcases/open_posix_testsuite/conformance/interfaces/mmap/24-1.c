/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [ENOMEM] MAP_FIXED was specified, and the range [addr,addr+len)
 * exceeds that allowed for the address space of a process;
 * or, if MAP_FIXED was not specified and
 * there is insufficient room in the address space to effect the mapping.
 *
 * Test Step:
 * 1. In a very long loop, keep mapping a shared memory object,
 *    until there this insufficient room in the address space;
 * 3. Should get ENOMEM.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "posixtest.h"

#define TNAME "mmap/24-1.c"

int main()
{
  char tmpfname[256];
  int shm_fd;

  void *pa = NULL;
  void *addr = NULL;
  size_t len;
  int prot = PROT_READ | PROT_WRITE;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;

  /* Size of the shared memory object */
  size_t shm_size = 1024;

  size_t mapped_size = 0;

  snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_25_1_%d",
           getpid());

  /* Create shared object */
	shm_unlink(tmpfname);
	shm_fd = shm_open(tmpfname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if (shm_fd == -1)
	{
		printf(TNAME " Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
  shm_unlink(tmpfname);
  if (ftruncate(shm_fd, shm_size) == -1) {
    printf(TNAME " Error at ftruncate(): %s\n", strerror(errno));
    return PTS_UNRESOLVED;
  }

  fd = shm_fd;
  len = shm_size;

  mapped_size = 0;
  while (mapped_size < SIZE_MAX)
  {
    pa = mmap (addr, len, prot, flag, fd, off);
    if (pa == MAP_FAILED && errno == ENOMEM)
    {
      printf ("Test Pass: " TNAME " Get ENOMEM: %s\n",
              strerror(errno));
      printf ("Total mapped size is %lu bytes\n", (unsigned long)mapped_size);
      exit(PTS_PASS);
    }

    mapped_size += shm_size;
    if (pa == MAP_FAILED)
      perror("Error at mmap()");
  }

  close(fd);
  printf ("Test Fail: Did not get ENOMEM as expected\n");
  return PTS_FAIL;
}