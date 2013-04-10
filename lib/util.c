#include "lib/util.h"
#include "lib/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
// For isInteger:
#include <assert.h>
#include <errno.h>
#include <limits.h>

char *format(const char *fmt, ...) {
    int n;
    int size = 100;     /* Guess we need no more than 100 bytes */
    char *p, *np;
    va_list ap;

    if ((p = malloc(size)) == NULL)
        return NULL;

    while (1) {

        /* Try to print in the allocated space */

        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        /* Check error code */

        if (n < 0)
            return NULL;

        /* If that worked, return the string */

        if (n < size)
            return p;

        /* Else try again with more space */

        size = n + 1;       /* Precisely what is needed */

        if ((np = realloc (p, size)) == NULL) {
            free(p);
            return NULL;
        } else {
            p = np;
        }

    }
}

struct vector *splitString(char const *constString, char *splitCharacters) {
    // strsep modifies the string you give it, so in order to use splitString
    // one string literals we need to make a mutable copy of the given string.
    char *string = strdup(constString);
    char *originalString = string;

    struct vector *result = makeVector(char*);
    // While there are still unread characters in the string.
    while (string != NULL) {
        char *substring = strsep(&string, splitCharacters);

        // Don't include the empty string in the results.
        if (strlen(substring) == 0)
            continue;

        // Make a copy of the substring so that we can free originalString
        // (because strsep just returns pointers to different locations
        // inside the string that you give it, so if we free the original
        // string the substrings will just be pointing to unallocated
        // memory and cause a segfault).
        substring = strdup(substring);
        push(result, substring);
    }

    free(originalString);

    return result;
}

int isInteger(char *string) {
    assert(string != NULL);

    errno = 0;
    char *endptr = NULL;
    int value = strtol(string, &endptr, 10);

    // Check for value out of range or some other error occurred.
    if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
            || (errno != 0 && value == 0)) {
        return 0;
    }

    // Check if it didn't actually convert anything.
    if (endptr == string) {
        return 0;
    }

    // Check if there are more characters left after the number.
    if (*endptr != '\0')
        return 0;

    return 1;
}

char *joinStrings(struct vector *strings, char *separator) {
    char *join(struct vector *strings, char *separator) {
        if (strings->length == 0)
            return "";

        char *lastString = get(char*, strings, strings->length - 1);
        if (strings->length == 1)
            return lastString;

        // Remove the last string from strings.
        strings->length -= 1;
        char *firstStrings = joinStrings(strings, separator);

        char *result = format("%s%s%s", firstStrings, separator, lastString);

        free(firstStrings);
    }

    // join modifies the vector given to it, so make a copy so we don't modify
    // the vector passed to joinStrings.
    struct vector *copy = vector_copy(strings);

    return join(copy, separator);
}

char *substring(char *string, int length) {

    char *substr = (char*)malloc(sizeof(char) * (length + 1));
    strncpy(substr, string, length);
    // Add null character.
    substr[length] = '\0';

    return substr;

}

char *readContents(char *filename) {

    FILE *file = fopen(filename, "r");
    assert(file != NULL);

    // Go to the end of the file and read the position to get the length of the
    // file.
    fseek(file, 0, SEEK_END);
    int length = ftell(file);

    // Return to the beginning of the file.
    rewind(file);

    // Try to read 'length' characters.
    char *contents = malloc(sizeof(char)*(length + 1));
    int charsRead = fread(contents, sizeof(char), length, file);
    // Add null character.
    contents[charsRead] = '\0';

    // TODO: Handle the error where charsRead != length (the number of
    // characters read isn't the same as the number of characters that ftell
    // said were in the file), instead of just doing an assert.
    assert(charsRead == length);

    fclose(file);

    return contents;

}

