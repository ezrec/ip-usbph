/*
 * Copyright 2008, Jason McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the LGPL v2
 */

#ifndef ARGV_H
#define ARGV_H

/*
 * Get a Unix-quoting rules (", ', and \) next-whitepace-delimited
 * argc/argv array from *src.
 *
 * Returns 0 on success, -1 if quoting rules are broken.
 */
int argv_from(const char *src, int *pargc, char ***pargv);

void argv_free(int argc, char **argv);

#endif /* ARGV_H */
