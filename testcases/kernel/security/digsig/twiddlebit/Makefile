CC=gcc


all: hw_signed swapbit
swapbit: swapbit.o
hw_signed: hw
	@bsign=`which bsign`; \
	if test -n $$bsign; then \
		echo "running bsign"; \
		$$bsign -s -P "--homedir=." hw; \
		cp hw hw_signed; \
	else \
		echo "No bsign found"; \
	fi;

clean:
	rm -f *.o hw swapbit hw_signed bsignoutput digsigoutput
