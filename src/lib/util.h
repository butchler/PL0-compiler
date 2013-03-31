#ifndef UTIL_H
#define UTIL_H

// Wrapper for sprintf that allocates the string for you. Copied from the
// manpage for printf.
char *format(const char *fmt, ...);

#endif
