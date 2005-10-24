SCRIPTS = tcp4-multi-diffnic01 \
	  tcp4-multi-diffnic02 tcp4-multi-diffnic03 \
	  tcp4-multi-diffnic04 tcp4-multi-diffnic05 \
	  tcp4-multi-diffnic06 tcp4-multi-diffnic07 \
	  tcp4-multi-diffnic08 \
	  tcp4-multi-diffnic09 tcp4-multi-diffnic10 \
	  tcp4-multi-diffnic11 tcp4-multi-diffnic12 \
	  tcp4-multi-diffnic13 tcp4-multi-diffnic14 \
	  tcp6-multi-diffnic01 \
	  tcp6-multi-diffnic02 tcp6-multi-diffnic03 \
	  tcp6-multi-diffnic04 tcp6-multi-diffnic05 \
	  tcp6-multi-diffnic06 tcp6-multi-diffnic07 \
	  tcp6-multi-diffnic08 \
	  tcp6-multi-diffnic09 tcp6-multi-diffnic10 \
	  tcp6-multi-diffnic11 tcp6-multi-diffnic12 \
	  tcp6-multi-diffnic13 tcp6-multi-diffnic14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
