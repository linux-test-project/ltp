/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The file descriptor fildes shall have been opened with read permission,
 * regardless of the protection options specified. If PROT_WRITE is
 * specified, the application shall ensure that it has opened the file
 * descriptor fildes with write permission unless MAP_PRIVATE
 * is specified in the flags parameter as described below.
 *
 * Test Step:
 * 1  Open a file RDONLY permition.
 * 2. Mmap the file to a memory region setting prot as PROT_WRITE.
 * 3. Setting flag as MAP_SHARED.
 * 4. Get EACCES error when mmap().
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

#define TNAME "mmap/6-4.c"

int main()
{
  char tmpfname[256];
  int total_size = 1024;

  void *pa = NULL;
  void *addr = NULL;
  size_t size = total_size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_6_4_%d",
           getpid());

  /* Create a tmp file, set the content as 'a' */
  unlink(tmpfname);
  fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    printf(TNAME " Error at open(): %s\n",
           strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  if (ftruncate(fd, total_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n",
           strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  close(fd);

  /* Open as read only */

  fd = open(tmpfname, O_RDONLY,
            S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    printf(TNAME " Error at 2nd open(): %s\n",
           strerror(errno));
    exit(PTS_UNRESOLVED);
  }
  unlink(tmpfname);

  prot = PROT_WRITE;
  flag = MAP_SHARED;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EACCES)
  {
    printf("Test PASS: " TNAME " Get EACCES Error: %s\n",
           strerror(errno));
    exit(PTS_PASS);
  }

  printf ("Test Fail: Did not get EACCES as expected\n");
  return PTS_FAIL;
}