function usage()
{
	echo "must pass in name of shell script to convert as arg 1"
	exit
}

function make_sed_script()
{
	sedcmd=`which sed`
	cat > fixit.sed <<-EOF
		#!$sedcmd -f

		/.*\<setup\>.*[#].*\<setup\>/bdo_setup
		/.*\<setup\>.*["].*\<setup\>.*["]/bdo_setup
		/.*[#"].*\<setup\>/bskip_setup
		:do_setup
		s/\<setup\>/tc_setup/
		:skip_setup

		/.*\<executes\>.*[#].*\<executes\>/bdo_executes
		/.*\<executes\>.*["].*\<executes\>.*["]/bdo_executes
		/.*[#"].*\<executes\>/bskip_executes
		:do_executes
		s/\<executes\>/tc_exec_or_break/
		:skip_executes

		/.*\<exists\>.*[#].*\<exists\>/bdo_exists
		/.*\<exists\>.*["].*\<exists\>.*["]/bdo_exists
		/.*[#"].*\<exists\>/bskip_exists
		:do_exists
		s/\<exists\>/tc_exist_or_break/
		:skip_exists

		s/\<utility.source\>/tc_utils.source/
		s/\<local_setup\>/tc_local_setup/
		s/\<local_cleanup\>/tc_local_cleanup/
		s/\<register_tc\>/tc_register/
		s/\<passfail\>/tc_pass_or_fail/
		s/\<maybefail\>/tc_fail_if_bad/
		s/\<maybebrok\>/tc_break_if_bad/
		s/\<isroot\>/tc_root_or_break/

		s/\<add_tempuser\>/tc_add_user_or_break/
		s/\<add_tempgroup\>/tc_add_group_or_break/
		s/\<del_tempuser\>/tc_del_user_or_break/
		s/\<del_tempgroup\>/tc_del_group_or_break/
		s/\<busyboxcmd\>/tc_is_busybox/
		s/\<isfstype\>/tc_is_fs_type/
		s/\<tst_res.[[:space:]]*TINFO\>/tc_info/
		s/\<tst_res.[[:space:]]*TWARN\>/tc_warn/
	EOF
	chmod +x fixit.sed
}

function make_awk_script()
{
	today=`date +"%d %b %Y"`
	awkcmd=`which awk`
	cat > fixit.awk <<-EOF
		#!$awkcmd -f
		BEGIN { found_history="no" }
		/^#[[:space:]]*History/ { found_history="yes" }
		/^$/ || /^############*$/	{ if ( found_history == "yes" ) {
							print "#\t\t$today - ($USER) updated to tc_utils.source"
							found_history="no"
							}
						}
		{print \$0}
	EOF
	chmod +x fixit.awk
}

#
# main
#
[ -f "$1" ] || usage

[ -e "$1-orig" ] && {
        echo ""
        echo "Cowardly refusing to process $1 since $1-orig already exists!"
        echo ""
        exit 1
}

1>&2 grep -qw trap $1 && echo "Uses trap command. Please convert to new-style tc_local_cleanup"
1>&2 grep -q "my.*cleanup" $1 && echo "Uses my_cleanup.  Please convert to new-style tc_local_cleanup"
1>&2 grep -q "my.*setup" $1 && echo "Uses my_setup.  Please convert to new-style tc_local_setup"
1>&2 grep -qw "TBROK" $1 && echo "Uses TBROK. Please convert to use tc_break_if_bad"
1>&2 grep -q "\$TCNAME" $1 && echo "Uses \$TCNAME. This is probably redundant"

mv $1 $1-orig
make_sed_script                       
make_awk_script
./fixit.sed $1-orig | ./fixit.awk > $1
chmod --reference $1-orig $1
rm fixit.sed fixit.awk

