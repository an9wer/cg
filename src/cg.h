#ifndef CG_H
#define CG_H

#include <stdio.h>
#include <time.h>

#define REDBUFSIZ 4096
#define STRBUFSIZ 128
#define DAYSIZ 86400   // 24 * 60 * 60

#define HIGHEST "\033[38;5;22m"
#define HIGH "\033[38;5;34m"
#define LOW "\033[38;5;82m"
#define LOWEST "\033[38;5;154m"
#define SELECT "\033[48;5;21m"
#define RESET "\033[0m"
#define NEWLINE "\n"
#define SQUARE "\u2B1B" // ⬛ thx: https://www.fileformat.info/info/unicode/char/2b1b/index.htm

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
    time_t day;
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

typedef struct {
    unsigned short x;
    unsigned short y;
} cursor_t;

enum {
    D_INIT,
    D_SELECT,
    D_RECOVER,
};

void cg_exit(int status);
void cursor_hidden(void);
void cursor_visible(void);
void tty_reset(void);
char *get_home_dir(void);
time_t truncate_time(time_t time);
void open_cg_file(void);
void parse_from_cg_file(void);
void generate_cg(void);
void tty_run(void);

#endif /* CG_H */
