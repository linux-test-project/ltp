/*
 * Copyright 1998-2002 by Albert Cahalan; all rights resered.         
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysmacros.h>
#include "version.h"
#include "devname.h"

#ifndef PAGE_SIZE 
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#endif

/* Who uses what:
 *
 * tty_to_dev   oldps, w (there is a fancy version in ps)
 * dev_to_tty   oldps, top, ps
 */

typedef struct tty_map_node {
  struct tty_map_node *next;
  int major_number; /* not unsigned! Ugh... */
  int minor_first, minor_last;
  char name[16];
  char devfs_type;
} tty_map_node;

static tty_map_node *tty_map = NULL;

/* Load /proc/tty/drivers for device name mapping use. */
static void load_drivers(void){
  char buf[10000];
  char *p;
  int fd;
  int bytes;
  fd = open("/proc/tty/drivers",O_RDONLY);
  if(fd == -1) goto fail;
  bytes = read(fd, buf, sizeof(buf) - 1);
  if(bytes == -1) goto fail;
  buf[bytes] = '\0';
  p = buf;
  while(( p = strstr(p, " /dev/") )){
    tty_map_node *tmn;
    int len;
    char *end;
    p += 6;
    end = strchr(p, ' ');
    if(!end) continue;
    len = end - p;
    tmn = calloc(1, sizeof(tty_map_node));
    tmn->next = tty_map;
    tty_map = tmn;
    /* if we have a devfs type name such as /dev/tts/%d then strip the %d but
       keep a flag. */
    if(len >= 3 && !strncmp(end - 2, "%d", 2)){
      len -= 2;
      tmn->devfs_type = 1;
    }
    strncpy(tmn->name, p, len);
    p = end; /* set p to point past the %d as well if there is one */
    while(*p == ' ') p++;
    tmn->major_number = atoi(p);
    p += strspn(p, "0123456789");
    while(*p == ' ') p++;
    switch(sscanf(p, "%d-%d", &tmn->minor_first, &tmn->minor_last)){
    default:
      /* Can't finish parsing this line so we remove it from the list */
      tty_map = tty_map->next;
      free(tmn);
      break;
    case 1:
      tmn->minor_last = tmn->minor_first;
      break;
    case 2:
      break;
    }
  }
fail:
  if(fd != -1) close(fd);
  if(!tty_map) tty_map = (tty_map_node *)-1;
}

/* Try to guess the device name from /proc/tty/drivers info. */
static int driver_name(char *restrict const buf, int maj, int min){
  struct stat sbuf;
  tty_map_node *tmn;
  if(!tty_map) load_drivers();
  if(tty_map == (tty_map_node *)-1) return 0;
  tmn = tty_map;
  for(;;){
    if(!tmn) return 0;
    if(tmn->major_number == maj && tmn->minor_first <= min && tmn->minor_last >= min) break;
    tmn = tmn->next;
  }
  sprintf(buf, "/dev/%s%d", tmn->name, min);  /* like "/dev/ttyZZ255" */
  if(stat(buf, &sbuf) < 0){
    if(tmn->devfs_type) return 0;
    sprintf(buf, "/dev/%s", tmn->name);  /* like "/dev/ttyZZ255" */
    if(stat(buf, &sbuf) < 0) return 0;
  }
  if(min != minor(sbuf.st_rdev)) return 0;
  if(maj != major(sbuf.st_rdev)) return 0;
  return 1;
}

/* Try to guess the device name (useful until /proc/PID/tty is added) */
static int guess_name(char *restrict const buf, int maj, int min){
  struct stat sbuf;
  int t0, t1;
  int tmpmin = min;
  switch(maj){
  case   4:
    if(min<64){
      sprintf(buf, "/dev/tty%d", min);
      break;
    }
    if(min<128){  /* to 255 on newer systems */
      sprintf(buf, "/dev/ttyS%d", min-64);
      break;
    }
    tmpmin = min & 0x3f;  /* FALL THROUGH */
  case   3:      /* /dev/[pt]ty[p-za-o][0-9a-z] is 936 */
    t0 = "pqrstuvwxyzabcde"[tmpmin>>4];
    t1 = "0123456789abcdef"[tmpmin&0x0f];
    sprintf(buf, "/dev/tty%c%c", t0, t1);
    break;
  case  17:  sprintf(buf, "/dev/ttyH%d",  min); break;
  case  19:  sprintf(buf, "/dev/ttyC%d",  min); break;
  case  22:  sprintf(buf, "/dev/ttyD%d",  min); break; /* devices.txt */
  case  23:  sprintf(buf, "/dev/ttyD%d",  min); break; /* driver code */
  case  24:  sprintf(buf, "/dev/ttyE%d",  min); break;
  case  32:  sprintf(buf, "/dev/ttyX%d",  min); break;
  case  43:  sprintf(buf, "/dev/ttyI%d",  min); break;
  case  46:  sprintf(buf, "/dev/ttyR%d",  min); break;
  case  48:  sprintf(buf, "/dev/ttyL%d",  min); break;
  case  57:  sprintf(buf, "/dev/ttyP%d",  min); break;
  case  71:  sprintf(buf, "/dev/ttyF%d",  min); break;
  case  75:  sprintf(buf, "/dev/ttyW%d",  min); break;
  case  78:  sprintf(buf, "/dev/ttyM%d",  min); break; /* conflict */
  case 105:  sprintf(buf, "/dev/ttyV%d",  min); break;
  case 112:  sprintf(buf, "/dev/ttyM%d",  min); break; /* conflict */
  /* 136 ... 143 are /dev/pts/0, /dev/pts/1, /dev/pts/2 ... */
  case 136 ... 143:  sprintf(buf, "/dev/pts/%d",  min+(maj-136)*256); break;
  case 148:  sprintf(buf, "/dev/ttyT%d",  min); break;
  case 154:  sprintf(buf, "/dev/ttySR%d", min); break;
  case 156:  sprintf(buf, "/dev/ttySR%d", min+256); break;
  case 164:  sprintf(buf, "/dev/ttyCH%d",  min); break;
  case 166:  sprintf(buf, "/dev/ttyACM%d", min); break; /* bummer, 9-char */
  case 172:  sprintf(buf, "/dev/ttyMX%d",  min); break;
  case 174:  sprintf(buf, "/dev/ttySI%d",  min); break;
  case 188:  sprintf(buf, "/dev/ttyUSB%d", min); break; /* bummer, 9-char */
  default: return 0;
  }
  if(stat(buf, &sbuf) < 0) return 0;
  if(min != minor(sbuf.st_rdev)) return 0;
  if(maj != major(sbuf.st_rdev)) return 0;
  return 1;
}

/* Linux 2.2 can give us filenames that might be correct.
 * Useful names could be in /proc/PID/fd/2 (stderr, seldom redirected)
 * and in /proc/PID/fd/255 (used by bash to remember the tty).
 */
static int link_name(char *restrict const buf, int maj, int min, int pid, const char *restrict name){
  struct stat sbuf;
  char path[32];
  int count;
  sprintf(path, "/proc/%d/%s", pid, name);  /* often permission denied */
  count = readlink(path,buf,PAGE_SIZE-1);
  if(count == -1) return 0;
  buf[count] = '\0';
  if(stat(buf, &sbuf) < 0) return 0;
  if(min != minor(sbuf.st_rdev)) return 0;
  if(maj != major(sbuf.st_rdev)) return 0;
  return 1;
}

/* number --> name */
unsigned dev_to_tty(char *restrict ret, unsigned chop, int dev, int pid, unsigned int flags) {
  static char buf[PAGE_SIZE];
  char *restrict tmp = buf;
  unsigned i = 0;
  int c;
  if((short)dev == (short)0) goto no_tty;
  if(linux_version_code > LINUX_VERSION(2, 7, 0)){  // not likely to make 2.6.xx
    if(link_name(tmp, major(dev), minor(dev), pid, "tty"   )) goto abbrev;
  }
  if(driver_name(tmp, major(dev), minor(dev)               )) goto abbrev;
  if(  link_name(tmp, major(dev), minor(dev), pid, "fd/2"  )) goto abbrev;
  if( guess_name(tmp, major(dev), minor(dev)               )) goto abbrev;
  if(  link_name(tmp, major(dev), minor(dev), pid, "fd/255")) goto abbrev;
  // fall through if unable to find a device file
no_tty:
  strcpy(ret, "?");
  return 1;
abbrev:
  if((flags&ABBREV_DEV) && !strncmp(tmp,"/dev/",5) && tmp[5]) tmp += 5;
  if((flags&ABBREV_TTY) && !strncmp(tmp,"tty",  3) && tmp[3]) tmp += 3;
  if((flags&ABBREV_PTS) && !strncmp(tmp,"pts/", 4) && tmp[4]) tmp += 4;
  /* gotta check before we chop or we may chop someone else's memory */
  if(chop + (unsigned long)(tmp-buf) <= sizeof buf)
    tmp[chop] = '\0';
  /* replace non-ASCII characters with '?' and return the number of chars */
  for(;;){
    c = *tmp;
    tmp++;
    if(!c) break;
    i++;
    if(c<=' ') c = '?';
    if(c>126)  c = '?';
    *ret = c;
    ret++;
  }
  *ret = '\0';
  return i;
}

/* name --> number */
int tty_to_dev(const char *restrict const name) {
  struct stat sbuf;
  static char buf[32];
  if(name[0]=='/' && stat(name, &sbuf) >= 0) return sbuf.st_rdev;
  snprintf(buf,32,"/dev/%s",name);
  if(stat(buf, &sbuf) >= 0) return sbuf.st_rdev;
  snprintf(buf,32,"/dev/tty%s",name);
  if(stat(buf, &sbuf) >= 0) return sbuf.st_rdev;
  snprintf(buf,32,"/dev/pts/%s",name);
  if(stat(buf, &sbuf) >= 0) return sbuf.st_rdev;
  return -1;
}
