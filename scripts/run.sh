#!/bin/bash
# Compiles and runs the given PL/0 program.

pl0File=$1
compiledCode=`mktemp --tmpdir=.`

# Compile PL/0.
./compile.sh $pl0File > $compiledCode
# Print results of compilation in case there were bugs.
echo "Compiler output:"
cat $compiledCode
echo

# Run compiled code with the VM.
echo "VM output:"
./vm $compiledCode

# Remove compiled code after VM exits.
rm $compiledCode

