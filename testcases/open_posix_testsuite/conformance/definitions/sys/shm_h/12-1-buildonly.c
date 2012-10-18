/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that all of the folowing symbolic constants from sys/ipc.h are defined:
 *  IPC_CREAT
 *  IPC_EXCL
 *  IPC_NOWAIT
 *  IPC_PRIVATE
 *  IPC_RMID
 *  IPC_SET
 *  IPC_STAT
 */

#include <sys/shm.h>

#ifndef IPC_CREAT
#error IPC_CREAT not defined
#endif

#ifndef IPC_EXCL
#error IPC_EXCL not defined
#endif

#ifndef IPC_NOWAIT
#error IPC_NOWAIT not defined
#endif

#ifndef IPC_PRIVATE
#error IPC_PRIVATE not defined
#endif

#ifndef IPC_RMID
#error IPC_RMID not defined
#endif

#ifndef IPC_SET
#error IPC_SET not defined
#endif

#ifndef IPC_STAT
#error IPC_STAT not defined
#endif
