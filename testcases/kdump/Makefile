CFLAGS = -Wall

all:
	./distro
	make -C tools all
	./setup

clean:
	make -C tools clean
	rm -f config control crontab testlists/lkdtm testfile
	mv -f /etc/sysconfig/kdump.orig /etc/sysconfig/kdump 2> /dev/null
	crontab -r
