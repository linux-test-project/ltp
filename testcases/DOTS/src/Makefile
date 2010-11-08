JAVAC ?= javac
JAR ?= jar

DOTS = dots
PERFMON = perfmon
FRAMEWORK = framework
BASECASE = basecase
ADVCASE = advcase

all: $(DOTS) $(PERFMON)

dots: $(FRAMEWORK) $(BASECASE) $(ADVCASE)
	$(JAVAC) -d ../classes -classpath ../classes dots/framework/Dots.java
	cd ../classes; $(JAR) cf ../Dots.jar dots/framework dots/basecase dots/advcase

framework:
	mkdir -p ../classes/dots/framework
	cd dots/framework; $(MAKE)

basecase:
	mkdir -p ../classes/dots/basecase
	cd dots/basecase; $(MAKE)

advcase:
	mkdir -p ../classes/dots/advcase
	cd dots/advcase; $(MAKE)

perfmon:
	mkdir -p ../classes/dots/perfmon
	cd dots/perfmon; $(MAKE)
	cd ../classes;$(JAR) cf ../Perfmon.jar dots/perfmon

clean:
	rm -f ../Dots.jar
	rm -f ../Perfmon.jar
	cd dots/framework; $(MAKE) clean
	cd dots/basecase; $(MAKE) clean
	cd dots/advcase; $(MAKE) clean
	cd dots/perfmon; $(MAKE) clean
