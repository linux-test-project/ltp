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
#include <sys/types.h>
#include <string.h>
#include "procps.h"
#include "escape.h"
#include "readproc.h"

// What it would be for a UTF-8 locale:
// "Z-------------------------------"
// "********************************"
// "********************************"
// "*******************************-"
// "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"  Trailing UTF-8, and badness in 8-bit.
// "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy"  Trailing UTF-8, and safe in 8-bit.
// "--222222222222222222222222222222"
// ".333333333333.3.44444444555566--"  The '.' means '3', with problem chars.
//
// Problems include non-shortest forms, UTF-16, and non-characters.
// The 4-byte, 5-byte, and 6-byte sequences are full of trouble too.

#if 0
/* sanitize a string, without the nice BSD library function:     */
/* strvis(vis_args, k->ki_args, VIS_TAB | VIS_NL | VIS_NOSLASH)  */
int octal_escape_str(char *restrict dst, const char *restrict src, size_t n){
  unsigned char c;
  char d;
  size_t i;
  const char codes[] =
  "Z------abtnvfr-------------e----"
  " *******************************"  /* better: do not print any space */
  "****************************\\***"
  "*******************************-"
  "--------------------------------"
  "********************************"
  "********************************"
  "********************************";
  for(i=0; i<n;){
    c = (unsigned char) *(src++);
    d = codes[c];
    switch(d){
    case 'Z':
      goto leave;
    case '*':
      i++;
      *(dst++) = c;
      break;
    case '-':
      if(i+4 > n) goto leave;
      i += 4;
      *(dst++) = '\\';
      *(dst++) = "01234567"[c>>6];
      *(dst++) = "01234567"[(c>>3)&07];
      *(dst++) = "01234567"[c&07];
      break;
    default:
      if(i+2 > n) goto leave;
      i += 2;
      *(dst++) = '\\';
      *(dst++) = d;
      break;
    }
  }
leave:
  *(dst++) = '\0';
  return i;
}
#endif

/* sanitize a string via one-way mangle */
int escape_str(char *restrict dst, const char *restrict src, int bufsize, int maxglyphs){
  unsigned char c;
  int my_glyphs = 0;
  int my_bytes = 0;
  const char codes[] =
  "Z-------------------------------"
  "********************************"
  "********************************"
  "*******************************-"
  "--------------------------------"
  "********************************"
  "********************************"
  "********************************";

  if(bufsize > maxglyphs+1) bufsize=maxglyphs+1; // FIXME: assumes 8-bit locale

  for(;;){
    if(my_glyphs >= maxglyphs) break;
    if(my_bytes+1 >= bufsize) break;
    c = (unsigned char) *(src++);
    if(!c) break;
    if(codes[c]=='-') c='?';
    my_glyphs++;
    my_bytes++;
    *(dst++) = c;
  }
  *(dst++) = '\0';
  return my_bytes;        // bytes of text, excluding the NUL
}

/////////////////////////////////////////////////

// escape an argv or environment string array
//
// bytes arg means sizeof(buf)
int escape_strlist(char *restrict dst, const char *restrict const *restrict src, size_t bytes){
  size_t i = 0;

//if(!*src){        just never call this function without checking first
//  do something nice
//}

  for(;;){
    i += escape_str(dst+i, *src, bytes-i, bytes-i);   // FIXME: byte/glyph
    if(bytes-i < 3) break;  // need room for space, a character, and the NUL
    src++;
    if(!*src) break;  // need something to print
    dst[i++] = ' ';
  }
  return i;    // bytes of text, excluding the NUL
}

///////////////////////////////////////////////////

int escape_command(char *restrict const outbuf, const proc_t *restrict const pp, int bytes, int glyphs, unsigned flags){
  int overhead = 1;  // the trailing NUL
  int end = 0;

  if(bytes > glyphs+1) bytes=glyphs+1; // FIXME: assumes 8-bit locale

  if(flags & ESC_ARGS){
    const char **lc = (const char**)pp->cmdline;
    if(lc && *lc) return escape_strlist(outbuf, lc, bytes);
  }
  if(flags & ESC_BRACKETS){
    overhead += 2;
  }
  if(flags & ESC_DEFUNCT){
    if(pp->state=='Z') overhead += 10;    // chars in " <defunct>"
    else flags &= ~ESC_DEFUNCT;
  }
  if(overhead >= bytes){  // if no room for even one byte of the command name
    // you'd damn well better have _some_ space
    outbuf[0] = '-';
    outbuf[1] = '\0';
    return 1;
  }
  if(flags & ESC_BRACKETS){
    outbuf[end++] = '[';
  }
  end += escape_str(outbuf+end, pp->cmd, bytes-overhead, glyphs-overhead+1);

  // Hmmm, do we want "[foo] <defunct>" or "[foo <defunct>]"?
  if(flags & ESC_BRACKETS){
    outbuf[end++] = ']';
  }
  if(flags & ESC_DEFUNCT){
    memcpy(outbuf+end, " <defunct>", 10);
    end += 10;
  }

  outbuf[end] = '\0';
  return end;  // bytes or glyphs, not including the NUL
}
