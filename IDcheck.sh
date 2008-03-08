#!/bin/sh

#    Copyright (c) International Business Machines  Corp., 2001
#
#    This program is free software;  you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY;  without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#    the GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program;  if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#   FILE        : IDcheck.sh
#   DESCRIPTION : checks for req'd users/groups and will create them if requested.
#   HISTORY     :
#       11/28/2001 Robbie Williamson (robbiew@austin.ibm.com)
#               written
#	03/05/2002 Jay Huie	(wjh@us.ibm.com)
#		Changed script to only ask regarding creation of IDs if
#			necessary. Cleaner automation of the script and
#			most distos now have IDs already added



# Prompt user if ids/groups should be created
clear
echo "Checking for required user/group ids"
echo ""

# Check ids and create if needed.  
NOBODY_ID=0
BIN_ID=0
DAEMON_ID=0
NOBODY_GRP=0
BIN_GRP=0
DAEMON_GRP=0
USERS_GRP=0
SYS_GRP=0
I_AM_ROOT=0

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

grep -q ^nobody: /etc/group
if [ $? != "0" ]; then
 NOBODY_GRP=1
fi
     
grep -q ^bin: /etc/group
if [ $? != "0" ]; then
 BIN_GRP=1
fi

grep -q ^daemon: /etc/group
if [ $? != "0" ]; then
 DAEMON_GRP=1
fi

grep -q ^users: /etc/group
if [ $? != "0" ]; then
 USERS_GRP=1
fi

grep -q ^sys: /etc/group
if [ $? != "0" ]; then
 SYS_GRP=1
fi

whoami | grep root > /dev/null
if [ $? = "0" ]; then
 I_AM_ROOT=1
fi

if [ -n "$CREATE" ]; then
  echo "CREATE variable set to $CREATE ..."
else
  if [ $NOBODY_ID != "0" ] || [ $BIN_ID != "0" ] || [ $DAEMON_ID != "0" ] || [ $NOBODY_GRP != "0" ] || [ $BIN_GRP != "0" ] || [ $DAEMON_GRP != "0" ] || [ $USERS_GRP != "0" ] || [ $SYS_GRP != "0" ] && [ $I_AM_ROOT != "0" ];
  then
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
  fi
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
  if [ $BIN_ID -eq "1" ] && [ $CREATE -eq "1" ]; then
    echo bin:x:1:1:bin:: >> /etc/passwd
  fi
  if [ $BIN_GRP -eq "1" ] && [ $CREATE -eq "1" ]; then
    echo bin:x:`id -u bin`:bin >> /etc/group
  fi
fi

if [ $DAEMON_ID -ne "1" ] && [ $DAEMON_GRP -ne "1" ]; then
  echo "Daemon user id and group exist."
else
  if [ $DAEMON_ID -eq "1" ] && [ $CREATE -eq "1" ]; then
    echo daemon:x:2:2:daemon:: >> /etc/passwd
  fi
  if [ $DAEMON_GRP -eq "1" ] && [ $CREATE -eq "1" ]; then
    echo daemon:x:`id -u daemon`:daemon >> /etc/group
  fi
fi

if [ $USERS_GRP -ne "1" ]; then
  echo "Users group exists."
else
  if [ $CREATE -eq "1" ]; then
    echo users:x:100: >> /etc/group
  fi
fi

if [ $SYS_GRP -ne "1" ]; then
  echo "Sys group exists."
else
  if [ $CREATE -eq "1" ]; then
    echo sys:x:3: >> /etc/group
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
            grep users /etc/group | cut -d: -f1 | grep users > /dev/null
            if [ $? -eq "0" ]; then
              grep sys /etc/group | cut -d: -f1 | grep sys > /dev/null
              if [ $? -eq "0" ]; then
               echo "Required users/groups exist."
               exit 0
              fi
            fi
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
