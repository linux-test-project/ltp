/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2001                 */
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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
   File:        CheckVer.c

   Description: Takes version numbers of software (or anything else for that matter)
                Compares them and then returns:

                0 - Older -  The version passed is older than the required version
                1 - Newer or matching - The version passed is at least the required version
                -1 - some kind of error condition

                This is a very simple program so very little error checking is done.
     
   mridge@us.ibm.com - Initial version   
     
******************************************************************************/     
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

#define DEBUG 1
#define MIN_ARGS  2
#define MIN_VER_LEN 255  // Seems like that should be enough
#define JUMP_POINT 2
     
int main (int argc, char *argv[])     
{
    int version = 1;
    int BaseVerLen, CurrentVerLen, i = 0;
    char BaseVersion[MIN_VER_LEN];
    char CurrentVersion[MIN_VER_LEN];


    if (argc < MIN_ARGS) {
        printf("Returning not enough params\n");
        exit (2);
    }

    strcpy(BaseVersion, argv[1]);
    strcpy(CurrentVersion, argv[2]);
    BaseVerLen = strlen(BaseVersion);
    CurrentVerLen = strlen(CurrentVersion);

    if (DEBUG) {
        printf("BaseVersion = %s \n", BaseVersion);
        printf("CurrentVersion = %s \n", CurrentVersion);
    }

    if (BaseVersion[i] > CurrentVersion[i]) {
        printf("Way too old\n");
        version = 0;
    }

    i+=JUMP_POINT;

    if (i <= BaseVerLen && i < CurrentVerLen) {
        if (version && BaseVersion[i] > CurrentVersion[i]) {
            printf("Just too old\n");
            version = 0;
        }
    }
    else if (BaseVerLen > CurrentVerLen){
        printf("Not enough points\n");
        version = 0;
    }

    i+=JUMP_POINT;

    if (i <= BaseVerLen && i < CurrentVerLen) {
        if (version && BaseVersion[i] > CurrentVersion[i]) {
            printf("Just a little too old\n");
            version = 0;
        }
    }
    else if (BaseVerLen > CurrentVerLen){
        printf("Not enough points\n");
        version = 0;
    }

    i+=JUMP_POINT;

    if (i <= BaseVerLen && i < CurrentVerLen) {
        if (version && BaseVersion[i] > CurrentVersion[i]) {
            printf("Just a smidge too old\n");
            version = 0;
        }
    }
    else if (BaseVerLen > CurrentVerLen){
        printf("Not enough points\n");
        version = 0;
    }

    if (DEBUG) {
        printf("Returning %d \n", version);
    }
    exit (version);
}

