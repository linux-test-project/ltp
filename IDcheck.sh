#!/bin/sh

id nobody
if [ $? -eq "0" ]; then
  id bin
  if [ $? -eq "0" ]; then
    id daemon
    if [ $? -eq "0" ]; then
      id -g nobody
      if [ $? -eq "0" ]; then
        id -g bin
        if [ $? -eq "0" ]; then
          id -g daemon
          if [ $? -eq "0" ]; then
            echo "Required users/groups exist."
            exit 0
          fi
        fi
      fi
    fi
  fi
fi
echo ""
echo "************************************************************************"
echo "* Please check that the required users/groups exist. See INSTALL file. *"
echo "************************************************************************"
echo ""
