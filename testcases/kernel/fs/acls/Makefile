all:
	@rm -f unistd.h
	@cp /lib/modules/`uname -r`/build/include/asm/unistd.h ./unistd.h
	$(CC) -Wall -o acl_file_test acl_file_test.c
	$(CC) -Wall -o acl_link_test acl_link_test.c
install:
	@ln -f acl_file_test ../../../bin/acl_file_test
	@ln -f acl_link_test ../../../bin/acl_link_test
	@ln -f acl_test01 ../../../bin/acl_test01	
clean:
	rm -f acl_file_test
	rm -f acl_link_test
	rm ./unistd.h
