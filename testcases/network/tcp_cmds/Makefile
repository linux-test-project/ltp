SUBDIRS = arp echo finger ftp host netstat perf_lan ping rcp rdist rlogin rsh rwho sendfile tcpdump telnet

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done


