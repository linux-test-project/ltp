SCRIPTS = icmp4-multi-diffnic01 \
	  icmp4-multi-diffnic02 icmp4-multi-diffnic03 \
	  icmp4-multi-diffnic04 icmp4-multi-diffnic05 \
	  icmp4-multi-diffnic06 icmp4-multi-diffnic07 \
	  icmp6-multi-diffnic01 \
	  icmp6-multi-diffnic02 icmp6-multi-diffnic03 \
	  icmp6-multi-diffnic04 icmp6-multi-diffnic05 \
	  icmp6-multi-diffnic06 icmp6-multi-diffnic07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
