#! /bin/sh

cat > check_pi.c <<EOF
#define _GNU_SOURCE
#include <pthread.h>

int main ()
{
	pthread_mutexattr_t attr;
	return pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
}
EOF

gcc -o check_pi check_pi.c -lpthread 2> check_pi.err
status=$?

if [ $status -eq 0 ] && [ ! -s check_pi.err ] && [ -x check_pi ]; then
    echo yes
else
    echo no
fi

rm -f check_pi.c check_pi check_pi.err
