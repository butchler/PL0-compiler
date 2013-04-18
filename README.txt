Run `make` to compile the assembler.

To use the assembler, run `./asm <MIPS assembly file>`. It will output the
compiled hexadecimal instructions to to stdout. To output the instructions to a
file, run `./asm filename.asm > output-filename.asc`.

Because of the way the parser is written, the assembler will print out an error
message and stop if there is a bad instruction, rather than outputting a
garbage instruction (e.g. 0xdeadbeef) like in the given test.asc example file.
