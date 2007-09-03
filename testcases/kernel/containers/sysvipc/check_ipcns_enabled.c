/*
 * Copyright 2007 IBM
 * Author: Rishikesh K Rajak <risrajak@in.ibm.com>
 *
 */

#include <sched.h>
#include <stdio.h>
#include "../libclone/libclone.h"
#include "test.h"

int dummy(void *v)
{
        return 0;
}
int main()
{
        void *childstack, *stack;
        int pid;

        if (tst_kvercmp(2,6,19) < 0)
                return 1;
        stack = malloc(getpagesize());
        if (!stack) {
                perror("malloc");
                return 2;
        }

        childstack = stack + getpagesize();

#ifdef __ia64__
        pid = clone2(dummy, childstack, getpagesize(), CLONE_NEWIPC, NULL, NULL, NULL, NULL);
#else
        pid = clone(dummy, childstack, CLONE_NEWIPC, NULL);
#endif

        if (pid == -1) 
                return 3;
        return 0;
}

