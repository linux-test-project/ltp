#!/bin/sh
whoami | grep root > /dev/null
if [ $? -eq 0 ];then
  chmod 755 $1
else
 echo ""
 echo "		         ************** WARNING **************"
 echo "		Cannot change permission or ownership of \"$1\"."
 echo "		           Tests in this directory will fail." 
 echo "		              Run "make install" as root." 
 echo "		         *************************************"
 sleep 2
fi

