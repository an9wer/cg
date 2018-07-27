#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cg.h"

void write_time_to_cg_file(void)
{
    FILE *stream;
    time_t now = time(NULL);

    open_cg_file(&stream);
    char buffer[STRBUFSIZ];
    snprintf(buffer, STRBUFSIZ, "%ld", now);
    fwrite(buffer, sizeof(char), strlen(buffer), stream);
    
    fclose(stream);
}

void parse_time_from_cg_file(void)
{
    FILE *stream;
    char buffer[STRBUFSIZ];

    open_cg_file(&stream);
    fread(buffer, sizeof(char), STRBUFSIZ, stream);
    time_t now = atol(buffer);
    printf("%ld\n", now);

    struct tm *tmp = localtime(&now);
    strftime(buffer, STRBUFSIZ, "%r, %a %b %d, %Y", tmp);
    puts(buffer);
}

int main(void)
{
    write_time_to_cg_file();
    parse_time_from_cg_file();
}
