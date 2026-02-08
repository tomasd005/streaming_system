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

static void build_errors_path(const char *input_csv, char *out, size_t out_sz) {
    const char *base = strrchr(input_csv, '/');
    char name[256];
    const char *dot;
    size_t len;

    if (!out || out_sz == 0) return;
    base = base ? base + 1 : input_csv;
    dot = strrchr(base, '.');
    len = dot ? (size_t)(dot - base) : strlen(base);
    if (len >= sizeof(name)) len = sizeof(name) - 1;
    memcpy(name, base, len);
    name[len] = '\0';
    snprintf(out, out_sz, "resultados/%s_errors.csv", name);
}

int parser_csv_stream_with_errors(const char *ficheiro_csv, char separador,
                                  parser_row_validate_cb cb, void *ctx) {
    FILE *f;
    FILE *ferr;
    char err_path[512];
    char *line;
    size_t cap;
    ssize_t len;
    int line_no;
    int processed;

    if (!ficheiro_csv || !cb) return -1;

    f = fopen(ficheiro_csv, "r");
    if (!f) return -1;

    build_errors_path(ficheiro_csv, err_path, sizeof(err_path));
    ferr = fopen(err_path, "w");

    line = NULL;
    cap = 0;
    line_no = 0;
    processed = 0;

    while ((len = getline(&line, &cap, f)) != -1) {
        char *raw_copy;
        char *work;
        char *cols[MAX_COLS] = {0};
        int ncols;
        bool ok;

        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
        if (len > 0 && line[len - 1] == '\r') line[--len] = '\0';

        line_no++;
        if (line_no == 1) {
            if (ferr) fprintf(ferr, "%s\n", line);
            continue;
        }

        raw_copy = strdup(line);
        work = strdup(line);
        if (!raw_copy || !work) {
            free(raw_copy);
            free(work);
            continue;
        }

        ncols = split_simple(work, separador, cols, MAX_COLS);
        ok = cb(cols, ncols, raw_copy, ctx);
        if (!ok && ferr) fprintf(ferr, "%s\n", raw_copy);

        free(raw_copy);
        free(work);
        processed++;
    }

    free(line);
    if (ferr) fclose(ferr);
    fclose(f);
    return processed;
}
