#include "parsers/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64

static int split_csv_quoted_inplace(char *line, char sep, char **cols, int max_cols) {
    int n = 0;
    char *p = line;
    if (!line || !cols || max_cols <= 0) return -1;

    while (n < max_cols) {
        char *start = p;

        if (*p == '"') {
            char *r = p + 1;
            char *w = p;
            int closed = 0;

            while (*r) {
                if (*r == '"') {
                    if (r[1] == '"') {
                        *w++ = '"';
                        r += 2;
                        continue;
                    }
                    closed = 1;
                    r++;
                    break;
                }
                *w++ = *r++;
            }
            if (!closed) return -1;

            while (*r == ' ' || *r == '\t') r++;
            while (*r && *r != sep) {
                if (*r != ' ' && *r != '\t') return -1;
                r++;
            }

            *w = '\0';
            cols[n++] = start;
            if (!*r) break;

            p = r + 1;
            if (*p == '\0' && n < max_cols) cols[n++] = p;
            continue;
        }

        while (*p && *p != sep) p++;
        if (*p == sep) {
            *p = '\0';
            cols[n++] = start;
            p++;
            if (*p == '\0' && n < max_cols) cols[n++] = p;
            continue;
        }

        cols[n++] = start;
        break;
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

        work = strdup(line);
        if (!work) {
            free(work);
            continue;
        }

        ncols = split_csv_quoted_inplace(work, separador, cols, MAX_COLS);
        ok = (ncols > 0) ? cb(cols, ncols, line, ctx) : false;
        if (!ok && ferr) fprintf(ferr, "%s\n", line);

        free(work);
        processed++;
    }

    free(line);
    if (ferr) fclose(ferr);
    fclose(f);
    return processed;
}
