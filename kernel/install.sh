#!/bin/bash 

if [[ $# -eq 0 ]]; then
    u="eadanila"
else 
    u=$1
fi

if [ ! -d /u/cs452/tftp/ARM/$u/ ]; then
    echo "/u/cs452/tftp/ARM/$u/ does not exist"
    exit 1
fi

export DESTDIR=/u/cs452/tftp/ARM/$u

make clean
make debug
make install

