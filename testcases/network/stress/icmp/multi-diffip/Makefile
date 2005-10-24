SCRIPTS = icmp4-multi-diffip01 \
	  icmp4-multi-diffip02 icmp4-multi-diffip03 \
	  icmp4-multi-diffip04 icmp4-multi-diffip05 \
	  icmp4-multi-diffip06 icmp4-multi-diffip07 \
	  icmp6-multi-diffip01 \
	  icmp6-multi-diffip02 icmp6-multi-diffip03 \
	  icmp6-multi-diffip04 icmp6-multi-diffip05 \
	  icmp6-multi-diffip06 icmp6-multi-diffip07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
