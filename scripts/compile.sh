#!/bin/bash
# Recompile and run the compiler with the given arguments.
# First remove the existing compiler executable so that if compilation fails it
# won't execute the old compiler.

if [ -f "compiler" ]; then rm compiler; fi

./build.sh

if [ -f "compiler" ]; then ./compiler $*; fi

