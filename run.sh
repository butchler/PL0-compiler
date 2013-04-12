#!/bin/bash
# Compiles and runs the given PL/0 program.

pl0File=$1
compiledCode=`mktemp`

# Compile PL/0.
./compile.sh $pl0File > $compiledCode

# Run compiled code with the VM.
./vm $compiledCode

# Remove compiled code after VM exits.
rm $compiledCode

