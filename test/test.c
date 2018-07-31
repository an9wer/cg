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

    open_cg_file(&stream);
    for (int i = 0; i < 100; i++) {
        snprintf(buffer, STRBUFSIZ, "%ld\n", now - i * DATESIZ);
        fwrite(buffer, sizeof(char), strlen(buffer), stream);
    }
    fclose(stream);
}

void test_parse(void)
{
    FILE *stream;
    commits_t commits = {0, NULL};

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
