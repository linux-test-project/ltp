#!/usr/bin/python
''' This Python script validates sched domain information in dmesg
    with information in sysfs topology
'''

import os
import sys
LIB_DIR = "%s/testcases/kernel/power_management/lib" % os.environ['LTPROOT']
sys.path.append(LIB_DIR)
from sched_mc import *

__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

# Run test based on the command line arguments
if __name__ == "__main__":

    try:
        clear_dmesg()
        count_num_cpu()
        map_cpuid_pkgid()
        set_sched_mc_power(1)
        verify_sched_domain_dmesg(1)
        set_sched_mc_power(0)
        verify_sched_domain_dmesg(0)
    except:
        sys.exit(1)
