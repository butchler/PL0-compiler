#include "test/lib/parser.h"
#include <string.h>

struct parseTree generateParseTree(char *form) {
    if (form[0] == '(') {
        struct vector *items = formToVector(form);

        char *name = get(char*, items, 0);

        struct vector *children = makeVector(struct parseTree);
        int i;
        for (i = 1; i < items->length; i++) {
            char *item = get(char*, items, i);
            struct parseTree childTree = generateParseTree(item);
            push(children, childTree);
        }

        return (struct parseTree){name, children};
    } else {
        return (struct parseTree){form, NULL};
    }
}

struct vector *formToVector(char *form) {
    char *formStart = strchr(form, '(');
    char *formEnd = strrchr(form, ')');

    char *skipWhitespace(char *start) {
        int numWhitespace = strspn(start, " \r\n\t");
        return start + numWhitespace;
    }
    char *skipNonWhitespace(char *start) {
        int numNonWhitespace = strcspn(start, " \r\n\t");
        return start + numNonWhitespace;
    }
    char *skipForm(char *start) {
        char *c = start + 1;
        int level = 1;
        while (level > 0) {
            if (*c == '(')
                level += 1;
            else if (*c == ')')
                level -= 1;
            c += 1;
        }

        return c;
    }

    struct vector *items = makeVector(char*);
    char *c = formStart + 1;
    while (c < formEnd) {

        char *start = skipWhitespace(c);

        char *end = NULL;
        if (*start == '(')
            end = skipForm(start);
        else
            end = skipNonWhitespace(start);
        if (end > formEnd)
            end = formEnd;

        int length = end - start;
        char *item = strndup(start, length);
        push(items, item);

        c = end;
    }

    return items;
}

