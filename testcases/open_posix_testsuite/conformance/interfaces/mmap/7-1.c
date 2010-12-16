/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * MAP_SHARED and MAP_PRIVATE describe the disposition of write references
 * to the memory object. If MAP_SHARED is specified, write references shall
 * change the underlying object. If MAP_PRIVATE is specified, modifications
 * to the mapped data by the calling process shall be visible only to the
 * calling process and shall not change the underlying object.
 * It is unspecified whether modifications to the underlying object done
 * after the MAP_PRIVATE mapping is established are visible through
 * the MAP_PRIVATE mapping.
 *
 * Test Step:
 * 1. mmap() a file, setting MAP_SHARED;
 * 2. modify the mapped memory;
 * 3. call msync() to synchronize the modification;
 * 4. munmap() this mapping;
 * 5. mmap() the same file again into memory;
 * 6. Check whether the modification has take effect;
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

#define TNAME "mmap/7-1.c"

int main()
{
  int rc;

  char tmpfname[256];
  char* data;
  int total_size = 1024;

  void *pa = NULL;
  void *addr = NULL;
  size_t size = total_size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  char * ch;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_7_1_%d",
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

  data = (char *) malloc(total_size);
  memset(data, 'a', total_size);
  if (write(fd, data, total_size) != total_size)
  {
    printf(TNAME "Error at write(): %s\n",
            strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  free(data);

  prot = PROT_READ | PROT_WRITE;
  flag = MAP_SHARED;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test Fail: " TNAME " Error at mmap: %s\n",
           strerror(errno));
    exit(PTS_FAIL);
  }

  ch = pa;
  *ch = 'b';

  /* Flush changes back to the file */

  if ((rc = msync(pa, size, MS_SYNC)) != 0)
  {
    printf(TNAME " Error at msync(): %s\n",
           strerror(rc));
    exit(PTS_UNRESOLVED);
  }

  munmap(pa, size);

  /* Mmap again */

  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test Fail: " TNAME " Error at 2nd mmap: %s\n",
           strerror(errno));
    exit(PTS_FAIL);
  }

  ch = pa;
  if (*ch == 'b')
  {
    printf("Test Pass\n");
    exit(PTS_PASS);
  }

  close(fd);
  munmap(pa, size);

  printf ("Test Fail, write referece does not change"
          "  the underlying file\n");
  return PTS_FAIL;
}