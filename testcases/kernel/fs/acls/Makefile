all:
ifneq ($(wildcard /usr/include/sys/xattr.h),)
	$(CC) -o acl_file_test acl_file_test.c
	$(CC) -o acl_link_test acl_link_test.c
else
	@echo File sys/xattr.h does not exist, attr tests cannot be build!
endif
install:
ifneq ($(wildcard /usr/include/sys/xattr.h),)
	ln -f acl_file_test ../../bin/
	ln -f acl_link_test ../../bin/
	ln -f acl_test01 ../../bin/
endif
clean:
	rm -f acl_file_test acl_link_test
