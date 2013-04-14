#!/bin/bash
# Recompile and run the compiler with the given arguments.

if [ -f "compiler" ]; then rm compiler; fi

./build.sh

if [ -f "compiler" ]; then ./compiler $*; fi

