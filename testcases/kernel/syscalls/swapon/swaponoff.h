
#ifndef __SWAP_ON_OFF_H_
#define __SWAP_ON_OFF_H_

/*
 * Read swapon(2) / swapoff(2) for a full history lesson behind the value of
 * MAX_SWAPFILES.
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
#define MAX_SWAPFILES 30
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 10)
#define MAX_SWAPFILES 32
#else
#define MAX_SWAPFILES 8
#endif

#endif
