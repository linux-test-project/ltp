#!/bin/sh

failed() {
	echo ""
	echo "             ************** WARNING **************"
	echo "    Cannot change permission or ownership of \"move_pages11\"."
	echo "                   Test move_pages11 will fail"
	echo "                   Run "make install" as root."
	echo "             *************************************"
	sleep 2
}

if [ -f move_pages11 ]; then
    chown root move_pages11 || failed
    chmod 04755 move_pages11 || failed
fi

exit 0
