/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mapping established by mmap() shall replace any previous
 * mappings for those whole pages containing any part of the address
 * space of the process starting at pa and continuing for len bytes.
 *
 * Test Step:
 * 1. Set the size of the file to be mapped as (2 * _SC_PAGE_SIZE);
 * 2. Map size = (_SC_PAGE_SIZE + 2) bytes into memory,
 *    setting the content as 'a'. The mapped address is pa.
 * 2. Map size2 = (_SC_PAGE_SIZE + 1) bytes into memory, starting at the same
 *    address as the first mmap, i.e. pa, using MAP_FIXED flag.
 *    Setting the cotent as 'b'
 * 3. Test whether byte *(pa + size) is 'b'.
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

#define TNAME "mmap/3-1.c"

int main()
{
  char tmpfname[256];
  char tmpfname2[256];
  char* data;
  long total_size;
  long page_size;

  void *pa = NULL;
  void *addr = NULL;
  size_t size;
  int prot = PROT_READ | PROT_WRITE;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;

  void *pa2 = NULL;
  void *addr2 = NULL;
  size_t size2;
  int prot2 = PROT_READ | PROT_WRITE;
  int flag2 ;
  int fd2;
  off_t off2 = 0;

  char *ch;

#ifndef MAP_FIXED
  printf("MAP_FIXED does not defined\n");
  exit(PTS_UNRESOLVED);
#endif

  page_size = sysconf(_SC_PAGE_SIZE);
  size = page_size + 2;
  size2 = page_size + 1;

  /* Size of the file */
  total_size = 2 * page_size;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_3_1_%d",
           getpid());
  snprintf(tmpfname2, sizeof(tmpfname2), "/tmp/pts_mmap_3_1_2_%d",
           getpid());
  data = (char *) malloc(total_size);
  unlink(tmpfname);
  unlink(tmpfname2);
  fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  fd2 = open(tmpfname2, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  if (fd == -1 || fd2 == -1)
  {
    printf(TNAME " Error at open(): %s\n",
           strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  unlink(tmpfname);
  unlink(tmpfname2);
  memset(data, 'a', total_size);
  if (write(fd, data, total_size) != total_size)
  {
    printf(TNAME "Error at write(), fd: %s\n",
            strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  memset(data, 'b', total_size);
  if (write(fd2, data, total_size) != total_size)
  {
    printf(TNAME "Error at write(), fd1: %s\n",
            strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  free(data);

  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf ("Test Fail: " TNAME " Error at mmap: %s\n",
            strerror(errno));
    exit(PTS_FAIL);
  }

  ch = pa + size;
  if (*ch != 'a')
  {
    printf ("Test Fail: " TNAME
            " The file did not mapped to memory\n");
    exit(PTS_FAIL);
  }

  /* Replace orginal mapping*/

  addr2 = pa;
  flag2 = MAP_SHARED | MAP_FIXED;
  pa2 = mmap(addr2, size2, prot2, flag2, fd2, off2);
  if (pa2 == MAP_FAILED)
  {
    printf ("Test Fail: " TNAME " Error at 2nd mmap: %s\n",
            strerror(errno));
    exit(PTS_FAIL);
  }

  if (pa2 != pa)
  {
    printf ("Test Fail: " TNAME " Error at mmap: "
            "Second mmap does not replace the first mmap\n");
    exit(PTS_FAIL);
  }

  ch = pa2 + size;
  if (*ch != 'b')
  {
    printf ("Test Fail: " TNAME
            " The original mapped page has not been replaced\n");
    exit(PTS_FAIL);
  }

  close(fd);
  close(fd2);
  munmap(pa, size);
  munmap(pa2, size2);
  printf ("Test Pass\n");
  return PTS_PASS;
}