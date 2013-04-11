void testFormToVector() {
    struct vector *items = formToVector("(aaa (bbb ccc) ddd)");

    assert(strcmp(get(char*, items, 0), "aaa") == 0);
    assert(strcmp(get(char*, items, 1), "(bbb ccc)") == 0);
    assert(strcmp(get(char*, items, 2), "ddd") == 0);

    freeVector(items);
}

void testGenerateParseTree() {
    struct parseTree tree = pt(aaa (bbb (identifier ccc)) (number 123));
    assert(strcmp(tree.name, "aaa") == 0);

    struct parseTree child = get(struct parseTree, tree.children, 0);
    assert(strcmp(child.name, "bbb") == 0);

    struct parseTree grandchild = get(struct parseTree, child.children, 0);
    assert(strcmp(grandchild.name, "identifier") == 0);

    char *identifier = get(struct parseTree, grandchild.children, 0).name;
    assert(strcmp(identifier, "ccc") == 0);

    struct parseTree child2 = get(struct parseTree, tree.children, 1);
    assert(strcmp(child2.name, "number") == 0);

    // TODO: Free parse trees.
}

testFormToVector();
testGenerateParseTree();

struct parseTree readAndParse(char *source, struct grammar grammar, char *startVariable) {
    struct vector *tokens = readPL0Tokens(source);
    assert(tokens != NULL);

    struct parseTree tree = parse(tokens, grammar, startVariable);
    if (isParseTreeError(tree) && getParserErrors() != NULL)
        puts(getParserErrors());
    assert(!isParseTreeError(tree));

    return tree;
}

// Test a very simple grammar with only one real rule.
// ===================================================
struct grammar grammar = {makeVector(struct rule)};
addRule(grammar, "@integer", "int @identifier ;");
addRule(grammar, "@identifier", "identifier-token");

struct parseTree tree = readAndParse("int x;", grammar, "@integer");
assert(parseTreesEqual(tree, pt(@integer int (@identifier x) ;)));
//assert(parseTreesSimilar(tree, pt(integer (identifier x))));

// Test a very simple subset of the PL/0 grammar.
// ==============================================
grammar = (struct grammar){makeVector(struct rule)};
addRule(grammar, "begin-block", "begin statements end");
// Just like in the lexer, the regexes for the longer strings must come
// before the shorter ones, with the grammar longer rules must come before
// shorter ones so that each variable is "greedy" and matches as much as it
// possibly can. So, rule that matches multiple statements must come before
// the rule that only matches one.
addRule(grammar, "statements", "statement ; statements");
addRule(grammar, "statements", "nothing");
addRule(grammar, "statement", "read-statement");
addRule(grammar, "statement", "write-statement");
addRule(grammar, "read-statement", "read identifier");
addRule(grammar, "write-statement", "write identifier");
addRule(grammar, "identifier", "identifier-token");

char *source = "begin\n"
               "   read x;\n"
               "   write x;\n"
               "end\n";
tree = readAndParse(source, grammar, "begin-block");

assert(parseTreesSimilar(tree,
  pt(begin-block
      (statements
          (statement (read-statement
                        (identifier x)))
          (statements
              (statement (write-statement
                         (identifier x))))))));

// Test the full PL/0 grammar.
// ===========================
source = "const a = 5, b = 10;\n"
         "int x, y, z;\n"
         "begin\n"
         "   read x;\n"
         "   read y;\n"
         "   x := x + y*a;\n"
         "   y:=(y+x)*b;\n"
         "   z := x * y;\n"
         "   write z\n"
         "end.\n";

tree = parsePL0Tokens(readPL0Tokens(source));
//printParseTree(tree);
if (isParseTreeError(tree))
    printf("Parser had errors:\n%s\n", getParserErrors());
// TODO: Write giant parse tree to test this program.
