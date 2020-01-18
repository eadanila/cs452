#!/bin/bash 

if [[ $# -eq 0 ]]; then
    u="eadanila"
else 
    u=$1
fi

make clean && make && cp kernel.elf /u/cs452/tftp/ARM/$u/kernel && chmod o=r /u/cs452/tftp/ARM/$u/kernel
