#!/bin/sh
echo "running func_tests/test_timetolive_v6"
func_tests/test_timetolive_v6
sleep 2;
echo "running func_tests/test_timetolive"
func_tests/test_timetolive
sleep 2;
echo "running func_test/test_tcp_style_v6"
func_tests/test_tcp_style_v6
sleep 2;
echo "func_tests/test_tcp_style"
func_tests/test_tcp_style 
sleep 2;
echo "running func_tests/test_sockopt_v6"
func_tests/test_sockopt_v6
sleep 2;
echo "running func_tests/test_sockopt"
func_tests/test_sockopt
sleep 2;
#echo "running func_tests/test_sendmsg_v6"
#func_tests/test_sendmsg_v6
#sleep 2;
#echo "running func_tests/test_sctp_recvmsg_v6"
#func_tests/test_sctp_recvmsg_v6
#sleep 2;
#echo "running func_tests/test_sctp_recvmsg"
#func_tests/test_sctp_recvmsg
#sleep 2;
echo "running func_tests/test_recvmsg"
func_tests/test_recvmsg
sleep 2;
echo "running func_tests/test_peeloff_v6"
func_tests/test_peeloff_v6
sleep 2;
echo "running func_tests/test_peeloff"
func_tests/test_peeloff
sleep 2;
echo "running func_tests/test_inaddr_any_v6"
func_tests/test_inaddr_any_v6
sleep 2;
echo "running func_tests/test_inaddr_any"
func_tests/test_inaddr_any
sleep 2;
echo "running func_tests/test_getname_v6"
func_tests/test_getname_v6
sleep 2;
echo "running func_tests/test_getname"
func_tests/test_getname
sleep 2;
echo "running func_tests/test_fragments_v6"
func_tests/test_fragments_v6
sleep 2;
echo "running func_tests/test_fragments"
func_tests/test_fragments
sleep 2;
echo "running func_tests/test_connect"
func_tests/test_connect
sleep 2;
echo "running func_tests/test_basic_v6"
func_tests/test_basic_v6
sleep 2;
echo "running func_tests/test_basic"
func_tests/test_basic
sleep 2;
echo "running func_tests/test_autoclose"
func_tests/test_autoclose
sleep 2;
echo "running func_tests/test_assoc_shutdown"
func_tests/test_assoc_shutdown
sleep 2;
echo "running func_tests/test_assoc_abort"
func_tests/test_assoc_abort
sleep 2;
echo "running func_tests/test_sctp_sendrecvmsg"
func_tests/test_sctp_sendrecvmsg
sleep 2;
echo "running func_tests/test_sctp_sendrecvmsg_v6"
func_tests/test_sctp_sendrecvmsg_v6
