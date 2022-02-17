#!/bin/sh -xe

SetupCrontab ()
{
    echo "Setup crontab."

    set +e
    crontab -r
    set -e

    # crontab in some distros will not read from STDIN.

    cat <<EOF >kdump.cron
SHELL=/bin/sh
PATH=/usr/bin:/usr/sbin:/sbin:/bin
MAILTO=root
@reboot cd "$(pwd)"; cd ..; ${0} >>/tmp/kdump-$(date +%F-%T).log 2>&1
EOF

    crontab kdump.cron

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

    # Debian and Ubuntu and Uos
    elif [ -x "/sbin/update-rc.d" ]; then
        /sbin/update-rc.d "${cron}" defaults
    fi
}

SetupKdump ()
{
    echo "Start kdump daemon."
    /etc/init.d/kdump restart

    echo "Enable kdump daemon by default."
    # Red Hat and SUSE.
    if [ -x "/sbin/chkconfig" ]; then
        /sbin/chkconfig kdump on

    # Debian and Ubuntu and Uos
    elif [ -x "/sbin/update-rc.d" ]; then
        /sbin/update-rc.d kdump defaults
    fi
}


PrepareVerify ()
{
    if [ "${last}" = "KLEXT" ]; then
        # If not mountable, skip it, and continue doing the test.
        set +e
        mount "${EXT3_PART}" /mnt
        set -e

        COREDIR=/mnt"${COREDIR}"

    elif [ "${last}" = "KLLBL" ]; then
        # If not mountable, skip it, and continue doing the test.
        set +e
        mount -L "${EXT3_LABEL}" /mnt
        set -e

        COREDIR=/mnt"${COREDIR}"

    elif [ "${last}" = "KLUID" ]; then
        # If not mountable, skip it, and continue doing the test.
        set +e
        mount "/dev/disk/by-uuid/${EXT3_UID}" /mnt
        set -e

        COREDIR=/mnt"${COREDIR}"

    elif [ "${last}" = "KLRAW" ]; then
        mkdir -p "${COREDIR}/${last}"

        # If not dumpable, skip it, and continue doing the test.
        set +e
        dd if="${RAW_PART}" of="${COREDIR}/${last}/vmcore" bs=1024
        set -e

    elif [ "${last}" = "KNSCP" ]; then
        if [ -z "${SCP_PATH}" ]; then
            echo "Fail: network destination not defined."
            exit 1
        fi

        file=$(ssh -i ~/.ssh/kdump_id_rsa "${SCP_PATH}" "ls -t ${COREDIR}/*/vmcore* \
         2>/dev/null | head -1")

        mkdir -p "${COREDIR}/${last}"

        if [ "${file}" ]; then
            # Not fatal error.
            set +e
            scp  -i ~/.ssh/kdump_id_rsa "${SCP_PATH}:${file}" "${COREDIR}/${last}"
            set -e
        fi

    elif [ "${last}" = "KNNFS" ]; then
        # Not fatal error.
        set +e
        mount "${NFS_PATH}" /mnt
        set -e

        COREDIR=/mnt"${COREDIR}"
    fi

    vmcore=$(ls -t "${COREDIR}"/*/vmcore* 2>/dev/null | head -1)
}

VerifyTest ()
{
    # Should not be here.
    if [ -z "${last}" ]; then
        echo "Should not be here!"
	echo "There must be something wrong with the test setup."
	exit 1
    fi

    echo "Verifying the result of previous test ${last}."
    ldir=$(ls -td "../${log}/$(hostname)."* | head -1)

    if [ -f "${vmcore}" ]; then
        echo "$(date +%F-%T): verification of test ${last} passed." \
        >>"${ldir}/status"

        ./verify.sh "../${conf}" "${vmcore}" "${CRASH}" \
        >>"${ldir}/${ITERATION}.${last}.$(date +%F-%T)"

        # Be careful to define COREDIR.
        rm -rf "${COREDIR}"/*

    else
        echo "$(date +%F-%T): verification of test ${last} failed:\
 vmcore NOT FOUND." >>"${ldir}/status"
        echo "vmcore NOT FOUND." \
        >>"${ldir}/${ITERATION}.${last}.$(date +%F-%T)"
    fi
}

RunTest ()
{

    sed -i "s/\(^REBOOT\)=.*/\1=$((count + 1))/" \
     "../${conf}"

    echo "Running current test ${i}."

    echo "$(date +%F-%T): running current test ${i}." \
    >> "${ldir}/status"

    # Save STDIO buffers.
    sync
    ./test.sh "../${conf}" "${i}" "../${log}"
}

# Start test.
conf="./runkdump.conf"
lib="lib"
log="log"

# Read test configuration file.
. "${conf}"

# Check mandatory variables.
if [ -z "${ITERATION}" ] || [ -z "${REBOOT}" ] || [ -z "${COREDIR}" ]
then
    echo "Fail: some mandatory variables are missing from\
 configuration file."
    exit 1
fi

cd "${lib}"

while [ "${ITERATION}" -ge 1 ]; do

# Reboot the machine first to take advantage of boot parameter
# changes.
if [ -z "${REBOOT}" ] || [ "${REBOOT}" -eq 0 ]; then
    echo "Setup test environment."

    SetupCrontab

    ./setup.sh "../${conf}"

    sed -i 's/\(^REBOOT\)=.*/\1=1/' "../${conf}"

    echo "System is going to reboot."
    /sbin/shutdown -r now
    sleep 60

else
    count=1

    for i in ${CRASHER} ${BASIC_LKDTM} ${EXTRA_LKDTM} ${EXTRA_DUMP} \
             END; do

        if [ "${count}" -eq "${REBOOT}" ]; then
            # Wait for machine fully booted.
            sleep 60

            # First Test.
            if [ "${REBOOT}" -eq 1 ]; then
                echo "First test..."
                echo "Verify Boot Loader."
                if ! grep 'crashkernel=' /proc/cmdline; then
                    echo "Fail: error changing Boot Loader, no crashkernel=."
                    exit 1
                fi

                SetupKdump

                # Creat log directory.
                mkdir -p "../${log}/$(hostname).$(date +%F-%T)"

                echo "Gather system information."

                ldir=$(ls -td "../${log}/$(hostname)."* | head -1)
                ./sysinfo.sh >"${ldir}/system.info"

            else
                PrepareVerify

                VerifyTest

		if [ "${i}" = END ]; then
		    # We are done.
		    break
		fi

            fi

            RunTest

            # Some tests could not reboot target. They can hung up
            # machine or leave it working. But we need to do all
            # tests. So we are going to reboot if we are in wrong
            # place.

            sleep 3600
            echo "$(date +%F-%T): manually reboot for test ${i}." >>"${ldir}/status"
            /sbin/shutdown -r now
            sleep 60
        fi

        # No test is scheduled to run.
        count=$((count + 1))
	last=${i}
    done
fi

if [ "${ITERATION}" -eq 1 ]; then
    # We are done.
    break

else
    # Run the next iteration.
    sed -i "s/\(^ITERATION\)=.*/\1=$((ITERATION - 1))/" \
     "../${conf}"
fi

done

# We are done.
# Reset.
sed -i "s/\(^REBOOT\)=.*/\1=0/" "../${conf}"
crontab -r
ldir=$(ls -td "../${log}/$(hostname)."* | head -1)
echo "$(date +%F-%T): test run complete." >>"${ldir}/status"

exit 0
