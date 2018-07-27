#ifndef CG_H
#define CG_H

#include <stdio.h>

#define CG_FILE "/.cg"

#define REDBUFSIZ 4096
#define STRBUFSIZ 128

#define SQUARE "⬛"

#define DEBUG
#ifdef DEBUG
#   define PERROR fprintf(stderr, "Error: %s:%d:%s\n", __FILE__, __LINE__, __func__)
#else
#   define PERROR
#endif /* DEBUG */

void open_cg_file(FILE **stream);

#endif /* CG_H */
