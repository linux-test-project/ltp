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
#include <signal.h>

void handler(int sig)
{
	return;
}

int main(void)
{
  struct sigaction sa;
  int i;

  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  for (i = 0; i < 32; i++) {
	  sigaction(i, &sa, NULL);
  }

  while (1)
	  ;
}