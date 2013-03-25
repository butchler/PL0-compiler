#ifndef PARSER_H
#define PARSER_H

#include "vector2.h"

struct parseTree;

define_vector(parse_tree_vector, struct parseTree)

struct parseTree {
    char *name;
    struct parse_tree_vector *children;
};

#endif
