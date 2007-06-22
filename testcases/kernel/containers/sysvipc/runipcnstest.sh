#!/bin/sh

exit_code=0
echo "sysvipc tests"
for type in none clone unshare; do
      echo "**sysvipc $type"
      shmnstest $type
      if [ $? -ne 0 ]; then
              exit_code=$?
      fi
done
exit $exit_code
