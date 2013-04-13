#!/bin/bash
# Rebuild and run assembler.

if [ -f asm ]; then rm asm; fi

./build.sh

if [ -f asm ]; then ./asm $*; fi

