#ifndef PARSER_H
#define PARSER_H

#include "src/lib/vector.h"

struct parseTree {
    char *name;
    struct vector *children;
};

struct parseTree getChild(struct parseTree parent, char *childName);
struct parseTree getLastChild(struct parseTree parent, char *childName);

#endif
