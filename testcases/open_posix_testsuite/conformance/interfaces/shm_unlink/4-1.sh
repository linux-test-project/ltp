#! /bin/sh
#
# Test that the reuse of the name subsequently causes shm_open() to fail
# if O_CREAT is not set even if the object continues to exist after the last
# shm_unlink()
#
# This is tested implicitly via assertions 1 and 2.

echo "Tested implicitly via assertions 1 and 2."
exit 0
