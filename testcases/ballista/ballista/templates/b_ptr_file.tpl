// b_ptr_file.tpl : Ballista Datatype Template for a FILE pointer
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

name FILE* b_ptr_file;

parent b_ptr_void;

includes
[
{
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <errno.h>
#include "b_ptr_void.h"
}
]

global_defines
[
{
#define TESTDIR "testdir"
#define TESTFILE        "testdir/testfile_fp"
}
]

dials
[
  //Note removed X_GRP, RWX_ALL, RWX_UG to speed up run
  enum_dial MODE : READ, WRITE, APPEND, READ_PLUS, WRITE_PLUS, APPEND_PLUS;
  enum_dial PERMISSIONS : R_USER, W_USER, X_USER, RWX_USER, R_GRP, NONE;
  enum_dial EXISTANCE : EXIST, CLOSED, DELETED;   
  enum_dial STATE : EMPTY, BEGINNING, EOF, PAST_EOF;   
]

access
[
{
   char fileMode[2];
   mode_t permissionMode;
   int oflag;
   int pos;
   int empty_flag = 0;

   struct stat stat_buf_temp;
   int fd;

   // Setup log file for template information

   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
   //fprintf (logFile, "\nNEW TEST \n");

   if (mkdir(TESTDIR,S_IRWXU|S_IRWXG|S_IRWXO)!=0) /* create test directory, u+rwx */   
   {  
     //if the directory already exists ignore the error
     if (errno != EEXIST) 
     {
       fprintf(logFile,"mkdir failed. Function not tested\n");
       fclose(logFile);
       exit(99);
     }
   }
   //remove the file, ignore error if already removed
   if (unlink (TESTFILE) != 0 )
   {
     if (errno !=  ENOENT) 
     {
       fprintf(logFile,"unlink failed. Function not tested\n");
       fclose(logFile);
       exit(99);
     }
   }

   if((fd = open (TESTFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644))==-1)
   {
     fprintf(logFile,"b_ptr_file datatype not setup properly - open failed. Function not tested\n");
     fclose(logFile);
     exit(99);
   }
}

   EMPTY
   {
     empty_flag = 1;
   }

   READ, READ_PLUS, APPEND, APPEND_PLUS
   {    
     if (! empty_flag)
     {
       write (fd, "I am a test file.\n", 18);
     }
   }

{  // all values
   if (close (fd)!=0) 
   {
     fprintf(logFile,"b_ptr_file datatype not setup properly - close failed. Function not tested\n");
     fclose(logFile);
     exit(99);	
   }
}
  // using the "default" oflag values used in open to correspond with fopen values.
  READ
  {
    fileMode[0] = 'r';
    oflag = O_RDONLY;   
  }
  WRITE
  {
    fileMode[0] = 'w';
    oflag = O_WRONLY | O_CREAT | O_TRUNC;
  }
  APPEND
  {
    fileMode[0] = 'a';
    oflag = O_WRONLY | O_CREAT | O_APPEND;
  }
  READ_PLUS
  {
    fileMode[0] = 'r';
    fileMode[1] = '+';
    oflag = O_RDWR;    
  }
  WRITE_PLUS
  {
    fileMode[0] = 'w';
    fileMode[1] = '+';
    oflag = O_RDWR | O_CREAT | O_TRUNC;
  }
  APPEND_PLUS
  {
    fileMode[0] = 'a';
    fileMode[1] = '+';
    oflag = O_RDWR | O_CREAT | O_APPEND;
  }


  R_USER
  {
    permissionMode = S_IRUSR;
  } 
  W_USER
  {
    permissionMode = S_IWUSR;
  }
  X_USER
  {
    permissionMode = S_IXUSR;
  }
  RWX_USER
  {
    permissionMode = S_IRWXU;
  }
  R_GRP
  {
    permissionMode = S_IRGRP;
  }
//  X_GRP 
//  {
//    permissionMode = S_IXGRP;
//  }
//  RWX_ALL
//  {
//    permissionMode = S_IRWXU | S_IRWXG | S_IRWXO;
//  } 
//  RWX_UG
//  {
//    permissionMode = S_IRWXU | S_IRWXG;
//  }
  NONE
  {
    permissionMode = 0;
  }


{
  if ((fd  = open(TESTFILE, oflag, permissionMode)) == -1)
  {
    fprintf(logFile,"b_ptr_file datatype not setup properly - open failed\n");
    fclose(logFile);
    exit (99);
  }
}   

  WRITE, WRITE_PLUS
  {
    if (! empty_flag)
    {
      write (fd, "I am a test file.\n", 18);
    }
  }


  BEGINNING
  {
    if ((pos = lseek(fd, 0L, SEEK_SET)) == -1L)
    {
      fprintf(logFile,"b_ptr_file datatype not setup properly - lseek SET failed\n");
      fclose(logFile);
      exit (99);     
    }
  }
  EOF
  {
    if ((pos = lseek(fd, 0L, SEEK_END)) == -1L)
    {
      fprintf(logFile,"b_ptr_file datatype not setup properly - lseek END failed\n");
      fclose(logFile);
      exit (99);
    }
  }
  PAST_EOF
  {
    if ((pos = lseek(fd, 10L, SEEK_END)) == -1L)
    {
      fprintf(logFile,"b_ptr_file datatype not setup properly - lseek END failed\n");
      fclose(logFile);
      exit (99);
    }
  }

{  // generic statement for all values
  _theVariable = fdopen(fd,fileMode);
  if (_theVariable == NULL)
  {
    fprintf(logFile,"b_ptr_file datatype not setup properly - fdopen TESTFILE failed. Function not tested\n");
    fclose(logFile);
    exit(99);
  }

  fclose (logFile);
}
]

commit
[
{
   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
}

  CLOSED
  {
    fclose(_theVariable);
  }

  DELETED
  {
    // try deleting without closing the file first
    char buffer[128];
    if ((sprintf(buffer, "rm %s", TESTFILE)) <= 0)
    {
      fprintf(logFile,"b_ptr_file datatype not setup properly - rm of the TESTFILE failed. Function not tested\n");
      fclose(logFile);
      exit(99);
    }
    system(buffer);
    // system("ls testdir/testfil* >> testdir/deleteDebug.txt");
  }
]

cleanup
[
{  
   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
} 

  CLOSED, EXIST
  {
    chown(TESTFILE, getuid(), getgid()); //attempt to change ownership
    chmod(TESTFILE,  S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH);//permits others to read/write; ignore errors
    if (fclose (_theVariable)!=0) 
    {       //ignore error if file already closed
      if (errno != EBADF) perror("close fname_tempfd");
    }
  }

{
  if (unlink(TESTFILE)!=0)
  {
    //ignore error if already removed.
    if (errno != ENOENT) 
    {
      fprintf(logFile,"unlink fname_testfilename\n");
    }
  }

  //also try to rmdir it because it may be a directory. ignore error messages
  rmdir(TESTFILE);
  fclose(logFile);
}
]
