#!/bin/bash

vfile=$1
extension="${vfile##*.}"
filename="${vfile%.*}"

if [ "${extension}" != "vsl" ]; then
    echo "file must be vsl, got .${extension}"
    exit 1
fi

function runc {
    "$@"
    local status=$?
    if [ ${status} -ne 0 ]; then
        echo "error with $1" >&2
        exit
    fi
}

runc ../bin/vslc.out < ${vfile} > ${filename}.s
runc gcc -o ${filename}.out ${filename}.s
