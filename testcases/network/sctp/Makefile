SUBDIRS =  lib accept bind bindx close connect listen read recv recvfrom recvmsg send sendmsg sendto setsockopt socket write

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

