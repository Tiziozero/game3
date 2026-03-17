#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;


    if (*s == 0)
        return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        end--;

    end[1] = '\0';
    return s;
}

static void set_env_var(const char *key, const char *value) {
#ifdef _WIN32
    _putenv_s(key, value);
#else
    setenv(key, value, 1);
#endif
}

int load_env(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("load_env fopen");
        return -1;
    }

    char line[2048];

    while (fgets(line, sizeof(line), f)) {
        // remove newline
        line[strcspn(line, "\r\n")] = 0;


        char *start = trim(line);

        // skip empty / comment
        if (*start == '\0' || *start == '#')
            continue;

        char *eq = strchr(start, '=');
        if (!eq)
            continue;

        *eq = '\0';

        char *key = trim(start);
        char *value = trim(eq + 1);

        // remove surrounding quotes
        if (*value == '"' || *value == '\'') {
            char quote = *value;
            value++;

            char *end = strrchr(value, quote);
            if (end)
                *end = '\0';
        }


        set_env_var(key, value);
    }

    fclose(f);
    return 0;
}
