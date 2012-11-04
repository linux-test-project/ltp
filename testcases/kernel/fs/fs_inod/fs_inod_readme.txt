Readme file for FS_INOD test

LAWYER STUFF:

   Copyright (c) International Business Machines  Corp., 2001

   This program is free software;  you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY;  without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program;  if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


TEST:

 	NAME:		fs_inod
	FUNCTIONALITY: 	File system stress - inode allocation/deallocation
 	DESCRIPTION:	Rapidly creates and deletes files through
			multiple processes running in the background.
			The user may specify the number of subdirectories
			to create, the number of files to create (per
			subdirectory), and the number of times to repeat
			the creation/deletion cycle.
USE:
	FS_INOD is a korn shell script that was originally written as part
	of a jfs stress test suite for AIX.  The command to execute the script
	is:

	./fs_inod [volumename] [numsubdirectories] [numfiles] [numloops]

    volumename: name of volume or filesystem to test
    numsubdirs:	number of subdirectories per main directory "dirX"
    numfiles:	number of files per subdirectory
    numloops:	number of loops for the creation/deletion cycle

	FS_INOD will create two directories (dir1 and dir2) in whichever
	directory it is executed, or you can set environment variable FS
	to the name of the volume you wish to test. This variable is
	currently set in function Main in the script, but may be changed.
	Below dir1 and dir2 it will create numsubdirs, and within each
	subdir it will create numfiles.  Then it will delete the files
	and subdirs.  It will repeat the creation/deletion process for
	numloops.  It should be noted that large values for numsubdirs
	and numfiles can cause the test to exceed the capacity of a given
	hard drive.  There is no error checking for this, so beware.
	Additionally, a large value for numloops may cause the test to
	run for a very long time.

	FS_INOD will completely clean up all files and directories
	created during the test.

	Currently all FS_INOD output goes to stdout, to preserve ouput
	redirect to a file.

FUTURE ENHANCEMENTS:

	1. Command line switches for all parameters. -DONE
	2. Help screen								 -DONE


CHANGE HISTORY:
DATE            	AUTHOR                  REASON
04/18/98        	Dara Morgenstein        Project Yeager (AIX)
02/08/01		Jay Inman				Modified to run standalone on Linux
05/24/01		Jay Inman				Added command line args
