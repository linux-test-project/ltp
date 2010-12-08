#!/bin/bash -xe
#
# Kdump test driver: test case will trigger panic, and then crash
# dump. The test result is collected via dumped vmcore. For more
# information about kdump dirver please refer to doc/README.kdump.
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is based on kdump test case in LTP.
#
# This file is released under the GPLv2.
#

sd=$(dirname "$0")
export ROOT=`(cd $sd/../..; pwd)`

export driver=kdump

. $ROOT/lib/functions.sh
setup_path
. $ROOT/lib/dirs.sh
. $ROOT/lib/mce.sh

setup_crontab ()
{
    echo "Setup crontab."

    set +e
    crontab -r
    set -e

    # crontab in some distros will not read from STDIN.

    cat <<EOF > $WDIR/kdump.cron
SHELL=/bin/bash
PATH=/usr/local/bin:/usr/bin:/usr/sbin:/sbin:/bin
MAILTO=root
@reboot cd "$(pwd)"; ${0} $conf >> $WDIR/log 2>&1; cat $WDIR/log > /dev/console
EOF

    crontab $WDIR/kdump.cron

    echo "Enable cron daemon by default."

    if [ -f /etc/init.d/crond ]; then
        cron=crond
    else
        # SUSE
        cron=cron
    fi

    # Red Hat and SUSE.
    if [ -x "/sbin/chkconfig" ]; then
        /sbin/chkconfig "${cron}" on

    # Debian and Ubuntu.
    elif [ -x "/sbin/update-rc.d" ]; then
        /sbin/update-rc.d "${cron}" defaults
    fi
}

setup_kdump ()
{
    echo "Start kdump daemon."

    if [ -f /etc/init.d/kdump ]; then
	    daemon=kdump
    else
	    #SLE11
	    daemon=boot.kdump
    fi

    /etc/init.d/"${daemon}" restart

    echo "Enable kdump daemon by default."
    # Red Hat and SUSE.
    if [ -x "/sbin/chkconfig" ]; then
        /sbin/chkconfig "${daemon}" on

    # Debian and Ubuntu.
    elif [ -x "/sbin/update-rc.d" ]; then
        /sbin/update-rc.d "${daemon}" defaults
    fi
}

get_klog()
{
    klog=$RDIR/$this_case/klog
    cat <<EOF > $WDIR/get_klog_gdb.cmd
dump memory $klog log_buf log_buf+log_end
EOF
    set +e
    gdb -batch -batch-silent -x $WDIR/get_klog_gdb.cmd $VMLINUX $vmcore \
	> /dev/null 2>&1
    ret=$?
    set -e
    if [ $ret -eq 0 -a -s $klog ]; then
	export klog
    else
	echo "  Failed: can not get kernel log"
	rm -rf $klog
    fi
}

dump_gcov()
{
    if [ -z "$GCOV" ]; then
	return
    fi
    if ! chk_gcov; then
	echo "gcov is not supported by kernel or there is no " \
	    "gcov utility installed"
	echo "disabling gcov support"
	unset GCOV
	return
    fi
    if [ -z "$KSRC_DIR" ]; then
	echo "  Failed: please set KSRC_DIR for GCOV"
	return
    else
	KSRC_DIR=$(set -P && cd "$KSRC_DIR" && pwd)
	if [ -z "$KSRC_DIR"]; then
	    echo "  Failed: Invalid KSRC_DIR, please check your setup"
	    return
	fi
    fi
    export KSRC_DIR
    local gcov_head_raw=$WDIR/dump_gcov_head_raw
    cat <<EOF > $WDIR/dump_gcov_gdb.cmd
dump value $gcov_head_raw (void *)gcov_info_head
EOF
    set +e
    gdb -batch -batch-silent -x $WDIR/dump_gcov_gdb.cmd $VMLINUX $vmcore \
	> /dev/null 2>&1
    ret=$?
    set -e
    if [ $ret -ne 0 -o ! -s $gcov_head_raw ]; then
	echo "  Failed: can not get kernel gcov_info_head"
	return
    fi
    wl=$(stat -c '%s' $gcov_head_raw)
    h=$(echo -n 0x; od -A n -t x$wl $gcov_head_raw | tr -d ' ')
    if ! gcovdump -g $h $vmcore &> /dev/null; then
	echo "  Failed: can not dump kernel gcov info"
	return
    fi
cat <<"EOF" > $WDIR/dump_gcov_deskew.sh
    fn="$1"
    bfn=$(basename "$fn")
    dbfn="${bfn##\.tmp_}"
    if [ -z "$dbfn" ]; then
        return
    fi
    d=$(dirname "$fn")
    cp $fn "$d/$dbfn"
EOF
    chmod +x $WDIR/dump_gcov_deskew.sh
    find $KSRC_DIR \( -name '*.tmp_*gcno' -o -name '*.tmp_*gcda' \) \
        -exec $WDIR/dump_gcov_deskew.sh \{\} \;

    export GCOV=dump
}

get_result ()
{
    vmcore=$(ls -t "${COREDIR}"/*/vmcore* 2>/dev/null | head -1)

    if [ -n "$vmcore" -a -f "$vmcore" ]; then
	export vmcore
	get_klog
	dump_gcov
    else
	echo "  Failed: can not get vmcore"
    fi

    export reboot=1

    $CDIR/$case_sh get_result
}

verify_case ()
{
    if [ -z "$vmcore" ]; then
	echo "  Failed: can not got vmcore"
    fi
    $CDIR/$case_sh verify
}

trigger_case ()
{
    # Be careful to define COREDIR.
    rm -rf "${COREDIR}"/*

    # Save STDIO buffers.
    sync
    $CDIR/$case_sh trigger
}

# Start test.
if [ $# -lt 1 ]; then
    die "Usage: $0 <config>"
fi

conf=$(basename "$1")

. $CONF_DIR/$conf

driver_prepare
set_tolerant 1
set_panic_on_oops 0

# Check mandatory variables.
if [ -z "${COREDIR}" ]; then
    die "Fail: some mandatory variables are missing from configuration file."
fi

# Reboot the machine first to take advantage of boot parameter
# changes.
if [ ! -f $WDIR/stamps/setupped ]; then
    echo "Setup test environment."

    setup_crontab

    $SDIR/setup.sh $CONF_DIR/$conf

    echo > $WDIR/stamps/setupped

    echo "System is going to reboot."
    /sbin/shutdown -r now
    sleep 60
    exit -1
fi

#if mce_inject is a module, it is ensured to have been loaded
if modinfo mce_inject > /dev/null 2>&1; then
    if ! lsmod | grep -q mce_inject; then
        if ! modprobe mce_inject; then
	    die "module mce_inject isn't supported ?"
        fi
    fi
fi

for case_sh in ${CASES}; do
    for this_case in $($CDIR/$case_sh enumerate); do
	export this_case
	_this_case=$(echo $this_case | tr '/' '_')

	if [ -f $WDIR/stamps/${_this_case}_done ]; then
	    continue
	fi

        # First Test.
	if [ ! -f $WDIR/stamps/first_test_checked ]; then
            echo "First test..."
            echo "Verify Boot Loader."
            if ! grep 'crashkernel=' /proc/cmdline; then
		die "Fail: error changing Boot Loader, no crashkernel=."
            fi
            setup_kdump
	    echo > $WDIR/stamps/first_test_checked
	fi

	if [ ! -f $WDIR/stamps/${_this_case}_triggered ]; then
	    echo > $WDIR/stamps/${_this_case}_triggered

	    mkdir -p $RDIR/$this_case
	    rm -rf $RDIR/$this_case/*
	    echo -e "\n$this_case:" | tee -a $RDIR/result

	    echo "Running current test $this_case."

	    trigger_case | tee -a $RDIR/result

	    triggering=1
	fi

	sleep 5

	if [ -z "$triggering" ]; then
            (get_result; verify_case) | tee -a $RDIR/result
	else
	    echo "  Failed: Failed to trigger kdump" | tee -a $RDIR/result
	fi
	echo > $WDIR/stamps/${_this_case}_done

        # Wait for system to fully boot and leave a chance for user to
        # stop test
	sleep 55
    done
done

echo "Test run complete" | tee -a $RDIR/result

# We are done.
# Reset.
crontab -r

exit 0
