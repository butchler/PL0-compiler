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

Note from Adam
--------------
There aren't any test cases that generate certain error messages, because the
parser doesn't really generate good error messages. When there's a parse error,
the parser just prints out a giant tree of errors that it encountered, but it's
not very clear from that which syntax error is the one that caused the problem.
I couldn't figure how to make the parser produce more helpful error message. In
hindsight, I probably should have just hand coded a recursive descent parser,
but it was more fun at the time to implement one that you could give a Context
Free Grammar for.
