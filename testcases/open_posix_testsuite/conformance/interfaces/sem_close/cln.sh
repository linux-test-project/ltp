#!/bin/sh
rm -f /tmp/sem_* >/dev/null 2>&1
rm -f a.out >/dev/null 2>&1

for sem_id in `ipcs -s | awk '{print $2}' | grep -Ev "Semaphore|semid"` ; do
    ipcrm sem $sem_id > /dev/null 2>&1
done

