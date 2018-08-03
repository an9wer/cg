#ifndef CG_H
#define CG_H

#include <stdio.h>
#include <time.h>

#define REDBUFSIZ 4096
#define STRBUFSIZ 128
#define DATESIZ 86400   // 24 * 60 * 60

#define HIGHEST "\033[38;5;22m"
#define HIGH "\033[38;5;34m"
#define LOW "\033[38;5;82m"
#define LOWEST "\033[38;5;154m"
//#define LOWEST "\033[38;5;118m"
#define RESET "\033[0m"
#define NEWLINE "\n"
#define SQUARE "⬛"

#define DEBUG
#ifdef DEBUG
#   define CG_FILE "/.cg.test"
#   define PERROR fprintf(stderr, "Error: %s:%d:%s\n", __FILE__, __LINE__, __func__)
#else
#   define CG_FILE "/.cg"
#   define PERROR
#endif /* DEBUG */

#define MAX(a, b) ((a) > (b) ? a : b)

typedef struct {
    time_t date;
    unsigned int count;
} commit_t;

typedef struct {
    size_t size;
    commit_t *commit;
    unsigned int max_count;
} commits_t;

typedef struct {
    unsigned int col;
    char *mon;
} header_t;

char *get_home_dir(void);
time_t truncate_time(time_t time);
void open_cg_file(FILE **stream);
void parse_from_cg_file(FILE **stream, commits_t *commits);
void generate_cg(commits_t *commits);

#endif /* CG_H */
