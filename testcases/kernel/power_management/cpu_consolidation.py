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
    parser.add_option("-c", "--mc_level", dest="mc_level",
        help="Sched mc power saving value 0/1/2")
    parser.add_option("-t", "--smt_level", dest="smt_level",
        default=0, help="Sched smt power saving value 0/1/2")
    (options, args) = parser.parse_args()
    test_thread_consld = 0
    

    try:
        set_sched_mc_power(options.mc_level)
        count_num_cpu()
        map_cpuid_pkgid()
        print "INFO: Created table mapping cpu to package"
        
        if int(options.smt_level) == 1 or int(options.smt_level) == 2 :
            if is_hyper_threaded():
                set_sched_smt_power(options.smt_level)
                # Trigger ebizzy with 2 threads only to verify logical CPU
                # consolidation
                test_thread_consld = 1
                trigger_workld(options.mc_level, test_thread_consld)
                generate_report()
                validate_cpu_consolidation(options.mc_level, options.smt_level)
                test_thread_consld = 0
            else:
                print "INFO: No Hyper-threading support in this machine"
                sys.exit(0)
    
        trigger_workld(options.mc_level, test_thread_consld)
        generate_report()
        validate_cpu_consolidation(options.mc_level, options.smt_level)
        sys.exit(0)
    except Exception, details:
        print "INFO(: CPU consolidation failed", details
        sys.exit(1)
