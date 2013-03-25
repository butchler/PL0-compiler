#ifndef PARSER_H
#define PARSER_H

#include "vector-types.h"

struct parseTree {
    char *name;
    struct parseTreeVector *children;
    struct stringVector *data;
};

#endif
