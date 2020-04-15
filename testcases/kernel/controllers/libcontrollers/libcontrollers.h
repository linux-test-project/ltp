/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        libcontrollers.h                                              */
/*                                                                            */
/* Description: This file contains the declarations for the functions and     */
/*              variables used by the library and the test files.             */
/*                                                                            */
/* Author:      Sudhir Kumar skumar@linux.vnet.ibm.com                        */
/*                                                                            */
/* History:                                                                   */
/* Created-     15/02/2008 -Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern char fullpath[PATH_MAX];
extern int FLAG;
extern volatile int timer_expired;
extern int retval;
extern unsigned int num_line;
extern unsigned int current_shares;
extern unsigned int total_shares;
extern unsigned int *shares_pointer;
extern char target[LINE_MAX];
extern struct dirent *dir_pointer;

enum{
	GET_SHARES	=1,
	GET_TASKS
};

static inline void error_function(char *msg1, char *msg2);

int read_shares_file(char *filepath);

int read_file(char *filepath, int action, unsigned int *value);

int scan_shares_files(unsigned int *shares_pointer);

int write_to_file (char * file, const char* mode, unsigned int value);

void signal_handler_alarm (int signal );

void signal_handler_sigusr2 (int signal);
