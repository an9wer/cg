#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
                    commits->max_count = MAX(commits->max_count, commits->commit[i].count);
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

void draw_body(commits_t *commits, time_t date, bool newline, char **output)
{
    char *plot;
    unsigned short flag = 0;
    for (int i = 0; i < commits->size; i++) {
        if (commits->commit[i].date == date) {
            if (commits->commit[i].count > commits->max_count / 4 * 3 && commits->commit[i].count <= commits->max_count)
                plot = newline ? HIGHEST SQUARE RESET NEWLINE : HIGHEST SQUARE RESET;
            else if (commits->commit[i].count > commits->max_count / 4 * 2 && commits->commit[i].count <= commits->max_count / 4 * 3)
                plot = newline ? HIGH SQUARE RESET NEWLINE : HIGH SQUARE RESET;
            else if (commits->commit[i].count > commits->max_count / 4 && commits->commit[i].count <= commits->max_count / 4 * 2)
                plot = newline ? LOW SQUARE RESET NEWLINE : LOW SQUARE RESET;
            else if (commits->commit[i].count > 0 && commits->commit[i].count <= commits->max_count / 4)
                plot = newline ? LOWEST SQUARE RESET NEWLINE : LOWEST SQUARE RESET;
            else {
                PERROR;
                exit(1);
            }
            *output = realloc(*output, strlen(*output) + strlen(plot) + 1);
            *output = strcat(*output, plot);
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        plot = newline ? SQUARE NEWLINE : SQUARE;
        *output = realloc(*output, strlen(*output) + strlen(plot) + 1);
        *output = strcat(*output, plot);
    }
}

int header_sort(const void *a, const void *b)
{
    if (((header_t *)a)->col > ((header_t *)b)->col) return 1;
    else return -1;
}

void draw_header(header_t *header)
{
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
    printf("    %s\n", output);
    free(output);
}

void draw_foot()
{
    printf("%*sless %s%s%s%s more\n", 92, "", LOWEST SQUARE RESET, LOW SQUARE RESET, HIGH SQUARE RESET, HIGHEST SQUARE RESET);
}

void generate_cg(commits_t *commits)
{
    header_t header[12];

    char *output = malloc(1);
    output[0] = '\0';

    char *month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char *week[7] = {"Sat ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sun "};
    time_t now = truncate_time(time(NULL));
    int now_wday = localtime(&now)->tm_wday;
    time_t latest_sun = now - DATESIZ * now_wday;
    time_t latest_day;
    time_t date;
    struct tm *tmp;
    for (int i = 0; i < 7; i++) {
        latest_day = latest_sun + i * DATESIZ;
        output = realloc(output, strlen(output) + strlen(week[i]) + 1);
        output = strcat(output, week[i]);
        for (int j = 52; j > 0; j--) {
            date = latest_day - j * 7 * DATESIZ;
            tmp = localtime(&date);
            if (tmp->tm_mday == 1) {
                header[tmp->tm_mon].col = (52 - j) * 2 + 3;
                header[tmp->tm_mon].mon = month[tmp->tm_mon];
            }
            draw_body(commits, date, false, &output);
        }
        if (i <= now_wday) {
            draw_body(commits, latest_day, true, &output);
        } else {
            output = realloc(output, strlen(output) + strlen(NEWLINE) + 1);
            output = strcat(output, NEWLINE);
        }
    }
    draw_header(header);
    printf("%s", output);
    draw_foot();
    free(output);
    free(commits->commit);
}
