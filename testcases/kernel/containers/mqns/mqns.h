#ifndef __MQNS_H
#define __MQNS_H

#include <sys/mount.h>
#include <mqueue.h>
#include "test.h"
#include "linux_syscall_numbers.h"
#include "libclone.h"

#define DEV_MQUEUE "/dev/mqueue"
#define DEV_MQUEUE2 "/dev/mqueue2"
#define SLASH_MQ1 "/MQ1"
#define NOSLASH_MQ1 "MQ1"
#define SLASH_MQ2 "/MQ2"
#define NOSLASH_MQ2 "MQ2"

#endif /* __MQNS_H */
