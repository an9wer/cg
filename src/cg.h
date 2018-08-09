#ifndef CG_H
#define CG_H

#include <stdio.h>
#include <time.h>

#define STRBUFSIZ 128
#define DAYSIZ 86400   // 24 * 60 * 60

#define HIGHEST "\x1b[38;5;22m"
#define HIGH "\x1b[38;5;34m"
#define LOW "\x1b[38;5;82m"
#define LOWEST "\x1b[38;5;154m"
#define SELECT "\x1b[48;5;21m"
#define RESET "\x1b[0m"
#define NEWLINE "\n"
#define SQUARE "\u2B1B" // ⬛ thx: https://www.fileformat.info/info/unicode/char/2b1b/index.htm

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

/* utils */
char *get_home_dir(void);
time_t truncate_time(time_t time);
int header_sort(const void *a, const void *b);

/* cg */
void cg_filename(char *cg_file, size_t max);
void cg_open(void);
void cg_close(void);
void cg_parse(void);
void cg_exit(int status);
void cg_generate(void);

void draw_header(void);
void draw_unit(time_t day, int mode);
void draw_body(void);
void draw_foot_and_select(void);

/* tty */
void tty_open(void);
void tty_close(void);
void tty_printf(const char *fmt, ...);
void tty_reset(void);
void tty_winsize(void);
void tty_init(void);
void tty_clear_all(void);
void tty_clear_entire_line(void);
void tty_newline(void);
void tty_run(void);

/* action */
void action_move_up(void);
void action_move_down(void);
void action_move_left(void);
void action_move_right(void);

/* cursor */
void cursor_init(void);
void cursor_hidden(void);
void cursor_visible(void);
void cursor_move_up(int step);
void cursor_move_down(int step);
void cursor_move_left(int step);
void cursor_move_right(int step);
void cursor_move_from_foot(void);
void cursor_move_to_foot(void);

void unit_move_up(int step);
void unit_move_down(int step);
void unit_move_left(int step);
void unit_move_right(int step);

#endif /* CG_H */
