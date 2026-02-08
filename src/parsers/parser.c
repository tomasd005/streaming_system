#include "parsers/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64

static int split_simple(char *line, char sep, char **cols, int max_cols) {
    int n = 0;
    char *p = line;
    while (n < max_cols) {
        cols[n++] = p;
        char *s = strchr(p, sep);
        if (!s) break;
        *s = '\0';
        p = s + 1;
    }
    return n;
}

int parser_csv_stream(const char *ficheiro_csv, char separador, int has_header, parser_row_cb cb, void *ctx) {
    FILE *f = fopen(ficheiro_csv, "r");
    if (!f) return -1;

    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    int line_no = 0;
    int processed = 0;

    while ((len = getline(&line, &cap, f)) != -1) {
        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
        if (len > 0 && line[len - 1] == '\r') line[--len] = '\0';
        line_no++;
        if (has_header && line_no == 1) continue;

        char *cols[MAX_COLS] = {0};
        int n = split_simple(line, separador, cols, MAX_COLS);
        if (cb) cb(cols, n, ctx);
        processed++;
    }

    free(line);
    fclose(f);
    return processed;
}
