#
# Software Inject library: Software inject specific functions
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

if [ -n "$this_case" ]; then
    bcase=$(basename $this_case)
fi
mcelog_result=$RDIR/$this_case/mcelog
klog=$RDIR/$this_case/klog

soft_inject_verify_mcelog()
{
    # check mcelog
    if [ -f $RDIR/$this_case/mcelog ]; then
	if [ -f $SDIR/refer/$bcase ]; then
	    mcelog_refer=$SDIR/refer/$bcase
	else
	    mcelog_refer=$SDIR/data/$bcase
	fi
	mce_reformat $mcelog_refer $RDIR/$this_case/mcelog_refer

	if mce_cmp $RDIR/$this_case/mcelog $mcelog_refer "$removes"; then
	    echo "  Passed: MCE log is ok"
	else
	    echo "  Failed: MCE log is different from input"
	fi
    else
	echo "  Failed: no MCE log result"
    fi
}

soft_inject_get_klog()
{
    local klog=$RDIR/$this_case/klog
    local orig_klog=$RDIR/$this_case/orig_klog

    if [ -f $klog ]; then
	cp $klog $orig_klog
	filter_fake_panic $orig_klog $klog
    fi
}

soft_inject_get_mcelog()
{
    case "$driver" in
	simple*)
	    get_mcelog_from_dev $mcelog_result
	    ;;
	kdump*)
	    get_mcelog_from_klog $klog $mcelog_result
	    ;;
	*)
	    echo '!!! Unsupported driver !!!'
    esac
}

# verify return value
soft_inject_verify_return_val()
{
    if [ -f $RDIR/$this_case/return ] && \
	[ $(cat $RDIR/$this_case/return) -eq 0 ]; then
	echo "  Passed: inject process continue"
    else
	echo "  Failed: inject process killed"
    fi
}

soft_inject_verify_kill()
{
    if [ -f $RDIR/$this_case/return ] && \
	[ $(cat $RDIR/$this_case/return) -eq 139 ]; then
	echo "  Passed: inject process killed!"
    else
	echo "  Failed: Not killed"
    fi
}

soft_inject_enumerate()
{
    local base=$(relative_path ${CDIR} ${SDIR})
    [ -z "$base" ] && die "BUG!!! Please contact your software vendor!"
    for c in $(cd $SDIR/data; ls *[^~]); do
	echo $base/$c
    done
}

soft_inject_trigger()
{
    mcelog &> /dev/null
    case "$driver" in
	kdump*)
	    mce-inject --no-random $SDIR/data/$bcase
	    ;;
	*)
	    mce-inject $SDIR/data/$bcase
	    ;;
    esac
    ret=$?
    echo $ret > $RDIR/$this_case/return
    sleep 1
}

soft_inject_verify_panic()
{
    local mce_panic="$1"
    verify_panic_via_klog $klog "$mce_panic"
}

soft_inject_verify_timeout()
{
    verify_timeout_via_klog $klog
}

soft_inject_verify_exp()
{
    verify_exp_via_klog $klog "$@"
}

soft_inject_main()
{
    op="$1"
    shift

    case "$op" in
	enumerate)
	    enumerate
	    ;;
	trigger)
	    trigger "$@"
	    ;;
	get_result)
	    get_result
	    ;;
	verify)
	    verify
	    ;;
	*)
	    die "Usage: $0 enumerate|trigger|get_result|verify"
    esac
    exit 0
}
