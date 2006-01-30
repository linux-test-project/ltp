JAVAC = javac
JAR = jar

DOTS = dots
PERFMON = perfmon
FRAMEWORK = framework
BASECASE = basecase
ADVCASE = advcase

all: $(DOTS) $(PERFMON)

dots: $(FRAMEWORK) $(BASECASE) $(ADVCASE)
	$(JAVAC) -d ../classes -classpath ../classes dots/framework/Dots.java
	cd ../classes;$(JAR) cf ../Dots.jar dots/framework dots/basecase dots/advcase

framework:
	mkdir -p ../classes/dots/framework
	cd dots/framework;make

basecase:
	mkdir -p ../classes/dots/basecase
	cd dots/basecase;make

advcase:
	mkdir -p ../classes/dots/advcase
	cd dots/advcase;make

perfmon:
	mkdir -p ../classes/dots/perfmon
	cd dots/perfmon;make
	cd ../classes;$(JAR) cf ../Perfmon.jar dots/perfmon

clean:
	rm -f ../Dots.jar
	rm -f ../Perfmon.jar
	cd dots/framework;make clean
	cd dots/basecase;make clean
	cd dots/advcase;make clean
	cd dots/perfmon;make clean
