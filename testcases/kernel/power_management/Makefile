################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################

all:
	@set -e; $(MAKE) check_kv_arch ;

check_kv_arch: check_kv_arch.c
	$(CC) -o $@ $< ../../../lib/tst_kvercmp.c -I../../../include

install:
	@set -e; ln -f check_kv_arch ../../bin/check_kv_arch; \
	ln -f  change_freq.sh ../../bin/change_freq.sh; \
	ln -f  change_govr.sh ../../bin/change_govr.sh; \
	ln -f  check_cpufreq_sysfs_files.sh ../../bin/check_cpufreq_sysfs_files.sh; \
	ln -f  check_config.sh ../../bin/check_config.sh; \
	ln -f  pwkm_load_unload.sh ../../bin/pwkm_load_unload.sh; \
	ln -f  pm_include.sh ../../bin/pm_include.sh; \
	ln -f  runpwtests.sh ../../bin/runpwtests.sh; \
	ln -f  test_sched_mc.sh ../../bin/test_sched_mc.sh; \
	ln -f  config_cpu_freq ../../bin/config_cpu_freq; \
	ln -f  config_cpu_idle ../../bin/config_cpu_idle; \
	ln -f  config_sched_mc ../../bin/config_sched_mc;
	ln -f  check_cpuidle_sysfs_files.sh ../../bin/check_cpuidle_sysfs_files.sh;
	chmod ugo+x *.sh
clean:
	rm -f check_kv_arch

