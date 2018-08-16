#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cg.h"

extern FILE *cg;

void generate_fake_data(void)
{
    char buffer[STRBUFSIZ];
    time_t now = time(NULL);

    char *home= get_home_dir();
    char cg_file[STRBUFSIZ];
    strcpy(cg_file, home);
    strcat(cg_file, CG_FILE);
    remove(cg_file);

    cg_open();
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 365 + 7; j++) {
            if (rand() % 7 == 0) {
                snprintf(buffer, STRBUFSIZ, "%ld\n", now - j * DAYSIZ);
                fwrite(buffer, sizeof(char), strlen(buffer), cg);
            }
        }
    }
    fclose(cg);
}

void test_parse(void)
{
    tty_run();
}

#ifdef DEBUG
int main(void)
{
    generate_fake_data();
    test_parse();
}
#endif
