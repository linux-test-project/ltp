/*
 * proc01.c - Tests Linux /proc file reading.
 *
 * Copyright (C) 2001 Stephane Fillod <f4cfe@free.fr>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 * 
 */

#include <errno.h>         /* for errno */
#include <stdio.h>         /* for NULL */
#include <stdlib.h>        /* for malloc() */
#include <string.h>        /* for string function */
#include <limits.h>        /* for PATH_MAX */
#include <sys/types.h>     /* for opendir(), readdir(), closedir(), stat() */
#include <sys/stat.h>      /* for [l]stat() */
#include <dirent.h>        /* for opendir(), readdir(), closedir() */
#include <unistd.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"


#define MAX_BUFF_SIZE 65536


char *TCID="proc01";
int TST_TOTAL=1;
extern int Tst_count;

static int opt_verbose=0;
static int opt_procpath=0;
static char *opt_procpathstr;
static int opt_buffsize=0;
static char *opt_buffsizestr;

static char *procpath="/proc";
size_t buffsize=1024;

/* FIXME: would it overflow on 32bits systems with >4Go RAM (HIGHMEM) ? */
size_t total_read=0;
unsigned int total_obj=0;

void cleanup()
{
  /*
   * remove the tmp directory and exit
   */

  TEST_CLEANUP;

  tst_rmdir();

  tst_exit();
}

void setup()
{
  /*
   * setup a default signal hander and a
   * temporary working directory.
   */
  tst_sig(FORK, DEF_HANDLER, cleanup);

  TEST_PAUSE;

  tst_tmpdir();
}

void help()
{
  printf("  -b x    read byte count\n");
  printf("  -r x    proc pathname\n");
  printf("  -v      verbose mode\n");
}

/*
 * add the -m option whose parameter is the
 * pages that should be mapped.
 */
option_t options[] = 
{
  { "b:", &opt_buffsize, &opt_buffsizestr },
  { "r:", &opt_procpath, &opt_procpathstr },
  { "v", &opt_verbose, NULL },
  { NULL, NULL, NULL }
};


/*
 * NB: this function is recursive 
 * returns 0 if no error encountered, otherwise number of errors (objs)
 *
 * REM: Funny enough, while devloping this function (actually replacing
 *	streamed fopen by standard open), I hit a real /proc bug.
 *	On a 2.2.13-SuSE kernel, "cat /proc/tty/driver/serial" would fail
 *	with EFAULT, while "cat /proc/tty/driver/serial > somefile" wouldn't.
 *	Okay, this might be due to a slight serial misconfiguration, but still.
 *	Analysis with strace showed up the difference was on the count size
 *	of read (1024 bytes vs 4096 bytes). So I tested further..
 *	read count of 512 bytes adds /proc/tty/drivers to the list 
 *	of broken proc files, while 64 bytes reads removes 
 *	/proc/tty/driver/serial from the list. Interresting, isn't it?
 *	Now, there's a -b option to this test, so you can try your luck. --SF
 *
 * It's more fun to run this test it as root, as all the files will be accessible!
 * (however, be careful, there might be some bufferoverflow holes..)
 * reading proc files might be also a good kernel latency killer.
 */
int
readproc(const char *obj)
{
   int           ret_val = 0;       /* return value from this routine */
   DIR           *dir;              /* pointer to a directory */
   struct dirent *dir_ent;          /* pointer to directory entries */
   char          dirobj[PATH_MAX];  /* object inside directory to modify */
   struct stat   statbuf;           /* used to hold stat information */
   int fd;
   ssize_t nread;
   static char buf[MAX_BUFF_SIZE];  /* static kills reentrancy, but we don't care about the contents */

   /* Determine the file type */
   if ( lstat(obj, &statbuf) < 0 ) {
      /* permission denied is not considered as error */
      if (errno != EACCES) {
	tst_resm(TINFO, "%s: lstat: %s", obj, strerror(errno));
        return 1;
	}
      return 0;
   }

   /* prevent loops .. */
   if ( S_ISLNK(statbuf.st_mode) )
   	return 0;

   total_obj++;

   /* Take appropriate action, depending on the file type */
   if ( S_ISDIR(statbuf.st_mode) ) {
      /* object is a directory */

      /* Open the directory to get access to what is in it */
      if ( (dir = opendir(obj)) == NULL ) {
      	if (errno != EACCES) {
		tst_resm(TINFO, "%s: opendir: %s", obj, strerror(errno));
	        return 1;
	}
        return 0;
      }

      /* Loop through the entries in the directory */
      for ( dir_ent = (struct dirent *)readdir(dir);
            dir_ent != NULL;
            dir_ent = (struct dirent *)readdir(dir)) {

         /* Ignore "." or ".." */
         if ( !strcmp(dir_ent->d_name, ".") || !strcmp(dir_ent->d_name, "..") ||
	      !strcmp(dir_ent->d_name, "kcore"))
            continue;

	if (opt_verbose)
		printf("%s\n",dir_ent->d_name);

         /* Recursively call this routine to test the current entry */
         snprintf(dirobj, PATH_MAX, "%s/%s", obj, dir_ent->d_name);
         ret_val += readproc(dirobj);
      }

      /* Close the directory */
      closedir(dir);
      return  ret_val;
      }
      else	/* if it's not a dir, read it! */
      {
   	if ( !S_ISREG(statbuf.st_mode) )
   		return 0;

	/* is NONBLOCK enough to escape from FIFO's ? */
	fd = open(obj, O_RDONLY | O_NONBLOCK);
	if (fd<0) {
      		if (errno != EACCES) {
			tst_resm(TINFO, "%s: open: %s", obj, strerror(errno));
		        return 1;
		}
		return 0;
	}

	nread = 1;
	while (nread > 0) {
		nread = read(fd, buf, buffsize);
		if (nread < 0) {
			/* ignore no perm (not root) and no process (terminated) errors */
			if (errno != EACCES && errno != ESRCH) {
				tst_resm(TINFO, "%s: read: %s", obj, strerror(errno));
				close(fd);
		        	return 1;
			}
			close(fd);
			return 0;
		}
		if (opt_verbose) {
#ifdef DEBUG
			printf("%d", nread);
#endif
			printf(".");
		}

		total_read += nread;
	}
	close(fd);
	if (opt_verbose)
		printf("\n");
     }

   /*
    * Everything must have went ok.
    */
   return 0;
}



int main(int argc, char *argv[])
{
  char *msg;
  int lc;


  if ( (msg=parse_opts(argc, argv, options, help)) != (char *) NULL )
   tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

  if (opt_buffsize) {
  	size_t bs;
	bs = atoi(opt_buffsizestr);
	if (bs <= MAX_BUFF_SIZE)
  		buffsize = bs;
	else
   		tst_brkm(TBROK, cleanup, "Invalid arg for -b (max: %u): %s", MAX_BUFF_SIZE,opt_buffsizestr);
  }

  if (opt_procpath) {
	procpath = opt_procpathstr ;
  }

  setup();

  for (lc=0; TEST_LOOPING(lc); lc++)
  {
    Tst_count=0;

	TEST( readproc(procpath) );

    if ( TEST_RETURN != 0 )
    {
      tst_resm(TFAIL, "readproc() failed with %d errors.", TEST_RETURN);
	}
	else
	{
      		tst_resm(TPASS, "readproc() completed successfully, "
		"total read: %u bytes, %u objs", total_read, total_obj);
	}
  }

  	cleanup();
	return 0;
}

