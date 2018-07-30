#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "cg.h"

#ifndef DEBUG
int main(void)
{
}
#endif

char *get_home_dir(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        PERROR;
        exit(1);
    }
    return pw->pw_dir;
}

time_t truncate_time(time_t time)
{
    return time / DATESIZ * DATESIZ;
}

void get_cg_file(char *cg_file, size_t max)
{
    char *home= get_home_dir();

    if (strlen(home) + strlen(CG_FILE) + 1 > max) {
        PERROR;
        exit(1);
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
            exit(1);
        }
    } else {
        *stream = fopen(cg_file, "w");
        fclose(*stream);
    }

    *stream = fopen(cg_file, "r+");
    rewind(*stream);
}

void parse_from_cg_file(FILE **stream, commits_t *commits)
{
    char buffer[STRBUFSIZ];
    time_t date;
    unsigned short flag;
    do {
        if (fgets(buffer, STRBUFSIZ, *stream)) {
            date = truncate_time(atol(buffer));

            flag = 0;
            for (int i = 0; i < commits->size; i++) {
                if (date == commits->commit[i].date) {
                    commits->commit[i].count += 1;
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) {
                commits->size += 1;
                commits->commit = realloc(commits->commit, sizeof(commit_t) * commits->size);
                commits->commit[commits->size-1].date = date;
                commits->commit[commits->size-1].count = 1;
            }
        } else if (ferror(*stream)) {
            PERROR;
            exit(1);
        }
    } while (!feof(*stream));
}

void generate_cg(commits_t *commits)
{
    char * output = malloc(1);
    output = strcat(output, "\0");

    time_t now = truncate_time(time(NULL));
    struct tm *tmp = localtime(&now);
    time_t latest_sun = now - DATESIZ * tmp->tm_wday;
    time_t latest_day;
    time_t date;
    unsigned short flag;
    for (int i = 0; i < 7; i++) {
        latest_day = latest_sun + i * DATESIZ;
        for (int j = 52; j > 0; j--) {
            date = latest_day - j * 7 * DATESIZ;
            flag = 0;
            for (int k = 0; k < commits->size; k++) {
                if (commits->commit[k].date == date) {
                    output = realloc(output, strlen(output) + sizeof(GREEN SQUARE RESET) + 1);
                    output = strcat(output, GREEN SQUARE RESET);
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) {
                output = realloc(output, strlen(output) + sizeof(SQUARE) + 1);
                output = strcat(output, SQUARE);
            }
        }
        if (i <= tmp->tm_wday) {
            flag = 0;
            for (int k = 0; k < commits->size; k++) {
                if (commits->commit[k].date == latest_day) {
                    output = realloc(output, strlen(output) + sizeof(GREEN SQUARE RESET "\n") + 1);
                    output = strcat(output, GREEN SQUARE RESET "\n");
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) {
                output = realloc(output, strlen(output) + sizeof(SQUARE "\n") + 1);
                output = strcat(output, SQUARE "\n");
            }
        } else {
            output = realloc(output, strlen(output) + sizeof("\n"));
            output = strcat(output, "\n");
        }
    }
    printf("%s", output);
}
