#!/bin/sh -xe

conf=${1}; shift
test=${1}
crasher=crasher
lkdtm=lkdtm

. "${conf}"

case "${test}" in

	"KEXEC-L")
                kexec -l /boot/vmlinuz --initrd=/boot/initrd \
                 --append="$(cat /proc/cmdline)"
                sleep 10
                kexec -e
                ;;
        "MNS")
                echo "Not implemented"
                ;;

        "MNN")
                echo "Not implemented"
                ;;

        "MCS")
                echo "Not implemented"
                ;;

        "MCN")
                echo "Not implemented"
                ;;

        "MCF")
                echo "Not implemented"
                ;;

	"ACS")
		echo c >/proc/sysrq-trigger
		;;

	"ACP")
		# Panic test 0 in crasher module: panic()
		insmod "${crasher}"/crasher.ko
		echo 0 >/proc/crasher
		;;

	"ACB")
		# Panic test 1 in crasher module: BUG()
		insmod "${crasher}"/crasher.ko
		echo 1 >/proc/crasher
		;;

	"ACE")
		# Panic test 2 in crasher module: panic_on_oops
		insmod "${crasher}"/crasher.ko
		echo 1 >/proc/sys/kernel/panic_on_oops
		echo 2 >/proc/crasher
		;;

	"ACL")
		# Panic test 3 in crasher module: hang w/double spinlock
		# requires nmi_watchdog be enabled
		insmod "${crasher}"/crasher.ko
		echo 3 >/proc/crasher
		;;

	"KPIDB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HARDWARE_ENTRY cpoint_type=BUG cpoint_count=05
		;;
	"KPIDE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HARDWARE_ENTRY cpoint_type=EXCEPTION cpoint_count=05
		;;
	"KPIDL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HARDWARE_ENTRY cpoint_type=LOOP cpoint_count=05
		;;
	"KPIDP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HARDWARE_ENTRY cpoint_type=PANIC cpoint_count=05
		;;
	"KPIDO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HARDWARE_ENTRY cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPIEB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HW_IRQ_EN cpoint_type=BUG cpoint_count=10
		;;
	"KPIEE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HW_IRQ_EN cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPIEL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HW_IRQ_EN cpoint_type=LOOP cpoint_count=10
		;;
	"KPIEP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HW_IRQ_EN cpoint_type=PANIC cpoint_count=10
		;;
	"KPIEO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_HW_IRQ_EN cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPTEB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_TASKLET_ENTRY cpoint_type=BUG cpoint_count=10
		;;
	"KPTEE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_TASKLET_ENTRY cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPTEL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_TASKLET_ENTRY cpoint_type=LOOP cpoint_count=10
		;;
	"KPTEP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_TASKLET_ENTRY cpoint_type=PANIC cpoint_count=10
		;;
	"KPTEO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=INT_TASKLET_ENTRY cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPBB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=FS_DEVRW cpoint_type=BUG cpoint_count=10
		;;
	"KPBE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=FS_DEVRW cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPBL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=FS_DEVRW cpoint_type=LOOP cpoint_count=10
		;;
	"KPBP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=FS_DEVRW cpoint_type=PANIC cpoint_count=10
		;;
	"KPBO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=FS_DEVRW cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPMSB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=MEM_SWAPOUT cpoint_type=BUG cpoint_count=10
		;;
	"KPMSE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=MEM_SWAPOUT cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPMSL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=MEM_SWAPOUT cpoint_type=LOOP cpoint_count=10
		;;
	"KPMSP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=MEM_SWAPOUT cpoint_type=PANIC cpoint_count=10
		;;
	"KPMSO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=MEM_SWAPOUT cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPTB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=TIMERADD cpoint_type=BUG cpoint_count=10
		;;
	"KPTE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=TIMERADD cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPTL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=TIMERADD cpoint_type=LOOP cpoint_count=10
		;;
	"KPTP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=TIMERADD cpoint_type=PANIC cpoint_count=10
		;;
	"KPTO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=TIMERADD cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPSB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=SCSI_DISPATCH_CMD cpoint_type=BUG cpoint_count=10
		;;
	"KPSE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=SCSI_DISPATCH_CMD cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPSL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=SCSI_DISPATCH_CMD cpoint_type=LOOP cpoint_count=10
		;;
	"KPSP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=SCSI_DISPATCH_CMD cpoint_type=PANIC cpoint_count=10
		;;
	"KPSO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=SCSI_DISPATCH_CMD cpoint_type=OVERFLOW cpoint_count=10
		;;
	"KPIB")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=IDE_CORE_CP cpoint_type=BUG cpoint_count=10
		;;
	"KPIE")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=IDE_CORE_CP cpoint_type=EXCEPTION cpoint_count=10
		;;
	"KPIL")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=IDE_CORE_CP cpoint_type=LOOP cpoint_count=10
		;;
	"KPIP")
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=IDE_CORE_CP cpoint_type=PANIC cpoint_count=10
		;;
	"KPIO")
		echo 1 >/proc/sys/kernel/panic_on_oops
		insmod "${lkdtm}"/lkdtm.ko cpoint_name=IDE_CORE_CP cpoint_type=OVERFLOW cpoint_count=01
		;;
        "KLEXT")
                echo "ext3 ${EXT3_PART}" >/etc/kdump.conf
                if [ "${FILTER}" -eq 1 ]; then
                        echo "core_collector makedumpfile ${MAKE_OPTIONS}" >>/etc/kdump.conf
                fi
                /etc/init.d/kdump restart
                echo c >/proc/sysrq-trigger
                ;;

        "KLLBL")
                echo "ext3 LABEL=${EXT3_LABEL}" >/etc/kdump.conf
                if [ "${FILTER}" -eq 1 ]; then
                        echo "core_collector makedumpfile ${MAKE_OPTIONS}" >>/etc/kdump.conf
                fi
                /etc/init.d/kdump restart
                echo c >/proc/sysrq-trigger
                ;;

        "KLUID")
                echo "ext3 UUID=${EXT3_UID}" >/etc/kdump.conf
                if [ "${FILTER}" -eq 1 ]; then
                        echo "core_collector makedumpfile ${MAKE_OPTIONS}" >>/etc/kdump.conf
                fi
                /etc/init.d/kdump restart
                echo c >/proc/sysrq-trigger
                ;;

        "KLRAW")
                echo "raw ${RAW_PART}" >/etc/kdump.conf
                if [ "${FILTER}" -eq 1 ]; then
                        echo "core_collector makedumpfile ${MAKE_OPTIONS}" >>/etc/kdump.conf
                fi
                /etc/init.d/kdump restart
                echo c >/proc/sysrq-trigger
                ;;

        "KNSCP")
                echo "net ${SCP_PATH}" >/etc/kdump.conf
                if [ "${LINK_DELAY}" ]; then
                    echo "link_delay ${LINK_DELAY}" >>/etc/kdump.conf
                fi

                expect -f ./ssh.tcl "/etc/init.d/kdump propagate" "${SCP_PASS}"
                if [ "${FILTER}" -eq 1 ]; then
                        echo "core_collector makedumpfile ${MAKE_OPTIONS}" >>/etc/kdump.conf
                fi
                /etc/init.d/kdump restart
                echo c >/proc/sysrq-trigger
                ;;

        "KNNFS")
                echo "net ${NFS_PATH}" >/etc/kdump.conf
                if [ "${LINK_DELAY}" ]; then
                    echo "link_delay ${LINK_DELAY}" >>/etc/kdump.conf
                fi

                if [ "${FILTER}" -eq 1 ]; then
                        echo "core_collector makedumpfile ${MAKE_OPTIONS}" >>/etc/kdump.conf
                fi
                /etc/init.d/kdump restart
                echo c >/proc/sysrq-trigger
                ;;

        "KDENB")
                echo "net ${SCP_PATH}" >/etc/kdump.conf
                expect -f ./ssh.tcl "/etc/init.d/kdump propagate" "${SCP_PASS}"
                /etc/init.d/kdump restart
                ;;

        *)
                echo "Unknown test."
                ;;

esac

exit 0
