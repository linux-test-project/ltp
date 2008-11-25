#!/bin/sh

# Use this script tool to run RPC & TIRPC Test Suite
# (c) 2007 Bull S.A.S.
# By C. LACABANNE - cyril.lacabanne@bull.net
# creation : 2007-06-07 revision : 2007-06-11

draw_title()
{
	clear

	echo "******************************"
	echo "*** RPC & TIRPC Test Suite ***"
	echo "******************************"
	echo ""
}

draw_main_menu()
{
	echo "Main menu :"
	echo " 0. Cancel and exit"
	echo " 1. Run all RPC & TIRPC Test Suite"
	echo " 2. Run all RPC Test Suite"
	echo " 3. Run all TIRPC Test Suite"
	echo " 4. Choose and run a part"
	echo -n "Choice : "
}

draw_domain_menu()
{
	echo "Domains menu :"
	echo " 0. Cancel and exit"
	echo " 1. RPC"
	echo " 2. TI-RPC"
	echo " 3. All"
	echo -n "Choice : "
}

draw_cat_menu()
{
	MNLST=$*
	i=1
	echo "Categories menu :"
	echo " 0. Cancel and exit"
	for categ in $*
	do
		echo " $i. "$categ
		i=`expr $i + 1`
	done
	echo -n "Choice : "
}

draw_subcat_menu()
{
	echo "Sub-ategories menu :"
	echo " 0. Cancel and exit"
	echo " 1. All categories"
	
	if [ "$1" = "RPC" ]
	then
		i=2
		find ./ -name "*$2_lib.sh" | grep -v "tirpc" | while read fic
		do
			echo " "$i". "$fic | sed -e 's/\.\/rpc_//g' | sed -e s/"_"$2"_lib.sh"//g
			i=`expr $i + 1`
			scat=( "${scat[@]}" "$(echo $fic | sed -e 's/\.\/rpc_//g' | sed -e s/"_"$2"_lib.sh"//g)" )
		done
	fi
	if [ "$1" = "TIRPC" ]
	then
		i=2
		find ./ -name "*$2_lib.sh" | grep "tirpc" | while read fic
		do
			echo " "$i". "$fic | sed -e 's/\.\/tirpc_//g' | sed -e s/"_"$2"_lib.sh"//g
			i=`expr $i + 1`
			scat=( "${scat[@]}" "$(echo $fic | sed -e 's/\.\/tirpc_//g' | sed -e s/"_"$2"_lib.sh"//g)" )
		done
	fi
	if [ "$1" = "ALL" ]
	then
		echo ""
	fi
	
	echo -n "Choice : "
}

draw_instance_menu()
{
	echo "Test instances :"
	echo "0 to cancel and exit, leave blank to keep default"
	echo -n "Enter the number of instances to launch per test : "
}

draw_waytest_menu()
{
	echo "Mode to run test suite :"
	echo " 0 to cancel and exit, leave blank to keep default"
	echo " 1. Several couples of server/client programs"
	echo " 2. One server replying to several clients programs"
	echo -n "Choice : "
}

# Initialization
RUN=1
MNLEVEL=0
SAVEPWD=$PWD
cd ../../../..
export LTPROOT=${PWD}
cd $SAVEPWD
EXECSCRIPT=${LTPROOT}/testcases/network/rpc/rpc-tirpc-full-test-suite/rpc_ts_run.sh
SCRIPTDIR=scripts
SCRIPTCATNAME="*_lib.sh"
VERBOSE=0

export SCRIPTDIR

# Main process

# check for automation mode
for arg in $* 
do
	if [ "$arg" = "-v" ]
	then
		VERBOSE=1
	fi
	if [ "$arg" = "-all" ]
	then
		# run all rpc & tirpc test suite
		tbl=( $(find ./ -name "*basic*_lib.sh") )
	
		CMD="$EXECSCRIPT "
		
		for fic in ${tbl[*]}
		do
			CMD="$CMD -l $fic"
		done
		
		if [ $VERBOSE -eq 1 ]
		then
			CMD="$CMD -v"
		fi
		
		$CMD
		exit 0
	fi
        if [ "$arg" = "-allrpc" ]
        then
                # run all rpc test suite
                tbl=( $(find ./ -name "rpc*basic*_lib.sh") )

                CMD="$EXECSCRIPT "

                for fic in ${tbl[*]}
                do
                        CMD="$CMD -l $fic"
                done

                if [ $VERBOSE -eq 1 ]
                then
                        CMD="$CMD -v"
                fi

                $CMD
                exit 0
        fi
        if [ "$arg" = "-alltirpc" ]
        then
                # run all tirpc test suite
                tbl=( $(find ./ -name "tirpc*basic*_lib.sh") )

                CMD="$EXECSCRIPT "

                for fic in ${tbl[*]}
                do
                        CMD="$CMD -l $fic"
                done

                if [ $VERBOSE -eq 1 ]
                then
                        CMD="$CMD -v"
                fi

                $CMD
                exit 0
        fi

done

# manual mode
while [ $RUN -eq 1 ]
do
	draw_title
	echo "Welcome to this Test Suite Wizard."
	echo "Remarks : start \"rpc_ts_wizard.sh -all\" to run all tests automatically."
	echo "Remarks : add -v flag to use verbose mode for Test Suite"
	echo ""
	draw_main_menu
	
	read MM_CHOICE
	if [ $MM_CHOICE -eq 0 ]
	then
		RUN=0
		MNLEVEL=0
	fi
	if [ $MM_CHOICE -eq 1 ]
	then
		RUN=0
		MNLEVEL=1
		SCRIPTCATNAME="*_lib.sh"
	fi
	if [ $MM_CHOICE -eq 2 ]
	then
		RUN=0
		MNLEVEL=2
		SCRIPTCATNAME="rpc*_lib.sh"
	fi
	if [ $MM_CHOICE -eq 3 ]
	then
		RUN=0
		MNLEVEL=3
		SCRIPTCATNAME="tirpc*_lib.sh"
	fi
	if [ $MM_CHOICE -eq 4 ]
	then
		RUN=0
		MNLEVEL=4
	fi
done

if [ $MNLEVEL -eq 1 ]
then
	# run all test suite
	tbl=( $(find ./ -name "$SCRIPTCATNAME") )
	
	CMD="$EXECSCRIPT "
	
	for fic in ${tbl[*]}
	do
		CMD="$CMD -l $fic"
	done
	
	draw_title
	draw_instance_menu
	read NUMBER
	
	if [ -z $NUMBER ]
	then
		NUMBER=1
	fi
	
	if [ $NUMBER -eq 0 ]
	then
		MNLEVEL=0
	else
		draw_title
		draw_waytest_menu
		read MM_CHOICE
		
		TESTWAY="onetomany"
		
		if [ -z $MM_CHOICE ]
		then
			MM_CHOICE=2
		fi
		
		if [ $MM_CHOICE -eq 1 ]
		then
			TESTWAY="manycouple"
		fi
		
		if [ $MM_CHOICE -eq 0 ]
		then
			MNLEVEL=0
		fi
		
		if [ $VERBOSE -eq 1 ]
		then
			CMD="$CMD -v"
		fi
			
		if [ $MNLEVEL -ne 0 ]
		then
			#echo "./$CMD -m $TESTWAY -n $NUMBER"
			$CMD -m $TESTWAY -n $NUMBER
		fi
	fi
fi

if [ $MNLEVEL -eq 2 ]
then
        # run all test suite
        tbl=( $(find ./ -name "$SCRIPTCATNAME") )

        CMD="$EXECSCRIPT "

        for fic in ${tbl[*]}
        do
                CMD="$CMD -l $fic"
        done

        draw_title
        draw_instance_menu
        read NUMBER

        if [ -z $NUMBER ]
        then
                NUMBER=1
        fi
        if [ $NUMBER -eq 0 ]
        then
                MNLEVEL=0
        else
                draw_title
                draw_waytest_menu
                read MM_CHOICE

                TESTWAY="onetomany"

                if [ -z $MM_CHOICE ]
                then
                        MM_CHOICE=2
                fi

                if [ $MM_CHOICE -eq 1 ]
                then
                        TESTWAY="manycouple"
                fi

                if [ $MM_CHOICE -eq 0 ]
                then
                        MNLEVEL=0
                fi

                if [ $VERBOSE -eq 1 ]
                then
                        CMD="$CMD -v"
                fi

                if [ $MNLEVEL -ne 0 ]
                then
                        #echo "./$CMD -m $TESTWAY -n $NUMBER"
                        $CMD -m $TESTWAY -n $NUMBER
                fi
        fi
fi

if [ $MNLEVEL -eq 3 ]
then
        # run all test suite
        tbl=( $(find ./ -name "$SCRIPTCATNAME") )

        CMD="$EXECSCRIPT "

        for fic in ${tbl[*]}
        do
                CMD="$CMD -l $fic"
        done

        draw_title
        draw_instance_menu
        read NUMBER

        if [ -z $NUMBER ]
        then
                NUMBER=1
        fi

        if [ $NUMBER -eq 0 ]
        then
                MNLEVEL=0
        else
                draw_title
                draw_waytest_menu
                read MM_CHOICE

                TESTWAY="onetomany"

                if [ -z $MM_CHOICE ]
                then
                        MM_CHOICE=2
                fi

                if [ $MM_CHOICE -eq 1 ]
                then
                        TESTWAY="manycouple"
                fi

                if [ $MM_CHOICE -eq 0 ]
                then
                        MNLEVEL=0
                fi

                if [ $VERBOSE -eq 1 ]
                then
                        CMD="$CMD -v"
                fi

                if [ $MNLEVEL -ne 0 ]
                then
                        #echo "./$CMD -m $TESTWAY -n $NUMBER"
                        $CMD -m $TESTWAY -n $NUMBER
                fi
        fi
fi

if [ $MNLEVEL -eq 4 ]
then
	# run a part only
	RUN=1
	MNLEVEL=1
	
	# domain
	while [ $RUN -eq 1 ]
	do
		draw_title
		echo "Select a domain"
		draw_domain_menu
		read MM_CHOICE
		if [ $MM_CHOICE -eq 1 ]
		then
			RUN=0
			DOMAIN="RPC"
		fi
		if [ $MM_CHOICE -eq 2 ]
		then
			RUN=0
			DOMAIN="TIRPC"
		fi
		if [ $MM_CHOICE -eq 3 ]
		then
			RUN=0
			DOMAIN="ALL"
		fi
		if [ $MM_CHOICE -eq 0 ]
		then
			RUN=0
			MNLEVEL=0
		fi
	done
	
	if [ $MNLEVEL -eq 0 ]
	then
		# exit test suite
		echo "Tests suite canceled"
		exit 0
	fi
	
	# category
	CAT=( `cat inc/categories` )
	RUN=1
	MNLEVEL=1
	
	while [ $RUN -eq 1 ]
	do
		draw_title
		echo "Select a category for $DOMAIN domain"
		draw_cat_menu ${CAT[*]}
		read MM_CHOICE
		
		if [ $MM_CHOICE -eq 0 ]
		then
			RUN=0
			MNLEVEL=0
		else
			if [ -n "${CAT[`expr $MM_CHOICE - 1`]}" ]
			then
				RUN=0
				MNLEVEL=1
			fi
		fi
	done
	
	if [ $MNLEVEL -eq 0 ]
	then
		# exit test suite
		echo "Tests suite canceled"
		exit 0
	fi

	if [ $DOMAIN = "ALL" ]
	then
		CMD="$EXECSCRIPT "
		tbl=( $(find ./ -name "*${CAT[`expr $MM_CHOICE - 1`]}_lib.sh") )
		
		for fic in ${tbl[*]}
		do
			CMD="$CMD -l $fic"
		done
	else
		# sub category
		RUN=1
		MNLEVEL=0
		
		while [ $RUN -eq 1 ]
		do
			draw_title
			echo "Select a sub category for ${CAT[`expr $MM_CHOICE - 1`]} category for $DOMAIN domain"
			draw_subcat_menu $DOMAIN ${CAT[`expr $MM_CHOICE - 1`]}
			read MM_CHOICE_SUB
			
			if [ $DOMAIN = "RPC" ]
			then
				#echo "${CAT[`expr $MM_CHOICE - 1`]}"
				scat=( $(find ./ -name "*_${CAT[`expr $MM_CHOICE - 1`]}_lib.sh" | grep -v "tirpc" | while read fic
				do
					echo $fic | sed -e 's/\.\/rpc_//g' | sed -e s/"_"${CAT[`expr $MM_CHOICE - 1`]}"_lib.sh"//g
				done) )
			fi
			if [ $DOMAIN = "TIRPC" ]
			then
				#echo "${CAT[`expr $MM_CHOICE - 1`]}"
				scat=( $(find ./ -name "*_${CAT[`expr $MM_CHOICE - 1`]}_lib.sh" | grep "tirpc" | while read fic 
				do 
					echo $fic | sed -e 's/\.\/tirpc_//g' | sed -e s/"_"${CAT[`expr $MM_CHOICE - 1`]}"_lib.sh"//g 
				done) )
			fi
			
			CMD="$EXECSCRIPT "
			
			if [ $MM_CHOICE -eq 0 ]
			then
				RUN=0
				MNLEVEL=0
			fi
			
			if [ $MM_CHOICE_SUB -eq 1 ]
			then
				RUN=0
				MNLEVEL=1
				if [ $DOMAIN = "RPC" ]
				then
					tbl=( $(find ./ -name "*_${CAT[`expr $MM_CHOICE - 1`]}_lib.sh" | grep -v "tirpc" ) )
				fi
				if [ $DOMAIN = "TIRPC" ]
				then
					tbl=( $(find ./ -name "*_${CAT[`expr $MM_CHOICE - 1`]}_lib.sh" | grep "tirpc" ) )
				fi
			else
				if [ $DOMAIN = "RPC" ]
				then
					tbl=( $(find ./ -name "*${scat[`expr $MM_CHOICE_SUB - 2`]}_${CAT[`expr $MM_CHOICE - 1`]}_lib.sh" | grep -v "tirpc" ) )
				fi
				if [ $DOMAIN = "TIRPC" ]
				then
					tbl=( $(find ./ -name "*${scat[`expr $MM_CHOICE_SUB - 2`]}_${CAT[`expr $MM_CHOICE - 1`]}_lib.sh" | grep "tirpc" ) )
				fi
				
				if [ ${#tbl[*]} -ne 0 ]
				then
					RUN=0
					MNLEVEL=1
				fi
			fi
		
			for fic in ${tbl[*]}
			do
				CMD="$CMD -l $fic"
			done
			
		done
	fi
	
	# execute chosen tests
	draw_title
	draw_instance_menu
	read NUMBER
	
	if [ -z $NUMBER ]
	then
		NUMBER=1
	fi
	
	if [ $NUMBER -eq 0 ]
	then
		MNLEVEL=0
	else
		draw_title
		draw_waytest_menu
		read MM_CHOICE
		
		TESTWAY="onetomany"
		
		if [ -z $MM_CHOICE ]
		then
			MM_CHOICE=2
		fi
		
		if [ $MM_CHOICE -eq 1 ]
		then
			TESTWAY="manycouple"
		fi
		
		if [ $MM_CHOICE -eq 0 ]
		then
			MNLEVEL=0
		fi
		
		if [ $VERBOSE -eq 1 ]
		then
			CMD="$CMD -v"
		fi
			
		if [ $MNLEVEL -ne 0 ]
		then
			$CMD -m $TESTWAY -n $NUMBER
		fi
	fi
fi

if [ $MNLEVEL -eq 0 ]
then
	# exit test suite
	echo "Tests suite canceled"
	exit 0
fi

exit 1
