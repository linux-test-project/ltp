#ifndef PROC_STATUS_H
#define PROC_STATUS_H

#include "procps.h"

EXTERN_C_BEGIN

extern const char * status(const proc_t *restrict task);

EXTERN_C_END

#endif
