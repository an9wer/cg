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

#ifndef DEBUG
int main(void)
{
}
#endif

void tty_reset()
{
    tcsetattr(tty, TCSANOW, &original_termios);
}

void tty_init()
{
    struct termios termios;

    if (tcgetattr(tty, &termios) < 0) {
        PERROR;
        exit(EXIT_FAILURE);
    }

    original_termios = termios;

    termios.c_lflag &= ~(ECHO | ICANON);
    termios.c_cc[VMIN] = 1;
    termios.c_cc[VTIME] = 0;

    if (tcsetattr(tty, TCSANOW, &termios) < 0) {
        PERROR;
        exit(EXIT_FAILURE);
    }
    
    if ((termios.c_lflag & (ECHO | ICANON)) || termios.c_cc[VMIN] != 1 || termios.c_cc[VTIME] != 0) {
        tty_reset();
        PERROR;
        exit(EXIT_FAILURE);
    }
}

void move_back_unit(int count)
{
    dprintf(tty, "\033[%iD", count * 2);
}


char *get_home_dir(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        PERROR;
        exit(EXIT_FAILURE);
    }
    return pw->pw_dir;
}

time_t truncate_time(time_t time)
{
    return time / DAYSIZ * DAYSIZ;
}

void get_cg_file(char *cg_file, size_t max)
{
    char *home= get_home_dir();

    if (strlen(home) + strlen(CG_FILE) + 1 > max) {
        PERROR;
        exit(EXIT_FAILURE);
    }
    cg_file = strcpy(cg_file, home);
    cg_file = strcat(cg_file, CG_FILE);
}

void open_cg_file(FILE **stream)
{
    struct stat stat_buf;
    char cg_file[STRBUFSIZ];

    get_cg_file(cg_file, STRBUFSIZ);

    // create cg file if it doesn't exist.
    if (stat(cg_file, &stat_buf) == 0) {
        if (!S_ISREG(stat_buf.st_mode)) {
            PERROR;
            exit(EXIT_FAILURE);
        }
    } else {
        *stream = fopen(cg_file, "w");
        fclose(*stream);
    }

    *stream = fopen(cg_file, "r+");
    rewind(*stream);
}

void parse_from_cg_file(FILE **stream)
{
    char buffer[STRBUFSIZ];
    time_t date;
    unsigned short flag;
    do {
        if (fgets(buffer, STRBUFSIZ, *stream)) {
            date = truncate_time(atol(buffer));

            flag = 0;
            for (int i = 0; i < commits.size; i++) {
                if (date == commits.commit[i].date) {
                    commits.commit[i].count += 1;
                    commits.max_count = MAX(commits.max_count, commits.commit[i].count);
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) {
                commits.size += 1;
                commits.commit = realloc(commits.commit, sizeof(commit_t) * commits.size);
                commits.commit[commits.size-1].date = date;
                commits.commit[commits.size-1].count = 1;
            }
        } else if (ferror(*stream)) {
            PERROR;
            exit(EXIT_FAILURE);
        }
    } while (!feof(*stream));
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
    output = realloc(output, strlen(output) + strlen(NEWLINE) + 1);
    output = strcat(output, NEWLINE);

    dprintf(tty, "    %s", output);

    free(output);
}

void draw_foot()
{
    dprintf(tty, "%*sless %s%s%s%s more\n", 92, "", LOWEST SQUARE RESET, LOW SQUARE RESET, HIGH SQUARE RESET, HIGHEST SQUARE RESET);
}

void _draw_unit(time_t day, bool selected, char **unit)
{
    unsigned int higher = commits.max_count;
    unsigned int high = commits.max_count / 4 * 3;
    unsigned int low = commits.max_count / 4 * 2;
    unsigned int lower = commits.max_count / 4;

    *unit = selected ? SELECT SQUARE RESET : SQUARE;  // default value

    for (int i = 0; i < commits.size; i++) {
        if (commits.commit[i].date == day) {
            if (commits.commit[i].count > high && commits.commit[i].count <= higher)
                *unit = selected ? SELECT HIGHEST SQUARE RESET : HIGHEST SQUARE RESET;
            else if (commits.commit[i].count > low && commits.commit[i].count <= high)
                *unit = selected ? SELECT HIGH SQUARE RESET : HIGH SQUARE RESET;
            else if (commits.commit[i].count > lower && commits.commit[i].count <= low)
                *unit = selected ? SELECT LOW SQUARE RESET : LOW SQUARE RESET;
            else if (commits.commit[i].count > 0 && commits.commit[i].count <= lower)
                *unit = selected ? SELECT LOWEST SQUARE RESET : LOWEST SQUARE RESET;
            else {
                PERROR;
                exit(EXIT_FAILURE);
            }
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
            case D_NORMAL :
            case D_RECOVER :
                _draw_unit(day, false, &unit);
                break;
            case D_SELECT :
                _draw_unit(day, true, &unit);
                break;
        }
    }

    dprintf(tty, "%s", unit);
    if (mode == D_NORMAL && day >= today - localtime(&today)->tm_wday * DAYSIZ)
        dprintf(tty, NEWLINE);
}

void draw_body_line(int mode)
{
    dprintf(tty, "\033[0K");

    time_t day;
    for (int i = cursor.x - 1; i < 53; i++) {
        day = body[cursor.y - 1][i];
        switch (mode) {
            case D_SELECT :
                if (i == cursor.x - 1)
                    draw_unit(day, D_SELECT);
                else
                    draw_unit(day, D_RECOVER);
                break;
            case D_RECOVER :
                draw_unit(day, D_RECOVER);
                break;
        }
    }
    move_back_unit(53 - cursor.x + 1);
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
            draw_unit(day, D_NORMAL);
        }
    }
}

void generate_cg()
{
    draw_header();
    draw_body();
    draw_foot();
    //free(commits->commit);
}

void cursor_init()
{
    cursor.x = 53;
    cursor.y = localtime(&today)->tm_wday + 1;
    dprintf(tty, "\033[%iA", 9);
    dprintf(tty, "\033[%iG", cursor.x * 2 + 4 - 1);
    dprintf(tty, "\033[%iB", cursor.y);
    //dprintf(tty, "\033[6n");

    draw_unit(body[cursor.y - 1][cursor.x - 1], D_SELECT);
    move_back_unit(53 - cursor.x + 1);
}

void draw_unit_under_cursor(void)
{
    draw_unit(body[cursor.y - 1][cursor.x - 1], D_SELECT);
}

void handle(char input)
{
    switch (input) {
        case 'q' :
            dprintf(tty, "\033[?25h");
            tty_reset();
            exit(EXIT_SUCCESS);
            break;
        case 'h' :
            draw_body_line(D_RECOVER);
            dprintf(tty, "\033[%iD", 2);
            cursor.x -= 1;
            draw_body_line(D_SELECT);
            break;
        case 'j' :
            draw_body_line(D_RECOVER);
            dprintf(tty, "\033[%iB", 1);
            cursor.y += 1;
            draw_body_line(D_SELECT);
            break;
        case 'k' :
            draw_body_line(D_RECOVER);
            dprintf(tty, "\033[%iA", 1);
            cursor.y -= 1;
            draw_body_line(D_SELECT);
            break;
        case 'l' :
            draw_body_line(D_RECOVER);
            dprintf(tty, "\033[%iC", 2);
            cursor.x += 1;
            draw_body_line(D_SELECT);
            break;
        default : ;
    }

}

void tty_run(void)
{
    tty = open("/dev/tty", O_RDWR);
    today = truncate_time(time(NULL));

    tty_init();

    /*
     * Q: hide cursor in terminal
     * thx: https://www.experts-exchange.com/questions/21115816/How-to-hide-cursor-in-a-Linux-Unix-console.html
     */
    dprintf(tty, "\033[?25l");

    FILE *stream;
    open_cg_file(&stream);
    parse_from_cg_file(&stream);
    generate_cg();
    fclose(stream);

    cursor_init();

    /* handle input */
    char input[1];
    while (read(tty, input, 1)) {
        handle(input[0]);
    }

    dprintf(tty, "\033[?25h");
    tty_reset();
}
