PL/0 compiler
=============
Authors: Nicholas Bardy, Adam Buechler

* For instructions on how to use the compiler, keep on reading this file.

* For an example of the compiler's and VM's output, see example-output.txt.

* For some examples of error messages that the parser and code generator
  produce, see errors.txt.

* See mips-assembler/ for an example of another compiler that uses the same
  Context Free Grammar based parser that this compiler uses (to see what I
  mean, look at src/pl0-parser.c and mips-assembler/src/asm-parser.c; the
  parsing is done by defining a Context Free Grammar and then passing it to a
  function that uses the grammar to parse the tokens using a brute
  force/recursive descent approach). It compiles a small subset of MIPS
  assembly code into hexadecimal MIPS instructions. I implemented this as an
  extra credit portion for a project in my Computer Logic & Organization class.  


Compiling the compiler:
-----------------------
Run `make` to compile the PL/0 compiler. It will produce an executable called 'compiler'.


Compiling PL0 code with the compiler:
-------------------------------------
To compile a PL/0 program, run:

./compiler in.pl0

The compiler accepts a first argument of a PL0 files and second argument of a
verbosity level. For example:

./compiler in.pl0 2

will output the original source code and will print the generated instructions
in a more human readable format.

Verbosity levels:
* 0 is the default level and prints out nothing but code suitable for the
  Virtual Machine to run (i.e. it will print out three numbers for each
  instruction, representing the opcode, lexical level, and modifier).
* 1 prints out the instructions in a more human readable format, printing line
  numbers and printing opcode names such as 'lit' and 'sto' instead of numbers.
* 2 prints out the original PL/0 source code in addition to the previous output.
* 3 prints out the token list produced by flex in addition to the previous output.
* 4 is the highest level and prints out the full parse tree produced by the
  parser in addition to the previous output.


Running PL/0 code:
------------------
After running the compiler, you can send its output to a file and then give
that file to the virtual machine:

./compiler in.pl0 0 > out
./vm out

If you're using bash as your shell, you can run ./vm <(./compiler in.pl0) for short. Alternatively, you can use make to compile and run PL/0 files in the examples folder. For example:

make if-else.pl0

will recompile the compiler if it's out of date, and then compile and run examples/if-else.pl0.

