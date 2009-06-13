/*
 * Copyright 2008 Jason McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the LGPL v2
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

/*
 * Gets the next character from *src, and modifies
 * *src to point to the next-next character.
 *
 * Handles the Unix-style '\' escape character.
 *
 * Returns the next character, or -1 if an error
 * (ie '\' at the end of a line)
 */
static int next_char(const char **src)
{
	int c;

	if (src == NULL || *src == NULL || (*src)[0] == 0) {
		return -EINVAL;
	}

	if ((*src)[0] == '\\') {
		if ((*src)[1] == 0) {
			return -EINVAL;
		}
		(*src)++;
	}

	c = (*src)[0];
	(*src)++;
	return c;
}

/* Packs quoted string into *pdst
 */
static int pack_quote(char **pdst, const char **psrc, char token)
{
	int err, c;
	char *dst = *pdst;
	const char *src = *psrc;

	while (*src != 0 && *src != token) {
		if (*src == '"' || *src == '\'') {
			c = *(src++);
			err = pack_quote(&dst, &src, c);
			if (err < 0) {
				return err;
			}

			continue;
		}

		c = next_char(&src);
		if (c < 0) {
			return c;
		}

		*(dst++) = c;
	}

	if (*src != token) {
		return -EINVAL;
	}

	*dst = 0;
	/* *src == token - skip */
	src++;
	*psrc = src;
	*pdst = dst;

	return 0;
}


int tokenizer_next(char **dst, const char **src)
{
	char *dp;
	const char *s = *src;
	int c, err;

	/* Skip leading white space if token == 0 */
	while (*s && isspace(*s)) {
		s++;
	}

	/* If nothing to do, indicate doneness.
	 */
	if (*s == 0 ) {
		*dst = NULL;
		return 0;
	}

	*dst = strdup(*src);
	dp = *dst;

	while (*s != 0 && !isspace(*s)) {
		/* Handle quoting
		 */
		if (*s == '"' || *s == '\'') {
			c = *(s++);
			err = pack_quote(&dp, &s, c);
			if (err < 0) {
				return -EINVAL;
			}
			continue;
		}

		c = next_char(&s);
		if (c < 0) {
			return c;
		}

		*(dp++) = c;
	}

	if (isspace(*s)) {
		s++;
	}
	*src = s;
	*dp = 0;

	return 0;
}

int argv_from(const char *src, int *pargc, char ***pargv)
{
	int argc=0;
	char **argv;
	char *cp;
	int err;

	argv = malloc(sizeof(char *));
	argv[0] = NULL;
	while (*src != 0) {
		err = tokenizer_next(&cp, &src);
		if (err < 0) {
			return err;
		}

		if (cp == NULL) {
			break;
		}

		argv[argc++] = strdup(cp);
		argv = realloc(argv, sizeof(char *) * (argc + 1));
		argv[argc] = NULL;
	}

	*pargc = argc;
	*pargv = argv;

	return 0;
}

void argv_free(int argc, char **argv)
{
	int i;
	for (i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
}

