#include "gestor_programas/gestor_teste.h"

#include "gestor_programa/gestor_programa.h"
#include "gestor_programa/gestor_queries.h"
#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#define PARSE_MAX_THREADS 16
#define PARSE_BATCH_DEFAULT 2048
#define PARSE_BATCH_MAX 65536

typedef struct {
    unsigned int total_cmds;
    unsigned int ok_cmds;
    unsigned int fail_cmds;
    unsigned int per_query_count[7];
    unsigned int per_query_fail[7];
    double per_query_time[7];
    double load_time_s;
    double query_time_s;
    double total_time_s;
    double memory_mb;
    int parse_threads;
    int parse_batch;
    int first_failed_cmd;
    int first_failed_line;
    char first_failed_reason[512];
} benchmark_stats_t;

static double elapsed_s(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) / 1e9;
}

static int parse_threads_from_env(void) {
    const char *s = getenv("LI3_PARSE_THREADS");
    char *end = NULL;
    long v;
    if (!s || *s == '\0') return 1;
    v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || v < 1) return 1;
    if (v > PARSE_MAX_THREADS) v = PARSE_MAX_THREADS;
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

static int env_flag_enabled(const char *name) {
    const char *v = getenv(name);
    if (!v) return 0;
    return strcmp(v, "1") == 0 || strcmp(v, "true") == 0 || strcmp(v, "TRUE") == 0;
}

static int query_id_from_command(const char *line) {
    const unsigned char *p = (const unsigned char *)line;
    while (*p && isspace(*p)) p++;
    if (*p >= '1' && *p <= '6') return (int)(*p - '0');
    return 0;
}

static void line_preview(const char *line, char *out, size_t out_sz) {
    size_t i = 0;
    if (!out || out_sz == 0) return;
    if (!line) {
        out[0] = '\0';
        return;
    }
    while (line[i] && line[i] != '\n' && line[i] != '\r' && i + 1 < out_sz) {
        out[i] = line[i];
        i++;
    }
    out[i] = '\0';
}

static int compare_output_files(const char *expected_path, const char *actual_path, int *diff_line,
                                char *reason, size_t reason_sz) {
    FILE *fe = fopen(expected_path, "r");
    FILE *fa = fopen(actual_path, "r");
    char *le = NULL;
    char *la = NULL;
    size_t ce = 0;
    size_t ca = 0;
    ssize_t ne, na;
    int line = 1;
    int rc = 0;

    if (!fe) {
        if (reason) snprintf(reason, reason_sz, "Ficheiro esperado inexistente: %.240s", expected_path);
        return -1;
    }
    if (!fa) {
        fclose(fe);
        if (reason) snprintf(reason, reason_sz, "Ficheiro obtido inexistente: %.240s", actual_path);
        return -1;
    }

    while (1) {
        char pe[160];
        char pa[160];
        ne = getline(&le, &ce, fe);
        na = getline(&la, &ca, fa);

        if (ne == -1 && na == -1) break;
        if (ne == -1 || na == -1 || strcmp(le, la) != 0) {
            if (diff_line) *diff_line = line;
            line_preview(ne == -1 ? "<EOF>" : le, pe, sizeof(pe));
            line_preview(na == -1 ? "<EOF>" : la, pa, sizeof(pa));
            if (reason) {
                snprintf(reason, reason_sz, "Diferenca na linha %d (esperado='%s' obtido='%s')",
                         line, pe, pa);
            }
            rc = 1;
            break;
        }
        line++;
    }

    free(le);
    free(la);
    fclose(fe);
    fclose(fa);
    return rc;
}

static void benchmark_write_json(const benchmark_stats_t *s) {
    FILE *f = fopen("resultados/benchmark.json", "w");
    int q;
    if (!f || !s) {
        if (f) fclose(f);
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"commands\": {\"total\": %u, \"ok\": %u, \"fail\": %u},\n",
            s->total_cmds, s->ok_cmds, s->fail_cmds);
    fprintf(f, "  \"timings\": {\"load_s\": %.6f, \"queries_s\": %.6f, \"total_s\": %.6f},\n",
            s->load_time_s, s->query_time_s, s->total_time_s);
    fprintf(f, "  \"memory_mb\": %.3f,\n", s->memory_mb);
    fprintf(f, "  \"parse_threads\": %d,\n", s->parse_threads);
    fprintf(f, "  \"parse_batch\": %d,\n", s->parse_batch);
    fprintf(f, "  \"queries\": {\n");
    for (q = 1; q <= 6; q++) {
        double avg = s->per_query_count[q] ? (s->per_query_time[q] / s->per_query_count[q]) : 0.0;
        double pct = s->query_time_s > 0.0 ? (100.0 * s->per_query_time[q] / s->query_time_s) : 0.0;
        fprintf(f,
                "    \"q%d\": {\"count\": %u, \"fail\": %u, \"time_s\": %.6f, \"avg_s\": %.6f, "
                "\"pct\": %.2f}%s\n",
                q, s->per_query_count[q], s->per_query_fail[q], s->per_query_time[q], avg, pct,
                (q == 6) ? "" : ",");
    }
    fprintf(f, "  },\n");
    fprintf(f,
            "  \"first_failure\": {\"command\": %d, \"line\": %d, \"reason\": \"%s\"}\n",
            s->first_failed_cmd, s->first_failed_line, s->first_failed_reason);
    fprintf(f, "}\n");
    fclose(f);
}

static void benchmark_print_summary(const benchmark_stats_t *st) {
    int q;

    printf("Tempo carregamento: %.3f s\n", st->load_time_s);
    printf("Tempo queries: %.3f s\n", st->query_time_s);
    printf("Tempo total: %.3f s\n", st->total_time_s);
    printf("Memoria utilizada: %.1f MB\n", st->memory_mb);
    printf("Parse threads: %d\n", st->parse_threads);
    printf("Parse batch: %d\n", st->parse_batch);
    printf("Comandos: %u | OK: %u | FAIL: %u\n", st->total_cmds, st->ok_cmds, st->fail_cmds);

    for (q = 1; q <= 6; q++) {
        double avg = st->per_query_count[q] ? st->per_query_time[q] / st->per_query_count[q] : 0.0;
        double pct = st->query_time_s > 0.0 ? (100.0 * st->per_query_time[q] / st->query_time_s) : 0.0;
        printf("Q%d -> execucoes:%u fail:%u tempo_total:%.6f s tempo_medio:%.6f s (%.2f%%)\n",
               q, st->per_query_count[q], st->per_query_fail[q], st->per_query_time[q], avg, pct);
    }

    if (st->fail_cmds > 0) {
        printf("Primeira discrepancia: comando %d", st->first_failed_cmd);
        if (st->first_failed_line > 0) printf(", linha %d", st->first_failed_line);
        printf(" -> %s\n", st->first_failed_reason);
    }
}

static int run_benchmark_once(const char *dataset_path, const char *input_file, const char *expected_dir,
                              benchmark_stats_t *st) {
    struct timespec t0, t_load_start, t_load_end, t_end;
    struct rusage r;
    gestor_programa_t *gp;
    FILE *fin;
    char *line = NULL;
    size_t cap = 0;
    ssize_t n;
    int cmd_idx;

    if (!st) return -1;
    memset(st, 0, sizeof(*st));
    st->parse_threads = parse_threads_from_env();
    st->parse_batch = parse_batch_from_env();

    gp = gestor_programa_criar();
    if (!gp) return -1;

    fin = fopen(input_file, "r");
    if (!fin) {
        gestor_programa_destruir(gp);
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &t0);
    clock_gettime(CLOCK_MONOTONIC, &t_load_start);
    if (gestor_programa_carregar_dataset(gp, dataset_path) != 0) {
        fclose(fin);
        gestor_programa_destruir(gp);
        return -1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t_load_end);
    st->load_time_s = elapsed_s(t_load_start, t_load_end);

    cmd_idx = 1;
    while ((n = getline(&line, &cap, fin)) != -1) {
        struct timespec q0, q1;
        int qid;
        int diff_line = 0;
        char expected_path[512];
        char actual_path[256];
        char reason[512] = {0};
        int cmp_rc = 0;
        int exec_rc;
        double dt;

        (void)n;
        utils_strip_newline(line);
        qid = query_id_from_command(line);
        st->total_cmds++;

        clock_gettime(CLOCK_MONOTONIC, &q0);
        exec_rc = gestor_queries_executar_comando(gp, line, cmd_idx);
        clock_gettime(CLOCK_MONOTONIC, &q1);
        dt = elapsed_s(q0, q1);

        st->query_time_s += dt;
        if (qid >= 1 && qid <= 6) {
            st->per_query_count[qid]++;
            st->per_query_time[qid] += dt;
        }

        if (expected_dir && expected_dir[0] != '\0') {
            snprintf(expected_path, sizeof(expected_path), "%s/command%d_output.txt", expected_dir, cmd_idx);
            snprintf(actual_path, sizeof(actual_path), "resultados/command%d_output.txt", cmd_idx);
            cmp_rc = compare_output_files(expected_path, actual_path, &diff_line, reason, sizeof(reason));
        }

        if (exec_rc == 0 && cmp_rc == 0) {
            st->ok_cmds++;
        } else {
            st->fail_cmds++;
            if (qid >= 1 && qid <= 6) st->per_query_fail[qid]++;
            if (st->first_failed_cmd == 0) {
                st->first_failed_cmd = cmd_idx;
                st->first_failed_line = diff_line;
                if (exec_rc != 0) {
                    snprintf(st->first_failed_reason, sizeof(st->first_failed_reason),
                             "Falha a executar comando");
                } else {
                    snprintf(st->first_failed_reason, sizeof(st->first_failed_reason), "%s", reason);
                }
            }
        }
        cmd_idx++;
    }

    free(line);
    fclose(fin);

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    st->total_time_s = elapsed_s(t0, t_end);

    getrusage(RUSAGE_SELF, &r);
    st->memory_mb = (double)r.ru_maxrss / 1024.0;

    gestor_programa_destruir(gp);
    return st->fail_cmds == 0 ? 0 : 1;
}

static int run_autotune_parse(const char *dataset_path, const char *input_file, const char *expected_dir,
                              int *best_threads, int *best_batch) {
    const int thread_candidates[] = {1, 2, 4, 8, 12, 16};
    const int batch_candidates[] = {512, 1024, 2048, 4096, 8192, 16384};
    const size_t nt = sizeof(thread_candidates) / sizeof(thread_candidates[0]);
    const size_t nb = sizeof(batch_candidates) / sizeof(batch_candidates[0]);
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    int have_best = 0;
    double best_time = 0.0;
    int bt = 1;
    int bb = PARSE_BATCH_DEFAULT;
    FILE *csv = fopen("resultados/autotune_parse.csv", "w");
    size_t i, j;

    if (cores < 1) cores = 1;
    if (csv) {
        fprintf(csv, "threads,batch,total_s,load_s,queries_s,memory_mb,ok,fail\n");
    }

    printf("Autotune parse: a testar configuracoes...\n");
    for (i = 0; i < nt; i++) {
        for (j = 0; j < nb; j++) {
            benchmark_stats_t st;
            int t = thread_candidates[i];
            int b = batch_candidates[j];
            int rc;
            char ts[16];
            char bs[16];

            if (t > cores) continue;

            snprintf(ts, sizeof(ts), "%d", t);
            snprintf(bs, sizeof(bs), "%d", b);
            setenv("LI3_PARSE_THREADS", ts, 1);
            setenv("LI3_PARSE_BATCH", bs, 1);

            rc = run_benchmark_once(dataset_path, input_file, expected_dir, &st);
            printf("AUTO threads=%d batch=%d -> total=%.3f s mem=%.1f MB fail=%u\n",
                   t, b, st.total_time_s, st.memory_mb, st.fail_cmds);
            if (csv) {
                fprintf(csv, "%d,%d,%.6f,%.6f,%.6f,%.3f,%u,%u\n", t, b,
                        st.total_time_s, st.load_time_s, st.query_time_s,
                        st.memory_mb, st.ok_cmds, st.fail_cmds);
            }

            if (rc == 0 && (!have_best || st.total_time_s < best_time)) {
                have_best = 1;
                best_time = st.total_time_s;
                bt = t;
                bb = b;
            }
        }
    }
    if (csv) fclose(csv);

    if (!have_best) return 1;
    *best_threads = bt;
    *best_batch = bb;
    return 0;
}

int gestor_teste_executar(const char *dataset_path, const char *input_file, const char *expected_dir) {
    benchmark_stats_t st;
    int rc;

    if (env_flag_enabled("LI3_AUTOTUNE_PARSE")) {
        int bt = 1;
        int bb = PARSE_BATCH_DEFAULT;
        if (run_autotune_parse(dataset_path, input_file, expected_dir, &bt, &bb) == 0) {
            char ts[16];
            char bs[16];
            snprintf(ts, sizeof(ts), "%d", bt);
            snprintf(bs, sizeof(bs), "%d", bb);
            setenv("LI3_PARSE_THREADS", ts, 1);
            setenv("LI3_PARSE_BATCH", bs, 1);
            printf("Melhor configuracao parse: threads=%d batch=%d\n", bt, bb);
        } else {
            printf("Autotune parse: nenhuma configuracao valida encontrada; a usar defaults.\n");
            unsetenv("LI3_PARSE_THREADS");
            unsetenv("LI3_PARSE_BATCH");
        }
    }

    rc = run_benchmark_once(dataset_path, input_file, expected_dir, &st);
    if (rc < 0) return 1;

    benchmark_print_summary(&st);
    benchmark_write_json(&st);
    printf("Benchmark JSON: resultados/benchmark.json\n");
    if (env_flag_enabled("LI3_AUTOTUNE_PARSE")) {
        printf("Autotune CSV: resultados/autotune_parse.csv\n");
    }

    return rc;
}
