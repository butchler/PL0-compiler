#ifndef UTIL_H
#define UTIL_H

// Wrapper for sprintf that allocates the string for you. Copied from the
// manpage for printf.
char *format(const char *fmt, ...);

// Wrapper for strsep that returns the split string as a vector of strings.
// Splits along any sequence of the given split characters.
struct vector *splitString(char const *constString, char *splitCharacters);

// Given a vector of strings, returns a string of all of the strings separted
// by the given separator. For example, if the vector has the strings "abc",
// "def", "ghi", and the separator is ", ", the resulting string would be
// "abc, def, ghi".
char *joinStrings(struct vector *strings, char *separator);

// Returns a new string that is a copy of the given string up to the given
// length.
char *substring(char *string, int length);

// Returns true if the given string represents an integer value.
// Based on the example in the manpage for strtol.
int isInteger(char *string);

// Takes a filename and opens the given files, reads the entire contents into a
// string, closes the file, and returns the string.
char *readContents(char *filename);

#endif
