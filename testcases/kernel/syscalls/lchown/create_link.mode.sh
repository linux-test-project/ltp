#!/bin/sh
whoami | grep root > /dev/null
if [ $? -eq 0 ];then
  chown root create_link
  chmod 04755 create_link
else
 echo ""
 echo "		         ************** WARNING **************"
 echo "		Cannot change permission or ownership of \"create_link\"."
 echo "		           Tests in this directory will fail" 
 echo "                       Run "make install" as root."
 echo "		         *************************************"
 sleep 2
fi

