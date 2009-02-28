#!/usr/bin/python
''' This Python script interprets various sched stats values.
    Validates cpu consolidation for given sched_mc_power_saving value
'''

import os
import sys
LIB_DIR = "%s/testcases/kernel/power_management/lib" % os.environ['LTPROOT']
sys.path.append(LIB_DIR)
from optparse import OptionParser
from sched_mc import *

__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

# Run test based on the command line arguments
if __name__ == "__main__":
    usage = "-w"
    parser = OptionParser(usage)
    parser.add_option("-w", "--workload", dest="workload",
        help="Test name that has be triggered")
    parser.add_option("-l", "--mc_level", dest="mc_level",
        help="Sched mc power saving value 0/1/2")
    (options, args) = parser.parse_args()

    try:
        set_sched_mc_power(options.mc_level)
     print "INFO: sched mc power saving set to %s" %options.mc_level
        count_num_cpu()
        map_cpuid_pkgid()
        print "INFO: Created table mapping cpu to package"
        trigger_workld(options.workload)
        generate_report()
        validate_cpu_consolidation()
        sys.exit(0)
    except:
        sys.exit(1)
