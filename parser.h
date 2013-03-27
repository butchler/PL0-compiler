#ifndef PARSER_H
#define PARSER_H

#include "vector.h"

struct parseTree {
    char *name;
    struct vector *children;
    struct vector *data;
};

#endif
