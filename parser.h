#ifndef PARSER_H
#define PARSER_H

#include "vector.h"

struct parseTree {
    char *name;
    struct vector *children;
    void *data;   // Used for certain leaf nodes, like identifier to store the
                  // name of the identifier, and number to store the value of
                  // the number.
};

#endif
