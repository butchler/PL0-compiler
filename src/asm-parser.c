#include <assert.h>
#include <stdio.h>
#include "src/asm.h"
#include "src/lib/parser.h"
#include "src/lib/vector.h"

struct parseTree parseAssembly(struct vector *tokens) {
    // Define a grammar for the assembly language.
    struct grammar grammar = {makeVector(struct rule)};

    // An assembly program is a list of lines.
    addRule(grammar, "@assembly", "@lines");

    addRule(grammar, "@lines", "@line @lines");
    addRule(grammar, "@lines", "nothing");

    // Each line can either be an optional label followed by an instruction, or
    // an assembler directive.
    addRule(grammar, "@line", "@label @instruction");
    addRule(grammar, "@line", "@directive");

    // Any line can have a label, but labels are optional.
    addRule(grammar, "@label", "@identifier :");
    addRule(grammar, "@label", "nothing");

    // Instructions
    addRule(grammar, "@instruction", "add   @register , @register , @register");
    addRule(grammar, "@instruction", "sub   @register , @register , @register");
    addRule(grammar, "@instruction", "addi  @register , @register , @number");
    addRule(grammar, "@instruction", "and   @register , @register , @register");
    addRule(grammar, "@instruction", "or    @register , @register , @register");
    addRule(grammar, "@instruction", "lw    @register , @mem-access");
    addRule(grammar, "@instruction", "sw    @register , @mem-access");
    addRule(grammar, "@instruction", "lui   @register , @number");
    addRule(grammar, "@instruction", "beq   @register , @register , @identifier");
    addRule(grammar, "@instruction", "slt   @register , @register , @register");
    addRule(grammar, "@instruction", "slti  @register , @register , @number");
    addRule(grammar, "@instruction", "sltu  @register , @register , @register");
    addRule(grammar, "@instruction", "sltiu @register , @register , @number");
    addRule(grammar, "@instruction", "j     @identifier");

    // Rules used inside of instructions
    addRule(grammar, "@register", "$ @number");
    addRule(grammar, "@register", "$ @identifier");
    addRule(grammar, "@mem-access", "@number ( @register )");
    addRule(grammar, "@number", "number-token");
    addRule(grammar, "@identifier", "identifier-token");

    // Assembler directives
    addRule(grammar, "@directive", ".ent @identifier");
    addRule(grammar, "@directive", ".end @identifier");

    // Parse the tokens using the grammar, with @assembly as the start variable.
    return parse(tokens, grammar, "@assembly");
}

