# definitions for the check scripts

#--------------------------------
#------ constants ---------------
#--------------------------------

# Note that usrpass1 is also used in Makefile.am
usrname1="one"
usrpass1="def"
usrname2="two"
usrpass2=""
usrname3="three"
usrpass3="abc"
srcdir=.

dsa_usrname1="pgp5"
# we use the sub key because we do not yet have the logic to
# to derive the first encryption key from a keyblock (I guess)
dsa_usrname2="0xCB879DE9"

dsa_keyrings="--keyring ./pubring.pkr --secret-keyring ./secring.skr"


plain_files="plain-1 plain-2 plain-3"
data_files="data-500 data-9000 data-32000 data-80000"
exp_files=""

# The testscripts expect the original language
LANG=
LANGUAGE=
LC_ALL=
LC_MESSAGES=

#--------------------------------
#------ utility functions -------
#--------------------------------

fatal () {
    echo "$pgmname: fatal:" $* >&2
    exit 1;
}

error () {
    echo "$pgmname:" $* >&2
    exit 1
}

info () {
    echo "$pgmname:" $* >&2
}


echo_n_init=no
echo_n () {
  if test "$echo_n_init" = "no"; then
    if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
      if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
	echo_n_n=
	echo_n_c='
'
      else
	echo_n_n='-n'
	echo_n_c=
      fi
    else
      echo_n_n=
      echo_n_c='\c'
    fi
    echo_n_init=yes
  fi
  echo $echo_n_n "${1}$echo_n_c"
}


#cleanup () {
#    rm $cleanup_files 2>/dev/null || true
#    echo "#empty" >./options
#}


#add_cleanup () {
#    cleanup_files="$cleanup_files $*"
#}


have_hash_algo () {
  if  gpg --homedir .  --version | grep "Hash:.*$1" >/dev/null
  then
	true
  else
	false
  fi
}

set -e
pgmname=`basename $0`
#trap cleanup SIGHUP SIGINT SIGQUIT

[ -z "$srcdir" ] && fatal "srcdir variable not set."

:
# end
