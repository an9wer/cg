#include <pwd.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "cg.h"

static char *week[7] = {
    "Sat", "Mon", "Tue",
    "Wed", "Thu", "Fri", "Sun",
};
static char *month[12] = {
    "Jan", "Feb", "Mar", "Apr",
    "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec",
};
static commits_t commits = {0, NULL, 1};
static struct termios original_termios;
static time_t body[7][53];
static cursor_t cursor;
static time_t today;
static int tty;

#ifdef DEBUG
FILE *cg;
#else
static FILE *cg;
#endif

#ifndef DEBUG
int main(void)
{
}
#endif

void cg_exit(int status)
{
    if (status != EXIT_SUCCESS) PERROR;
    if (commits.commit != NULL) free(commits.commit);
    tty_reset();
    exit(status);
}

/* tty  {{{ */
void tty_reset(void)
{
    cursor_visible();
    tcsetattr(tty, TCSANOW, &original_termios);
}

void tty_winsize(void)
{
    struct winsize ws;
    if (ioctl(tty, TIOCGWINSZ, &ws) == -1) cg_exit(EXIT_FAILURE);
    if (ws.ws_row < 12 || ws.ws_col < 112) cg_exit(EXIT_FAILURE);
}

void tty_init(void)
{
    struct termios termios;

    if (tcgetattr(tty, &termios) < 0)
        cg_exit(EXIT_FAILURE);

    original_termios = termios;

    termios.c_lflag &= ~(ECHO | ICANON);
    termios.c_cc[VMIN] = 1;
    termios.c_cc[VTIME] = 0;

    if (tcsetattr(tty, TCSANOW, &termios) < 0)
        cg_exit(EXIT_FAILURE);
    
    if ((termios.c_lflag & (ECHO | ICANON)) || termios.c_cc[VMIN] != 1 || termios.c_cc[VTIME] != 0)
        cg_exit(EXIT_FAILURE);

    cursor_hidden();
}

void tty_clear_entire_line(void)
{
    dprintf(tty, "\033[3K");
}

void tty_newline(void)
{
    dprintf(tty, NEWLINE "\033[K");
}
/* }}} tty */


/* corsor {{{ */
void cursor_init(void)
{
    cursor.x = 52;
    cursor.y = localtime(&today)->tm_wday;
}

/*
 * Q: hide cursor in terminal
 * thx: https://www.experts-exchange.com/questions/21115816/How-to-hide-cursor-in-a-Linux-Unix-console.html
 */
void cursor_hidden(void)
{
    dprintf(tty, "\033[?25l");
}

void cursor_visible(void)
{
    dprintf(tty, "\033[?25h");
}

void cursor_move_up(int step)
{
    dprintf(tty, "\033[%iA", step);
}

void cursor_move_down(int step)
{
    dprintf(tty, "\033[%iB", step);
}

void cursor_move_left(int step)
{
    dprintf(tty, "\033[%iD", step);
}

void cursor_move_right(int step)
{
    dprintf(tty, "\033[%iC", step);
}

void cursor_move_from_last(void)
{
    cursor_move_up(7 - cursor.y);
    cursor_move_right(4 + cursor.x * 2);
}

void cursor_move_to_last(void)
{
    cursor_move_down(7 - cursor.y);
    cursor_move_left(4 + cursor.x * 2);
}

void unit_move_up(int step)
{
    cursor.y -= step;
    if (cursor.y < 0) cg_exit(EXIT_FAILURE);
    cursor_move_up(step);
}

void unit_move_down(int step)
{
    cursor.y += step;
    if (cursor.y > 6) cg_exit(EXIT_FAILURE);
    cursor_move_down(step);
}

void unit_move_left(int step)
{
    cursor.x -= step;
    if (cursor.x < 0) cg_exit(EXIT_FAILURE);
    cursor_move_left(2 * step);
}

void unit_move_right(int step)
{
    cursor.x += step;
    if (cursor.x > 52) cg_exit(EXIT_FAILURE);
    cursor_move_right(2 * step);
}
/* }}} cursor */

char *get_home_dir(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) cg_exit(EXIT_FAILURE);
    return pw->pw_dir;
}

time_t truncate_time(time_t time)
{
    return time / DAYSIZ * DAYSIZ;
}

void get_cg_file(char *cg_file, size_t max)
{
    char *home= get_home_dir();

    if (strlen(home) + strlen(CG_FILE) + 1 > max) cg_exit(EXIT_FAILURE);

    cg_file = strcpy(cg_file, home);
    cg_file = strcat(cg_file, CG_FILE);
}

void open_cg_file()
{
    struct stat stat_buf;
    char cg_file[STRBUFSIZ];

    get_cg_file(cg_file, STRBUFSIZ);

    // create cg file if it doesn't exist.
    if (stat(cg_file, &stat_buf) == 0) {
        if (!S_ISREG(stat_buf.st_mode))
            cg_exit(EXIT_FAILURE);
    } else {
        cg = fopen(cg_file, "w");
        fclose(cg);
    }

    cg = fopen(cg_file, "r+");
    rewind(cg);
}

void parse_from_cg_file()
{
    char buffer[STRBUFSIZ];
    time_t day;
    unsigned short flag;
    do {
        if (fgets(buffer, STRBUFSIZ, cg)) {
            day = truncate_time(atol(buffer));

            flag = 0;
            for (int i = 0; i < commits.size; i++) {
                if (day == commits.commit[i].day) {
                    commits.commit[i].count += 1;
                    commits.max_count = MAX(commits.max_count, commits.commit[i].count);
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) {
                commits.size += 1;
                commits.commit = realloc(commits.commit, sizeof(commit_t) * commits.size);
                commits.commit[commits.size-1].day = day;
                commits.commit[commits.size-1].count = 1;
            }
        } else if (ferror(cg)) 
            cg_exit(EXIT_FAILURE);
    } while (!feof(cg));
}

int header_sort(const void *a, const void *b)
{
    if (((header_t *)a)->col > ((header_t *)b)->col) return 1;
    else return -1;
}

void draw_header()
{
    header_t header[12];
    time_t latest_week_day;
    time_t day;
    struct tm *tmp;
    for (int i = 0; i < 7; i++) {
        latest_week_day = today - localtime(&today)->tm_wday * DAYSIZ + i * DAYSIZ;
        for (int j = 52; j >= 0; j--) {
            day = latest_week_day - j * 7 * DAYSIZ;
            tmp = localtime(&day);
            if (tmp->tm_mday == 1) {
                header[tmp->tm_mon].col = (52 - j) * 2 + 3;
                header[tmp->tm_mon].mon = month[tmp->tm_mon];
            }
        }
    }

    qsort(header, 12, sizeof(header_t), header_sort);

    char output_partial[STRBUFSIZ];
    sprintf(output_partial, "%*s", header[0].col, header[0].mon);

    char *output = malloc(strlen(output_partial) + 1);
    output[0] = '\0';
    output = strcat(output, output_partial);

    for (int i = 1 ; i < 12; i++) {
        sprintf(output_partial, "%*s", header[i].col - header[i-1].col, header[i].mon);

        output = realloc(output, strlen(output) + strlen(output_partial) + 1);
        output = strcat(output, output_partial);
    }
    //output = realloc(output, strlen(output) + strlen(NEWLINE) + 1);
    //output = strcat(output, NEWLINE);

    dprintf(tty, "    %s", output);
    tty_newline();

    free(output);
}

void _draw_unit(time_t day, bool select, char **unit)
{
    unsigned int higher = commits.max_count;
    unsigned int high = commits.max_count / 4 * 3;
    unsigned int low = commits.max_count / 4 * 2;
    unsigned int lower = commits.max_count / 4;

    *unit = select ? SELECT SQUARE RESET : SQUARE;  // default value

    for (int i = 0; i < commits.size; i++) {
        if (commits.commit[i].day == day) {
            if (commits.commit[i].count > high && commits.commit[i].count <= higher)
                *unit = select ? SELECT HIGHEST SQUARE RESET : HIGHEST SQUARE RESET;
            else if (commits.commit[i].count > low && commits.commit[i].count <= high)
                *unit = select ? SELECT HIGH SQUARE RESET : HIGH SQUARE RESET;
            else if (commits.commit[i].count > lower && commits.commit[i].count <= low)
                *unit = select ? SELECT LOW SQUARE RESET : LOW SQUARE RESET;
            else if (commits.commit[i].count > 0 && commits.commit[i].count <= lower)
                *unit = select ? SELECT LOWEST SQUARE RESET : LOWEST SQUARE RESET;
            else
                cg_exit(EXIT_FAILURE);
            break;
        }
    }
}

void draw_unit(time_t day, int mode)
{
    char *unit = NULL;

    if (day > today)
        unit = "  ";
    else {
        switch (mode) {
            case D_INIT :
            case D_RECOVER :
                _draw_unit(day, false, &unit);
                break;
            case D_SELECT :
                _draw_unit(day, true, &unit);
                break;
        }
    }

    dprintf(tty, "%s", unit);

    if (mode == D_INIT && day >= today - localtime(&today)->tm_wday * DAYSIZ)
        tty_newline();
    if (mode == D_SELECT || mode == D_RECOVER)
        cursor_move_left(2);
}

void draw_body()
{
    time_t latest_week_day;
    time_t day;

    for (int i = 0; i < 7; i++) {
        dprintf(tty, "%-4s", week[i]);
        latest_week_day = today - localtime(&today)->tm_wday * DAYSIZ + i * DAYSIZ;
        for (int j = 52; j >= 0; j--) {
            day = latest_week_day - j * 7 * DAYSIZ;

            body[i][52-j] = day;
            draw_unit(day, D_INIT);
        }
    }
    cursor_move_from_last();
}

void draw_foot_and_select()
{
    cursor_move_to_last();
    tty_clear_entire_line();

    char date[STRBUFSIZ];
    time_t day = body[cursor.y][cursor.x];
    strftime(date, STRBUFSIZ, "%Y/%m/%d", localtime(&day));

    unsigned int count = 0;
    for (int i = 0; i < commits.size; i++) {
        if (day == commits.commit[i].day) {
            count = commits.commit[i].count;
            break;
        }
    }
    char date_count_bar[STRBUFSIZ];
    sprintf(date_count_bar, "%s: %d", date, count);

    char level_bar[STRBUFSIZ];
    sprintf(level_bar, "less %s%s%s%s%s more", SQUARE, LOWEST SQUARE RESET,
            LOW SQUARE RESET, HIGH SQUARE RESET, HIGHEST SQUARE RESET);

    dprintf(tty, "%s%*s%s", date_count_bar, 110 - (int) strlen(date_count_bar) - 20, "", level_bar);

    cursor_move_left(10000);    // hack
    cursor_move_from_last();

    draw_unit(body[cursor.y][cursor.x], D_SELECT);
}

void generate_cg(void)
{
    cursor_init();
    draw_header();
    draw_body();
    draw_foot_and_select();
}

void move_up(void)
{
    tty_winsize();

    time_t day = body[cursor.y][cursor.x];
    draw_unit(day, D_RECOVER);

    if (cursor.y == 0 && cursor.x == 0) {
        unit_move_right(52);
        for (int i = 0; i < 7; i++) {
            unit_move_down(1);
            if (body[cursor.y][cursor.x] == today)
                break;
        }
    } else if (cursor.y == 0) {
        unit_move_left(1);
        unit_move_down(6);
    } else {
        unit_move_up(1);
    }
    draw_foot_and_select();
}

void move_down(void)
{
    tty_winsize();

    time_t day = body[cursor.y][cursor.x];
    draw_unit(day, D_RECOVER);
    
    if (day + DAYSIZ > today) {
        unit_move_left(cursor.x);
        unit_move_up(cursor.y);
    } else if (cursor.y == 6) {
        unit_move_right(1);
        unit_move_up(6);
    } else {
        unit_move_down(1);
    }
    draw_foot_and_select();
}

void move_left(void)
{
    tty_winsize();

    time_t day = body[cursor.y][cursor.x];
    draw_unit(day, D_RECOVER);

    if (cursor.x == 0) {
        if (cursor.y == 0) unit_move_down(6);
        else unit_move_up(1);

        unit_move_right(52);
        if (body[cursor.y][cursor.x] > today)
            unit_move_left(1);
    } else
        unit_move_left(1);

    draw_foot_and_select();
}

void move_right(void)
{
    tty_winsize();

    time_t day = body[cursor.y][cursor.x];
    draw_unit(day, D_RECOVER);

    if (cursor.x == 51 && (body[cursor.y][cursor.x] + DAYSIZ > today) || cursor.x == 52) {
        if (cursor.y == 6) unit_move_up(6);
        else unit_move_down(1);
        unit_move_left(cursor.x);
    }
    else
        unit_move_right(1);

    draw_foot_and_select();
}

void handle(char input)
{
    switch (input) {
        case 'q' :
            cg_exit(EXIT_SUCCESS);
            break;
        case 'h' :
            move_left();
            break;
        case 'j' :
            move_down();
            break;
        case 'k' :
            move_up();
            break;
        case 'l' :
            move_right();
            break;
    }
}

void tty_run(void)
{
    tty = open("/dev/tty", O_RDWR);
    today = truncate_time(time(NULL));

    tty_winsize();
    tty_init();

    open_cg_file();
    parse_from_cg_file();

    generate_cg();

    /* handle input */
    char input[1];
    while (read(tty, input, 1)) {
        handle(input[0]);
    }
}

