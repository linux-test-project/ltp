#!/bin/sh

# Prompt user if ids/groups should be created
echo -n "If any required user ids and/or groups are missing, would you like these created? Y/N "
read ans
case $ans in
    Y*|y*)
        CREATE=1
	;;
    *)
        CREATE=0 
        ;;
esac

# Check ids and create if needed.  
NOBODY_ID=0
BIN_ID=0
DAEMON_ID=0
NOBODY_GRP=0
BIN_GRP=0
DAEMON_GRP=0

id nobody > /dev/null
if [ $? != "0" ]; then
 NOBODY_ID=1
fi

id bin > /dev/null
if [ $? != "0" ]; then
 BIN_ID=1
fi

id daemon > /dev/null
if [ $? != "0" ]; then
 DAEMON_ID=1
fi

id -g nobody > /dev/null
if [ $? != "0" ]; then
 NOBODY_GRP=1
fi
     
id -g bin > /dev/null
if [ $? != "0" ]; then
 BIN_GRP=1
fi

id -g daemon > /dev/null
if [ $? != "0" ]; then
 DAEMON_GRP=1
fi

if [ $NOBODY_ID != "1" ] && [ $NOBODY_GRP != "1" ]; then
  echo "Nobody user id and group exist."
else
  if [ $NOBODY_ID -eq "1" ] && [ $CREATE -eq "1" ]; then
    echo nobody:x:99:99:Nobody:: >> /etc/passwd
  fi
  if [ $NOBODY_GRP -eq "1" ] && [ $CREATE -eq "1" ]; then
    echo nobody:x:`id -u nobody`: >> /etc/group
  fi
fi
 
if [ $BIN_ID != "1" ] && [ $BIN_GRP != "1" ]; then
  echo "Bin user id and group exist."
else
  if [[ $BIN_ID -eq "1" ]] && [[ $CREATE -eq "1" ]]; then
    echo bin:x:1:1:bin:: >> /etc/passwd
  fi
  if [[ $BIN_GRP -eq "1" ]] && [[ $CREATE -eq "1" ]]; then
    echo bin:x:`id -u bin`:bin >> /etc/group
  fi
fi

if [[ $DAEMON_ID -ne "1" ]] && [[ $DAEMON_GRP -ne "1" ]]; then
  echo "Daemon user id and group exist."
else
  if [[ $DAEMON_ID -eq "1" ]] && [[ $CREATE -eq "1" ]]; then
    echo daemon:x:2:2:daemon:: >> /etc/passwd
  fi
  if [[ $DAEMON_GRP -eq "1" ]] && [[ $CREATE -eq "1" ]]; then
    echo daemon:x:`id -u daemon`:daemon >> /etc/group
  fi
fi

id nobody > /dev/null
if [ $? -eq "0" ]; then
  id bin > /dev/null
  if [ $? -eq "0" ]; then
    id daemon > /dev/null
    if [ $? -eq "0" ]; then
      id -g nobody > /dev/null
      if [ $? -eq "0" ]; then
        id -g bin > /dev/null
        if [ $? -eq "0" ]; then
          id -g daemon > /dev/null
          if [ $? -eq "0" ]; then
            echo "Required users/groups exist."
            exit 0
          fi
        fi
      fi
    fi
  fi
fi
echo "*****************************************"
echo "* Required users/groups do NOT exist!!! *"
echo "*                                       *"
echo "* Some kernel/syscall tests will FAIL!  *"
echo "*****************************************"
exit 1
