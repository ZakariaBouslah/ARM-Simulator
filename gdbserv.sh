#!/bin/sh 
gdb=arm-none-eabi-gdb
./arm_simulator --gdb-port 58000 &
$gdb -ex "file Examples/$1" -x gdbserv 