#!/bin/sh

groff -t -e -mandoc -Tascii epoll.4 | col -bx > epoll.txt
groff -t -e -mandoc -Tascii epoll_create.2 | col -bx > epoll_create.txt
groff -t -e -mandoc -Tascii epoll_ctl.2 | col -bx > epoll_ctl.txt
groff -t -e -mandoc -Tascii epoll_wait.2 | col -bx > epoll_wait.txt
groff -t -e -mandoc -Tps epoll.4 > epoll.ps
groff -t -e -mandoc -Tps epoll_create.2 > epoll_create.ps
groff -t -e -mandoc -Tps epoll_ctl.2 > epoll_ctl.ps
groff -t -e -mandoc -Tps epoll_wait.2 > epoll_wait.ps

