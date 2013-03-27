#ifndef VECTORIZE_H
#define VECTORIZE_H

// Awesome alias for generateParseTree so that you don't have to type a bunch
// of quotes or \'s to make a multiline string.
#define pt(forms) generateParseTree("(" #forms ")")

struct vector *vectorizeForm(char *form);
struct parseTree generateParseTree(char *forms);

#include "test/lib/vectorize.c"

#endif
