#!/bin/sh

failed() {
	echo ""
	echo "             ************** WARNING **************"
	echo "    Cannot change permission or ownership of \"move_pages03\"."
	echo "                   Test move_pages03 will fail"
	echo "                   Run "make install" as root."
	echo "             *************************************"
	sleep 2
}

if [ -f move_pages03 ]; then
    chown root move_pages03 || failed
    chmod 04755 move_pages03 || failed
fi

exit 0
