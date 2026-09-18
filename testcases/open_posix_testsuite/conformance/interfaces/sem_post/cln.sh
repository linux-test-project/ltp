#!/bin/sh
rm -f /tmp/mysem >/dev/null 2>&1
rm -f /tmp/sem_* >/dev/null 2>&1
rm -f a.out

for sem_id in `ipcs -s | awk '$1 == "key" { p=1; next; } p && NF == 5 {print $2}'` ; do
    ipcrm sem $sem_id > /dev/null 2>&1
done

