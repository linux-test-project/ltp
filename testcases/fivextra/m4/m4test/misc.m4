divert(-1)
define(`USER', `root')
define(`TMP', maketemp(`/tmp/hejXXXXXX'))
syscmd(`grep "^'USER`:" /etc/passwd | awk -F: "{print \$3}"'  > TMP)
define(`UID', include(TMP))
syscmd(`rm -f' TMP)
divert
UID
