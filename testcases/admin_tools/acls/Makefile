all:
	rm -f unistd.h
	ln -s /lib/modules/`uname -r`/build/include/asm/unistd.h ./unistd.h
	$(CC) -Wall -o acl_file_test acl_file_test.c
	$(CC) -Wall -o acl_link_test acl_link_test.c
install:
	ln -f acl_file_test ../../bin/
	ln -f acl_link_test ../../bin/
	ln -f acl_test01 ../../bin/	
clean:
