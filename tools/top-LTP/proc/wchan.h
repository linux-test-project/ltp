#ifndef PROCPS_PROC_WCHAN_H
#define PROCPS_PROC_WCHAN_H

#include "procps.h"

EXTERN_C_BEGIN

extern const char * wchan(unsigned long address, unsigned pid);
extern int   open_psdb(const char *restrict override);
extern int   open_psdb_message(const char *restrict override, void (*message)(const char *, ...));

EXTERN_C_END

#endif
