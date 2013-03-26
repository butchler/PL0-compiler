#ifndef PARSER_H
#define PARSER_H

#include "src/lib/vector.h"

struct parseTree {
    char *name;
    struct vector *children;
};

#endif
