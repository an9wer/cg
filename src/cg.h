#ifndef CG_H
#define CG_H

#include <stdio.h>
#include <time.h>

#define CG_FILE "/.cg"

#define REDBUFSIZ 4096
#define STRBUFSIZ 128
#define DATESIZ 86400   // 24 * 60 * 60

#define GREEN "\033[0;32m"
#define RESET "\033[0m"
#define SQUARE "⬛"

#define DEBUG
#ifdef DEBUG
#   define PERROR fprintf(stderr, "Error: %s:%d:%s\n", __FILE__, __LINE__, __func__)
#else
#   define PERROR
#endif /* DEBUG */

typedef struct {
    time_t date;
    unsigned int count;
} commit_t;

typedef struct {
    size_t size;
    commit_t *commit;
} commits_t;

enum week {
    Sun,
    Mon,
    Tue,
    Wed,
    Thu,
    Fri,
    Sat,
};

time_t truncate_time(time_t time);
void open_cg_file(FILE **stream);
void parse_from_cg_file(FILE **stream, commits_t *commits);
void generate_cg(commits_t *commits);

#endif /* CG_H */
