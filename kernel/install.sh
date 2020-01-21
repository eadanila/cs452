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

if [ -f /u/cs452/tftp/ARM/$u/kernel ]; then
    echo "Removing old kernel from tftp server"
    rm /u/cs452/tftp/ARM/$u/kernel
fi

make clean && make debug && cp kernel.elf /u/cs452/tftp/ARM/$u/kernel && chmod o=r /u/cs452/tftp/ARM/$u/kernel
