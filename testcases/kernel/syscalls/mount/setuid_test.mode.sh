#!/bin/sh
whoami | grep root > /dev/null
if [ $? -eq 0 ];then
	chown root:root setuid_test
        chmod 04511 setuid_test
else
 echo ""
 echo "		         ************** WARNING **************"
 echo "		Cannot change permission or ownership of \"setuid_test\"."
 echo "		           Tests in this directory will fail" 
 echo "                       Run "make install" as root."
 echo "		         *************************************"
 sleep 2
fi

