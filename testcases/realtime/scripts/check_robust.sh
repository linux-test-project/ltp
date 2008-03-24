#! /bin/sh

cat > check_robust.c <<EOF
#define _GNU_SOURCE
#include <pthread.h>

int main ()
{
	pthread_mutexattr_t attr;
	return pthread_mutexattr_setrobust_np(&attr, 0);
}
EOF

gcc -o check_robust check_robust.c -lpthread 2> check_robust.err
status=$?

if [ $status -eq 0 ] && [ ! -s check_robust.err ] && [ -x check_robust ]; then
    echo yes
else
    echo no
fi

rm -f check_robust.c check_robust check_robust.err
