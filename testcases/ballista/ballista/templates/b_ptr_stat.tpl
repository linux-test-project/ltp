// b_ptr_stat.tpl : Ballista Datatype Template for stat pointer
// Copyright (C) 1998-2001  Carnegie Mellon University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

name structStatptr b_ptr_stat;

parent b_ptr_buf;

includes
[
{
#define structStatptr struct stat *
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include "b_ptr_buf.h"
}
]

global_defines
[
{
static struct stat stat_temp;

#define TESTDIR "testdir"
#define TESTFILE        "testdir/testfile_stat"
}
]

dials
[
  enum_dial STAT : ZEROS, NEG1S, MAX, FILE_SH, FILE_PASSWD, FILE_TEST;
]

access
[
  ZEROS
  {
    stat_temp.st_mode = 0;
    stat_temp.st_ino = 0;
    stat_temp.st_dev = 0;
    stat_temp.st_nlink = 0;
    stat_temp.st_uid = 0;
    stat_temp.st_gid = 0;
    stat_temp.st_size = 0;
    stat_temp.st_atime = 0;
    stat_temp.st_mtime = 0;
    stat_temp.st_ctime = 0;
  }

  NEG1S
  {
    stat_temp.st_mode = -1;
    stat_temp.st_ino = -1;
    stat_temp.st_dev = -1;
    stat_temp.st_nlink = -1;
    stat_temp.st_uid = -1;
    stat_temp.st_gid = -1;
    stat_temp.st_size = -1;
    stat_temp.st_atime = -1;
    stat_temp.st_mtime = -1;
    stat_temp.st_ctime = -1;
  }

  MAX
  {  // These are very system dependent - values used are the smallest MAX for the 3 supported systems
    stat_temp.st_mode = 2*MAXINT +1;
    stat_temp.st_ino = 2*MAXINT +1;
    stat_temp.st_dev = MAXINT;  
    stat_temp.st_nlink = 2*MAXSHORT +1;
    stat_temp.st_uid = MAXINT;
    stat_temp.st_gid = MAXINT; 
    stat_temp.st_size = MAXLONG; 
    stat_temp.st_atime = MAXINT;
    stat_temp.st_mtime = MAXINT;
    stat_temp.st_ctime = MAXINT;
  }

  FILE_SH
  {
    if((stat("/bin/sh", &stat_temp))!=0)
    {
      perror("stat(\"/bin/sh\")failed ");
    }
  }

  FILE_PASSWD
  {
    if((stat("/etc/passwd", &stat_temp))!=0)
    {
      perror("stat(\"/etc/passwd\") failed");
    }
  }

  FILE_TEST
  {
    int fd;

    // Setup log file for template information

    FILE* logFile = NULL;
    if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
    {
      exit(99);
    }

    if (mkdir(TESTDIR,S_IRWXU|S_IRWXG|S_IRWXO)!=0) /* create test directory, u+rwx */
    {
      //if the directory already exists ignore the error
      if (errno != EEXIST) 
      {
         fprintf(logFile,"b_ptr_stat - FILE_TEST mkdir(\"testdir\")failed. Function not tested\n");
         fclose(logFile);
         exit(99);
      }
    }

    //remove the file/directory, ignore error if already removed
    chown(TESTFILE, getuid(), getgid()); //attempt to change ownership
    chmod(TESTFILE, S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH);
    rmdir(TESTFILE);//may be a directory
    remove(TESTFILE);

    fd = open (TESTFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) 
    {
      fprintf (logFile, "b_ptr_stat - FILE_TEST open in stat_createfile\n");
      fclose(logFile);
      exit(99);
    }

    write (fd, "I am a test file.\n", 18);

    if (close (fd)!=0)
    {
      fprintf (logFile, "b_ptr_stat - FILE_TEST close in stat_createfile\n");
      fclose(logFile);
      exit(99);
    }

    if((stat(TESTFILE, &stat_temp))!=0)
    {
      fprintf(logFile, "b_ptr_stat - FILE_TEST stat(TESTFILE failed)\n");
      fclose(logFile);
      exit(99);
    }
    fclose(logFile);
  }

{
  _theVariable = &stat_temp;
}
	
]

commit
[
]

cleanup
[
  FILE_TEST
  {
    remove(TESTFILE);  

    rmdir(TESTDIR);
  }
]
