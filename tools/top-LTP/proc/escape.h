#ifndef PROCPS_PROC_ESCAPE_H
#define PROCPS_PROC_ESCAPE_H

//#include <stdio.h>
#include <sys/types.h>
#include "procps.h"
#include "readproc.h"

EXTERN_C_BEGIN

#define ESC_ARGS     0x1  // try to use cmdline instead of cmd
#define ESC_BRACKETS 0x2  // if using cmd, put '[' and ']' around it
#define ESC_DEFUNCT  0x4  // mark zombies with " <defunct>"

extern int escape_strlist(char *restrict dst, const char *restrict const *restrict src, size_t n);
extern int escape_str(char *restrict dst, const char *restrict src, int bufsize, int maxglyphs);
extern int octal_escape_str(char *restrict dst, const char *restrict src, size_t n);
extern int simple_escape_str(char *restrict dst, const char *restrict src, size_t n);

extern int escape_command(char *restrict const outbuf, const proc_t *restrict const pp, int bytes, int glyphs, unsigned flags);

EXTERN_C_END
#endif
