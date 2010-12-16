/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Test the sethostname() call.
 * This call will result in a CAP_SYS_ADMIN capable check.
 */
int main(int argc, char **argv) {

  int rc;
  char buf[255];

  rc = gethostname(buf, sizeof(buf));
  if (rc != 0) {
    perror("test_sethostname:gethostname");
    exit(2);
  }

  rc = sethostname(buf, strlen(buf));
  if (rc != 0) {
    perror("test_sethostname:sethostname");
    exit(1);
  }

  exit(0);

}