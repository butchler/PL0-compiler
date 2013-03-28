#include "test/lib/generator.h"
#include <stdio.h>   // For sscanf
#include <string.h>  // For strlen

int instructionsEqual(struct vector *instructions, char *expectedInstructions) {
    int i; int offset = 0;
    for (i = 0; i < instructions->length; i++) {
        struct instruction currentInstruction = get(struct instruction, instructions, i);

        char instruction[3]; int lexicalLevel; int modifier; int bytesRead;
        int numMatched = sscanf(expectedInstructions + offset, "%s %d %d%n", instruction, &lexicalLevel, &modifier, &bytesRead);
        offset += bytesRead;

        if (numMatched < 3)
            return 0;

        int opcode = getOpcode(instruction);

        if (opcode != currentInstruction.opcode ||
                lexicalLevel != currentInstruction.lexicalLevel ||
                modifier != currentInstruction.modifier)
            return 0;
    }

    if (offset == strlen(expectedInstructions))
        return 1;

    return 0;
}

