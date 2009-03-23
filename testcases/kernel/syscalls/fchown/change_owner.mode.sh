#!/bin/sh

failed() {
	echo ""
	echo "             ************** WARNING **************"
	echo "    Cannot change permission or ownership of \"change_owner\"."
	echo "               Tests in this directory will fail"$
	echo "                       Run "make install" as root."
	echo "             *************************************"
	sleep 2
}

chown root change_owner || failed
chmod 04755 change_owner || failed

exit 0
