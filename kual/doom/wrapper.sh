#!/bin/bash
# bomberfish 2024
# File: wrapper.sh
# Dirt simple wrapper script for kdoom. Redirects all output to /mnt/us/kdoom.log while preserving cli args.

WD=$(dirname $0)

if [ ! -f $WD/kdoom ]; then
    echo "kdoom binary not found!"
    eips "kdoom binary not found!"
    exit 1
fi

$WD/kdoom $* &> /mnt/us/kdoom.log
