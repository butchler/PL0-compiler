#ifndef PARSER_H
#define PARSER_H

#include "vector.h"

struct parseTree;

declareVector(parseTreeVector, struct parseTree)

struct parseTree {
    char *name;
    struct parseTreeVector *children;
    char *data;
};

#endif
