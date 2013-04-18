
# Compile the assembler.
SOURCES = src/*.c src/lib/*.c
asm: $(SOURCES)
	gcc -g -o $@ -Isrc $(SOURCES) -lfl

# Run make test to run tthe assembler on a test file.
test: asm
	./asm test.asm

