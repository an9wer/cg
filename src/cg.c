#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cg.h"

char *get_home_dir(void)
{
    struct passwd *pw = getpwuid(getuid());
    // TODO: if (pw == NULL)
    return pw->pw_dir;
}

void get_cg_file(char *cg_file, size_t max)
{
    char *home= get_home_dir();

    if (strlen(home) + strlen(CG_FILE) + 1 > max) {
        // TODO: Error
        fprintf(stderr, "Error: get_cg_file");
        exit(1);
    }
    cg_file = strcpy(cg_file, home);
    cg_file = strcat(cg_file, CG_FILE);
}

void cg_read(void)
{
    char cg_file[STRBUFSIZ];
    get_cg_file(cg_file, STRBUFSIZ);

    FILE *fp = fopen(cg_file, "r");
    // TODO: if (fp == NULL)

    size_t capacity = REDBUFSIZ, buffer_size = 0;
    char *buffer = malloc(capacity - buffer_size);
    while ((buffer_size += fread(buffer + buffer_size, sizeof(char), capacity - buffer_size, fp)) == capacity) {
        capacity *= 1.5;
        buffer = realloc(buffer, capacity);
    }
    printf("%s", buffer);

    free(buffer);
    fclose(fp);
}

int main(void)
{
    cg_read();
}
