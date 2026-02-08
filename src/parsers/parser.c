#include "parsers/parser.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64
#define PARSE_BATCH_DEFAULT 2048
#define PARSE_BATCH_MAX 65536
#define PARSE_MAX_THREADS 16

typedef struct {
    char *raw;
    char *work;
    char *cols[MAX_COLS];
    int ncols;
    bool parse_ok;
} parse_job_t;

typedef struct {
    parse_job_t *jobs;
    int count;
    int tid;
    int threads;
    char sep;
} parse_worker_args_t;

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

static void parse_job_run(parse_job_t *job, char sep) {
    if (!job || !job->raw) return;
    job->work = strdup(job->raw);
    if (!job->work) return;

    job->ncols = split_csv_quoted_inplace(job->work, sep, job->cols, MAX_COLS);
    job->parse_ok = (job->ncols > 0);
}

static void *parse_worker_run(void *arg) {
    parse_worker_args_t *a = (parse_worker_args_t *)arg;
    int i;

    if (!a || !a->jobs || a->count <= 0 || a->threads <= 0) return NULL;

    for (i = a->tid; i < a->count; i += a->threads) {
        parse_job_run(&a->jobs[i], a->sep);
    }
    return NULL;
}

static int parse_threads_from_env(void) {
    const char *s = getenv("LI3_PARSE_THREADS");
    char *end = NULL;
    long v;

    if (!s || *s == '\0') return 1;
    v = strtol(s, &end, 10);
    if (end == s || *end != '\0') return 1;
    if (v < 1) return 1;
    if (v > PARSE_MAX_THREADS) return PARSE_MAX_THREADS;
    return (int)v;
}

static int parse_batch_from_env(void) {
    const char *s = getenv("LI3_PARSE_BATCH");
    char *end = NULL;
    long v;

    if (!s || *s == '\0') return PARSE_BATCH_DEFAULT;
    v = strtol(s, &end, 10);
    if (end == s || *end != '\0') return PARSE_BATCH_DEFAULT;
    if (v < 32) return 32;
    if (v > PARSE_BATCH_MAX) return PARSE_BATCH_MAX;
    return (int)v;
}

static void trim_eol(char *line, ssize_t *len) {
    if (!line || !len || *len <= 0) return;
    if (*len > 0 && (line[*len - 1] == '\n' || line[*len - 1] == '\r')) line[--(*len)] = '\0';
    if (*len > 0 && line[*len - 1] == '\r') line[--(*len)] = '\0';
}

static int process_batch(parse_job_t *jobs, int count, int threads, char sep,
                         parser_row_validate_cb cb, void *ctx, FILE *ferr) {
    int i;
    int processed = 0;

    if (!jobs || count <= 0 || !cb) return 0;

    if (threads > 1 && count >= threads) {
        pthread_t tids[PARSE_MAX_THREADS];
        parse_worker_args_t args[PARSE_MAX_THREADS];
        int t;
        int created = 0;

        if (threads > PARSE_MAX_THREADS) threads = PARSE_MAX_THREADS;

        for (t = 0; t < threads; t++) {
            args[t].jobs = jobs;
            args[t].count = count;
            args[t].tid = t;
            args[t].threads = threads;
            args[t].sep = sep;
            if (pthread_create(&tids[t], NULL, parse_worker_run, &args[t]) == 0) {
                created++;
            } else {
                break;
            }
        }
        for (t = 0; t < created; t++) {
            pthread_join(tids[t], NULL);
        }

        if (created != threads) {
            for (i = 0; i < count; i++) {
                if (!jobs[i].work && jobs[i].raw) parse_job_run(&jobs[i], sep);
            }
        }
    } else {
        for (i = 0; i < count; i++) parse_job_run(&jobs[i], sep);
    }

    for (i = 0; i < count; i++) {
        bool ok = false;
        if (jobs[i].parse_ok) ok = cb(jobs[i].cols, jobs[i].ncols, jobs[i].raw, ctx);
        if (!ok && ferr && jobs[i].raw) fprintf(ferr, "%s\n", jobs[i].raw);
        free(jobs[i].work);
        free(jobs[i].raw);
        jobs[i].work = NULL;
        jobs[i].raw = NULL;
        processed++;
    }
    return processed;
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
    int parse_threads;
    parse_job_t *batch;
    int batch_cap;
    int batch_count;

    if (!ficheiro_csv || !cb) return -1;

    f = fopen(ficheiro_csv, "r");
    if (!f) return -1;

    build_errors_path(ficheiro_csv, err_path, sizeof(err_path));
    ferr = fopen(err_path, "w");

    line = NULL;
    cap = 0;
    line_no = 0;
    processed = 0;
    parse_threads = parse_threads_from_env();
    batch_cap = parse_batch_from_env();
    batch = calloc((size_t)batch_cap, sizeof(*batch));
    if (!batch) {
        free(line);
        if (ferr) fclose(ferr);
        fclose(f);
        return -1;
    }
    batch_count = 0;

    while ((len = getline(&line, &cap, f)) != -1) {
        trim_eol(line, &len);

        line_no++;
        if (line_no == 1) {
            if (ferr) fprintf(ferr, "%s\n", line);
            continue;
        }

        if (batch_count == batch_cap) {
            processed += process_batch(batch, batch_count, parse_threads, separador, cb, ctx, ferr);
            batch_count = 0;
        }

        batch[batch_count].raw = strdup(line);
        batch[batch_count].work = NULL;
        batch[batch_count].ncols = 0;
        batch[batch_count].parse_ok = false;
        if (!batch[batch_count].raw) {
            if (ferr) fprintf(ferr, "%s\n", line);
            continue;
        }
        batch_count++;
    }

    if (batch_count > 0) {
        processed += process_batch(batch, batch_count, parse_threads, separador, cb, ctx, ferr);
    }

    free(batch);
    free(line);
    if (ferr) fclose(ferr);
    fclose(f);
    return processed;
}
