
# Compile the PL/0 compiler.
SOURCES = src/*.c src/lib/*.c
compiler: $(SOURCES)
	gcc -g -o $@ -Isrc $(SOURCES) -lfl

# Compile and run a PL/0 source file.
%.pl0: compiler ALWAYS_RUN
	@#./compiler examples/$@ | ./vm -
	@# The above doesn't work because we want the VM to be able to read the
	@# user's input from stdin, and piping sets stdin to the output of the
	@# compiler. <(command) runs command and then creates a temporary file with
	@# the output of the command, but it's a bash feature, so we need to run the
	@# whole thing with bash.
	bash -c './vm <(./compiler examples/$@)'
ALWAYS_RUN:
	@# Forces %.pl0 rules to always run even if all files are up to date.

