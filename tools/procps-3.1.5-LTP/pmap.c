/*
 * Copyright 2002 by Albert Cahalan; all rights reserved.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "proc/version.h"  // FIXME: we need to link the lib for this :-(

static void usage(void) NORETURN;
static void usage(void){
  fprintf(stderr,
    "Usage: pmap [-r] [-x] pid...\n"
    "-x  show details\n"
  );
  exit(1);
}


static int V_option;
static int r_option;  // ignored -- for SunOS compatibility
static int x_option;


static const char *get_args(unsigned pid){
  static char cmdbuf[64];
  char buf[32];
  int fd;
  ssize_t count;

  do{
    sprintf(buf,"/proc/%u/cmdline",pid);
    if( (( fd=open(buf,O_RDONLY) )) == -1) break;
    count = read(fd, cmdbuf, sizeof(cmdbuf)-1);
    close(fd);
    if(count<1) break;
    cmdbuf[count] = '\0';
    if(!isprint(cmdbuf[0])) break;
    while(count--) if(!isprint(cmdbuf[count])) cmdbuf[count]=' ';
    return cmdbuf;
  }while(0);

  do{
    char *cp;
    sprintf(buf,"/proc/%u/stat",pid);
    if( (( fd=open(buf,O_RDONLY) )) == -1) break;
    count = read(fd, cmdbuf, sizeof(cmdbuf)-1);
    close(fd);
    if(count<1) break;
    cmdbuf[count] = '\0';
    while(count--) if(!isprint(cmdbuf[count])) cmdbuf[count]=' ';
    cp = strrchr(cmdbuf,')');
    if(!cp) break;
    cp[0] = ']';
    cp[1] = '\0';
    cp = strchr(cmdbuf,'(');
    if(!cp) break;
    if(!isprint(cp[1])) break;
    cp[0] = '[';
    return cp;
  }while(0);

  return "[]";  // as good as anything
}


static int one_proc(unsigned pid){
  char buf[32];
  char mapbuf[9600];
  unsigned long total_shared = 0ul;
  unsigned long total_private = 0ul;

  sprintf(buf,"/proc/%u/maps",pid);
  if(!freopen(buf, "r", stdin)) return 1;
  printf("%u:   %s\n", pid, get_args(pid));
  if(x_option)
    printf("Address       kB Resident Shared Private Permissions       Name\n");
  while(fgets(mapbuf,sizeof mapbuf,stdin)){
    char flags[32];
    const char *perms;
    char *tmp; // to clean up unprintables
    unsigned long start, end, diff;
    unsigned long long pgoff;
    sscanf(mapbuf,"%lx-%lx %s %Lx", &start, &end, flags, &pgoff);
    tmp = strchr(mapbuf,'\n');
    if(tmp) *tmp='\0';
    tmp = mapbuf;
    while(*tmp){
      if(!isprint(*tmp)) *tmp='?';
      tmp++;
    }
    
    if(flags[0]=='r'){
      if(flags[1]=='w'){
        if(flags[2]=='x') perms = "read/write/exec";
        else              perms = "read/write     ";
      }else{
        if(flags[2]=='x') perms = "read/exec      ";
        else              perms = "read           ";
      }
    }else{
      if(flags[1]=='w'){
        if(flags[2]=='x') perms = "write/exec     ";
        else              perms = "write          ";
      }else{
        if(flags[2]=='x') perms = "exec           ";
        else              perms = "none           ";
      }
    }
    diff = end-start;
    if(flags[3]=='s') total_shared  += diff;
    if(flags[3]=='p') total_private += diff;
    if(x_option){
      const char *cp = strrchr(mapbuf,'/');
      if(cp && cp[1]) cp++;
      if(!cp) cp = " [ anon ]";    // yeah, 1 space
      printf(
        (sizeof(long)==8)
          ? "%016lx %7ld       - %7ld %7ld %s   %s\n"
          : "%08lx %7ld       - %7ld %7ld %s   %s\n",
        start,
        diff>>10,
        (flags[3]=='s') ? diff>>10 : 0,
        (flags[3]=='p') ? diff>>10 : 0,
        perms,
        cp
      );
    }else{
      const char *cp = strchr(mapbuf,'/');
      if(!cp) cp = "  [ anon ]";   // yeah, 2 spaces
      printf(
        (sizeof(long)==8)
          ? "%016lx %6ldK %s   %s\n"
          : "%08lx %6ldK %s   %s\n",
        start,
        diff>>10,
        perms,
        cp
      );
    }
    
  }
  if(x_option){
    if(sizeof(long)==8){
      printf("----------------  ------  ------  ------  ------\n");
      printf(
        "total kB %15ld       - %7ld %7ld\n",
        (total_shared + total_private) >> 10,
        total_shared >> 10,
        total_private >> 10
      );
    }else{
      printf("--------  ------  ------  ------  ------\n");
      printf(
        "total kB %7ld       - %7ld %7ld\n",
        (total_shared + total_private) >> 10,
        total_shared >> 10,
        total_private >> 10
      );
    }
  }else{
    if(sizeof(long)==8) printf(" total %16ldK\n", (total_shared + total_private) >> 10);
    else                printf(" total %8ldK\n", (total_shared + total_private) >> 10);
  }
  return 0;
}


int main(int argc, char *argv[]){
  unsigned *pidlist;
  unsigned count = 0;
  unsigned u;
  int ret = 0;

  if(argc<2) usage();
  pidlist = malloc(sizeof(unsigned)*argc);  // a bit more than needed perhaps

  while(*++argv){
    if(!strcmp("--version",*argv)){
      V_option++;
      continue;
    }
    if(**argv=='-'){
      char *walk = *argv;
      if(!walk[1]) usage();
      while(*++walk){
        switch(*walk){
        case 'V':
          V_option++;
          break;
        case 'x':
          x_option++;
          break;
        case 'r':
          r_option++;
          break;
        default:
          usage();
        }
      }
    }else{
      char *walk = *argv;
      char *endp;
      unsigned long pid;
      if(!strncmp("/proc/",walk,6)) walk += 6;
      if(*walk<'0' || *walk>'9') usage();
      pid = strtoul(walk, &endp, 0);
      if(pid<1ul || pid>0x7ffffffful || *endp) usage();
      pidlist[count++] = pid;
    }
  }

  if(x_option>1 || V_option>1 || r_option>1) usage();  // dupes
  if(V_option){
    if(count|x_option|r_option) usage();
    fprintf(stdout, "pmap (%s)\n", procps_version);
    return 0;
  }
  if(count<1) usage();   // no processes

  u=0;
  while(u<count) ret |= one_proc(pidlist[u++]);

  return ret;
}
