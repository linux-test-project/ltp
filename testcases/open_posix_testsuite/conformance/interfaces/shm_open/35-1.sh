#! /bin/sh
#
# Test that the shm_open() function sets errno = EEXIST if O_CREAT and O_EXCL
# are set and the named shared memory object already exists.
#
# This is tested implicitly via assertion 22.

echo "Tested implicitly via assertion 22."
exit 0
