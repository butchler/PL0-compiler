PL-0 compile
============
Authors: Nicholas Bardy, Adam Buechler

Compiling the compiler:
-----------------------
./build.sh

run the build.sh script to compile everything
this outputs the compiler executable as:
"compiler"


Compiling PL0 code with the compiler:
-------------------------------------
./compiler in.pl0 0

The compiler accepts a first argument of a PL0 files and second argument of a verbosity level.

Verbosity
0 prints out nothing but code for the Virtual Machine
4 is the highest level

Running code:
-------------
The "run.sh" script compiles and runs a Pl0 program on the virtual machine.
Example:

./run.sh pl0
