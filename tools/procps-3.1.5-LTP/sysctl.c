
/*
 * Sysctl 1.01 - A utility to read and manipulate the sysctl parameters
 *
 *
 * "Copyright 1999 George Staikos
 * This file may be used subject to the terms and conditions of the
 * GNU General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details."
 *
 * Changelog:
 *            v1.01:
 *                   - added -p <preload> to preload values from a file
 *
 * Changes by Albert Cahalan, 2002.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "proc/procps.h"
#include "proc/version.h"


// Proof that C++ causes brain damage:
typedef int bool;
static bool true  = 1;
static bool false = 0;

/*
 *    Globals...
 */

static const char PROC_PATH[] = "/proc/sys/";
static const char DEFAULT_PRELOAD[] = "/etc/sysctl.conf";
static bool PrintName;
static bool PrintNewline;
static bool IgnoreError;

/* error messages */
static const char ERR_UNKNOWN_PARAMETER[] = "error: Unknown parameter '%s'\n";
static const char ERR_MALFORMED_SETTING[] = "error: Malformed setting '%s'\n";
static const char ERR_NO_EQUALS[] = "error: '%s' must be of the form name=value\n";
static const char ERR_INVALID_KEY[] = "error: '%s' is an unknown key\n";
static const char ERR_UNKNOWN_WRITING[] = "error: unknown error %d setting key '%s'\n";
static const char ERR_UNKNOWN_READING[] = "error: unknown error %d reading key '%s'\n";
static const char ERR_PERMISSION_DENIED[] = "error: permission denied on key '%s'\n";
static const char ERR_OPENING_DIR[] = "error: unable to open directory '%s'\n";
static const char ERR_PRELOAD_FILE[] = "error: unable to open preload file '%s'\n";
static const char WARN_BAD_LINE[] = "warning: %s(%d): invalid syntax, continuing...\n";


static void slashdot(char *restrict p, char old, char new){
  p = strpbrk(p,"/.");
  if(!p) return;            /* nothing -- can't be, but oh well */
  if(*p==new) return;       /* already in desired format */
  while(p){
    char c = *p;
    if(c==old) *p=new;
    if(c==new) *p=old;
    p = strpbrk(p+1,"/.");
  }
}



/*
 *     Display the usage format
 *
 */
static int Usage(const char *restrict const name) {
   printf("usage:  %s [-n] [-e] variable ... \n"
          "        %s [-n] [-e] -w variable=value ... \n" 
          "        %s [-n] [-e] -a \n" 
          "        %s [-n] [-e] -p <file>   (default /etc/sysctl.conf) \n"
          "        %s [-n] [-e] -A\n", name, name, name, name, name);
   return -1;
}


/*
 *     Strip the leading and trailing spaces from a string
 *
 */
static char *StripLeadingAndTrailingSpaces(char *oneline) {
   char *t;

   if (!oneline || !*oneline)
      return oneline;

   t = oneline;
   t += strlen(oneline)-1;

   while ((*t==' ' || *t=='\t' || *t=='\n' || *t=='\r') && t!=oneline)
      *t-- = 0;

   t = oneline;

   while ((*t==' ' || *t=='\t') && *t!=0)
      t++;

   return t;
}



/*
 *     Read a sysctl setting 
 *
 */
static int ReadSetting(const char *restrict const name) {
   int rc = 0;
   char *restrict tmpname;
   char *restrict outname;
   char inbuf[1025];
   FILE *restrict fp;

   if (!name || !*name) {
      fprintf(stderr, ERR_INVALID_KEY, name);
      return -1;
   }

   /* used to open the file */
   tmpname = malloc(strlen(name)+strlen(PROC_PATH)+1);
   strcpy(tmpname, PROC_PATH);
   strcat(tmpname, name); 
   slashdot(tmpname+strlen(PROC_PATH),'.','/'); /* change . to / */

   /* used to display the output */
   outname = strdup(name);
   slashdot(outname,'/','.'); /* change / to . */

   fp = fopen(tmpname, "r");

   if (!fp) {
      switch(errno) {
      case ENOENT:
         if (!IgnoreError) {
            fprintf(stderr, ERR_INVALID_KEY, outname);
            rc = -1;
         }
         break;
      case EACCES:
         fprintf(stderr, ERR_PERMISSION_DENIED, outname);
         rc = -1;
         break;
      default:
         fprintf(stderr, ERR_UNKNOWN_READING, errno, outname);
         rc = -1;
         break;
      }
   } else {
      while(fgets(inbuf, 1024, fp)) {
         /* already has the \n in it */
         if (PrintName) {
            fprintf(stdout, "%s = %s", outname, inbuf);
         } else {
            if (!PrintNewline) {
              char *nlptr = strchr(inbuf,'\n');
              if(nlptr) *nlptr='\0';
            }
            fprintf(stdout, "%s", inbuf);
         }
      }
      fclose(fp);
   }

   free(tmpname);
   free(outname);
   return rc;
}



/*
 *     Display all the sysctl settings 
 *
 */
static int DisplayAll(const char *restrict const path, bool ShowTableUtil) {
   int rc = 0;
   int rc2;
   DIR *restrict dp;
   struct dirent *restrict de;
   char *restrict tmpdir;
   struct stat ts;

   dp = opendir(path);

   if (!dp) {
      fprintf(stderr, ERR_OPENING_DIR, path);
      rc = -1;
   } else {
      readdir(dp); readdir(dp);   /* skip . and .. */
      while (( de = readdir(dp) )) {
         tmpdir = (char *restrict)malloc(strlen(path)+strlen(de->d_name)+2);
         sprintf(tmpdir, "%s%s", path, de->d_name);
         rc2 = stat(tmpdir, &ts);       /* should check this return code */
         if (rc2 != 0) {
            perror(tmpdir);
         } else {
            if (S_ISDIR(ts.st_mode)) {
               strcat(tmpdir, "/");
               DisplayAll(tmpdir, ShowTableUtil);
            } else {
               rc |= ReadSetting(tmpdir+strlen(PROC_PATH));
            }
         }
         free(tmpdir);
      }
      closedir(dp);
   }
   return rc;
}


/*
 *     Write a sysctl setting 
 *
 */
static int WriteSetting(const char *setting) {
   int rc = 0;
   const char *name = setting;
   const char *value;
   const char *equals;
   char *tmpname;
   FILE *fp;
   char *outname;

   if (!name) {        /* probably don't want to display this err */
      return 0;
   } /* end if */

   equals = index(setting, '=');
 
   if (!equals) {
      fprintf(stderr, ERR_NO_EQUALS, setting);
      return -1;
   }

   value = equals + 1;      /* point to the value in name=value */   

   if (!*name || !*value || name == equals) { 
      fprintf(stderr, ERR_MALFORMED_SETTING, setting);
      return -2;
   }

   /* used to open the file */
   tmpname = malloc(equals-name+1+strlen(PROC_PATH));
   strcpy(tmpname, PROC_PATH);
   strncat(tmpname, name, (int)(equals-name)); 
   tmpname[equals-name+strlen(PROC_PATH)] = 0;
   slashdot(tmpname+strlen(PROC_PATH),'.','/'); /* change . to / */

   /* used to display the output */
   outname = malloc(equals-name+1);                       
   strncpy(outname, name, (int)(equals-name)); 
   outname[equals-name] = 0;
   slashdot(outname,'/','.'); /* change / to . */
 
   fp = fopen(tmpname, "w");

   if (!fp) {
      switch(errno) {
      case ENOENT:
         if (!IgnoreError) {
            fprintf(stderr, ERR_INVALID_KEY, outname);
            rc = -1;
         }
         break;
      case EACCES:
         fprintf(stderr, ERR_PERMISSION_DENIED, outname);
         rc = -1;
         break;
      default:
         fprintf(stderr, ERR_UNKNOWN_WRITING, errno, outname);
         rc = -1;
         break;
      }
   } else {
      fprintf(fp, "%s\n", value);
      fclose(fp);

      if (PrintName) {
         fprintf(stdout, "%s = %s\n", outname, value);
      } else {
         if (PrintNewline)
            fprintf(stdout, "%s\n", value);
         else
            fprintf(stdout, "%s", value);
      }
   }

   free(tmpname);
   free(outname);
   return rc;
}



/*
 *     Preload the sysctl's from the conf file
 *           - we parse the file and then reform it (strip out whitespace)
 *
 */
static int Preload(const char *restrict const filename) {
   FILE *fp;
   char oneline[257];
   char buffer[257];
   char *t;
   int n = 0;
   int rc = 0;
   char *name, *value;

   if (!filename || ((fp = fopen(filename, "r")) == NULL)) {
      fprintf(stderr, ERR_PRELOAD_FILE, filename);
      return -1;
   }

   while (fgets(oneline, 256, fp)) {
      oneline[256] = 0;
      n++;
      t = StripLeadingAndTrailingSpaces(oneline);

      if (strlen(t) < 2)
         continue;

      if (*t == '#' || *t == ';')
         continue;

      name = strtok(t, "=");
      if (!name || !*name) {
         fprintf(stderr, WARN_BAD_LINE, filename, n);
         continue;
      }

      StripLeadingAndTrailingSpaces(name);

      value = strtok(NULL, "\n\r");
      if (!value || !*value) {
         fprintf(stderr, WARN_BAD_LINE, filename, n);
         continue;
      }

      while ((*value == ' ' || *value == '\t') && *value != 0)
         value++;

      sprintf(buffer, "%s=%s", name, value);
      rc |= WriteSetting(buffer);
   }

   fclose(fp);
   return rc;
}



/*
 *    Main... 
 *
 */
int main(int argc, char **argv) {
   const char *me = (const char *)basename(argv[0]);
   bool SwitchesAllowed = true;
   bool WriteMode = false;
   int ReturnCode = 0;
   const char *preloadfile = DEFAULT_PRELOAD;

   PrintName = true;
   PrintNewline = true;
   IgnoreError = false;

   if (argc < 2) {
       return Usage(me);
   }

   argv++;

   for (; argv && *argv && **argv; argv++) {
      if (SwitchesAllowed && **argv == '-') {        /* we have a switch */
         if ((*argv)[1] && (*argv)[2]){       // don't yet handle "sysctl -ew"
              if (!strcmp("--help",*argv)) {
                 Usage(me);
                 exit(0);
              }
              if (!strcmp("--version",*argv)) {
                 fprintf(stdout, "sysctl (%s)\n",procps_version);
                 exit(0);
              }
              fprintf(stderr, ERR_UNKNOWN_PARAMETER, *argv);
              return Usage(me);
         }
         switch((*argv)[1]) {
         case 'b':
              /* This is "binary" format, which means more for BSD. */
              PrintNewline = false;
           /* FALL THROUGH */
         case 'n':
              PrintName = false;
           break;
         case 'e':
              IgnoreError = true;
           break;
         case 'w':
              SwitchesAllowed = false;
              WriteMode = true;
           break;
         case 'p':
              argv++;
              if (argv && *argv && **argv) {
                 preloadfile = *argv;
              }
              return Preload(preloadfile);
         case 'a': /* string and integer values (for Linux, all of them) */
         case 'A': /* the above, including "opaques" (would be unprintable) */
         case 'X': /* the above, with opaques completly printed in hex */
              SwitchesAllowed = false;
              return DisplayAll(PROC_PATH, ((*argv)[1] == 'a') ? false : true);
         case 'V':
              fprintf(stdout, "sysctl (%s)\n",procps_version);
              exit(0);
         case 'h':
         case '?':
              return Usage(me);
         default:
              fprintf(stderr, ERR_UNKNOWN_PARAMETER, *argv);
              return Usage(me);
         }
      } else {
         SwitchesAllowed = false;
         if (WriteMode)
            ReturnCode = WriteSetting(*argv);
         else
            ReturnCode = ReadSetting(*argv);
      }
   }

   return ReturnCode;
}


