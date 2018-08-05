#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cg.h"

void generate_fake_data(void)
{
    FILE *stream;
    char buffer[STRBUFSIZ];
    time_t now = time(NULL);

    char *home= get_home_dir();
    char cg_file[STRBUFSIZ];
    strcpy(cg_file, home);
    strcat(cg_file, CG_FILE);
    remove(cg_file);

    open_cg_file(&stream);
    srand(time(NULL));
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 365; j++) {
            if (rand() % 7 == 0) {
                snprintf(buffer, STRBUFSIZ, "%ld\n", now - j * DAYSIZ);
                fwrite(buffer, sizeof(char), strlen(buffer), stream);
            }
        }
    }
    fclose(stream);
}

void test_parse(void)
{
    FILE *stream;
    commits_t commits = {0, NULL, 1};

    open_cg_file(&stream);
    parse_from_cg_file(&stream, &commits);
    generate_cg(&commits);

    fclose(stream);
}

int main(void)
{
    generate_fake_data();
    test_parse();
}
