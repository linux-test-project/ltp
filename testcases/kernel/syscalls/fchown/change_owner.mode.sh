#!/bin/sh
whoami | grep root > /dev/null
if [ $? -eq 0 ];then
  chown root change_owner
  chmod 04755 change_owner
else
 echo ""
 echo "		         ************** WARNING **************"
 echo "		Cannot change permission or ownership of \"change_owner\"."
 echo "		           Tests in this directory will fail" 
 echo "                       Run "make install" as root."
 echo "		         *************************************"
 sleep 2
fi

