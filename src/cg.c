#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "cg.h"

char *get_home_dir(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        PERROR;
        exit(1);
    }
    return pw->pw_dir;
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

void parse_from_cg_file(void)
{
}


#ifndef DEBUG
int main(void)
{
}
#endif
