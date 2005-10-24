SCRIPTS = icmp4-uni-basic01 \
	  icmp4-uni-basic02 icmp4-uni-basic03 \
	  icmp4-uni-basic04 icmp4-uni-basic05 \
	  icmp4-uni-basic06 icmp4-uni-basic07 \
	  icmp6-uni-basic01 \
	  icmp6-uni-basic02 icmp6-uni-basic03 \
	  icmp6-uni-basic04 icmp6-uni-basic05 \
	  icmp6-uni-basic06 icmp6-uni-basic07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
