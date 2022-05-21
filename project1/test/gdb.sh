#!/bin/bash

killall -w server

gdb --args ../src/server 8080 8443 log.txt lock /home/zzxy/Documents/15441/project1/static /home/zzxy/Documents/15441/project1/cgi-bin priv cert
