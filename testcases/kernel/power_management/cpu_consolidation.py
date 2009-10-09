#!/usr/bin/python
''' This Python script interprets various sched stats values.
    Validates cpu consolidation for given sched_mc_power_saving value
'''

import os
import sys
import time
LIB_DIR = "%s/lib" % os.path.dirname(__file__)
sys.path.append(LIB_DIR)
from optparse import OptionParser
from sched_mc import *

__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv

    usage = "-w"
    parser = OptionParser(usage)
    parser.add_option("-v", "--variation_test", dest="vary_mc_smt",
        default=False, action="store_true", help="Vary sched_mc & sched_smt. \
            -c and -t inputs are initial value of sched_mc & sched_smt")
    parser.add_option("-c", "--mc_value", dest="mc_value",
        default=0, help="Sched mc power saving value 0/1/2")
    parser.add_option("-t", "--smt_value", dest="smt_value",
        default=0, help="Sched smt power saving value 0/1/2")
    parser.add_option("-w", "--workload", dest="work_ld",
        default="ebizzy", help="Workload can be ebizzy/kernbench")
    parser.add_option("-s", "--stress", dest="stress",
        default="partial", help="Load on system is full/partial [i.e 50%]/thread")
    (options, args) = parser.parse_args()

    try:
        count_num_cpu()
        count_num_sockets()
        # User would set option -v / -vc / -vt to test cpu consolidation
        # gets disabled when sched_mc &(/) sched_smt is disabled when
        # workload is already running in the system 
        if options.vary_mc_smt:

            # Since same code is used for testing package consolidation and core
            # consolidation is_multi_socket & is_hyper_threaded check is done
            if is_multi_socket():
                if options.mc_value:
                    set_sched_mc_power(options.mc_value)
                    mc_value=int(options.mc_value)
                else:    
                    set_sched_mc_power(1)
                    mc_value=int(options.mc_value)
            if is_hyper_threaded():
                if options.smt_value:
                    set_sched_smt_power(options.smt_value)
                    smt_value=int(options.smt_value)
                else:
                    set_sched_smt_power(1)
                    smt_value=1

            #Generate arguments for trigger workload, run workload in background
            map_cpuid_pkgid()
            background="yes"
            duration=360
            pinned="no"
            if int(options.mc_value) < 2:
                trigger_ebizzy (smt_value, "partial", duration, background, pinned)
                work_ld="ebizzy"
                #Wait for 120 seconds and then validate cpu consolidation works
                #When sched_mc & sched_smt is set
                import time
                time.sleep(120)
            else:
                #Wait for 120 seconds and then validate cpu consolidation works
                #When sched_mc & sched_smt is set
                trigger_kernbench (smt_value, "partial", background, pinned) 
                work_ld="kernbench"
                import time
                time.sleep(240)

            generate_report()
            status = validate_cpu_consolidation(work_ld, mc_value, smt_value)
            if status == 0:
                print "INFO: Consolidation worked sched_smt &(/) sched_mc is set"
                #Disable sched_smt & sched_mc interface values
                if (options.vary_mc_smt and options.mc_value) and is_multi_socket():
                    set_sched_mc_power(0)
                    #Reset sched_smt bcoz when sched_smt is set process still
                    #continue to consolidate
                    if is_hyper_threaded():
                        set_sched_smt_power(0)
                if (options.vary_mc_smt and options.smt_value) and is_hyper_threaded():
                    set_sched_smt_power(0)
                time.sleep(120)
                generate_report()
                status = validate_cpu_consolidation(options.work_ld,options.mc_value, options.smt_value)
                #CPU consolidation should fail as sched_mc &(/) sched_smt is disabled
                if status == 1:
                    return(0)
                else:
                    return(1)
            else:
                print "INFO: CPU consolidation failed when sched_mc &(/) \
sched_smt was enabled. This is pre-requisite to proceed"
                return(status)
        else:           
            #The else part of the code validates behaviour of sched_mc
            # and sched_smt set to 0, 1 & 2
            if is_multi_socket():
                set_sched_mc_power(options.mc_value)
            if is_hyper_threaded():
                set_sched_smt_power(options.smt_value)
                #Commented after observing changes in behaviour in 2.6.31-rc7
                #stress="thread"
            map_cpuid_pkgid()
            print "INFO: Created table mapping cpu to package"
            background="no"
            duration=60
            pinned ="no"
            trigger_workld( options.smt_value, options.work_ld, options.stress, duration, background, pinned)
            generate_report()
            status = validate_cpu_consolidation(options.work_ld,options.mc_value, options.smt_value)
            reset_schedmc()
            if is_hyper_threaded():
                reset_schedsmt()
            return(status)        
    except Exception, details:
        print "INFO: CPU consolidation failed", details
        return(1)

if __name__ == "__main__":
    sys.exit(main())
