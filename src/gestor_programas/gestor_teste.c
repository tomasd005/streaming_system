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
    if (v > 16) v = 16;
    return (int)v;
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

int gestor_teste_executar(const char *dataset_path, const char *input_file, const char *expected_dir) {
    struct timespec t0, t_load_start, t_load_end, t_end;
    struct rusage r;
    gestor_programa_t *gp;
    benchmark_stats_t st = {0};
    FILE *fin;
    char *line;
    size_t cap;
    ssize_t n;
    int cmd_idx;

    gp = gestor_programa_criar();
    if (!gp) return 1;
    st.parse_threads = parse_threads_from_env();

    fin = fopen(input_file, "r");
    if (!fin) {
        gestor_programa_destruir(gp);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &t0);
    clock_gettime(CLOCK_MONOTONIC, &t_load_start);
    if (gestor_programa_carregar_dataset(gp, dataset_path) != 0) {
        fclose(fin);
        gestor_programa_destruir(gp);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t_load_end);
    st.load_time_s = elapsed_s(t_load_start, t_load_end);

    line = NULL;
    cap = 0;
    cmd_idx = 1;
    while ((n = getline(&line, &cap, fin)) != -1) {
        struct timespec q0, q1;
        int qid;
        int diff_line = 0;
        char expected_path[512];
        char actual_path[256];
        char reason[512] = {0};
        int cmp_rc;
        int exec_rc;
        double dt;

        (void)n;
        utils_strip_newline(line);
        qid = query_id_from_command(line);
        st.total_cmds++;

        clock_gettime(CLOCK_MONOTONIC, &q0);
        exec_rc = gestor_queries_executar_comando(gp, line, cmd_idx);
        clock_gettime(CLOCK_MONOTONIC, &q1);
        dt = elapsed_s(q0, q1);

        st.query_time_s += dt;
        if (qid >= 1 && qid <= 6) {
            st.per_query_count[qid]++;
            st.per_query_time[qid] += dt;
        }

        snprintf(expected_path, sizeof(expected_path), "%s/command%d_output.txt", expected_dir, cmd_idx);
        snprintf(actual_path, sizeof(actual_path), "resultados/command%d_output.txt", cmd_idx);
        cmp_rc = compare_output_files(expected_path, actual_path, &diff_line, reason, sizeof(reason));

        if (exec_rc == 0 && cmp_rc == 0) {
            st.ok_cmds++;
        } else {
            st.fail_cmds++;
            if (qid >= 1 && qid <= 6) st.per_query_fail[qid]++;
            if (st.first_failed_cmd == 0) {
                st.first_failed_cmd = cmd_idx;
                st.first_failed_line = diff_line;
                if (exec_rc != 0) {
                    snprintf(st.first_failed_reason, sizeof(st.first_failed_reason),
                             "Falha a executar comando");
                } else {
                    snprintf(st.first_failed_reason, sizeof(st.first_failed_reason), "%s", reason);
                }
            }
        }
        cmd_idx++;
    }
    free(line);
    fclose(fin);

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    st.total_time_s = elapsed_s(t0, t_end);

    getrusage(RUSAGE_SELF, &r);
    st.memory_mb = (double)r.ru_maxrss / 1024.0;

    printf("Tempo carregamento: %.3f s\n", st.load_time_s);
    printf("Tempo queries: %.3f s\n", st.query_time_s);
    printf("Tempo total: %.3f s\n", st.total_time_s);
    printf("Memoria utilizada: %.1f MB\n", st.memory_mb);
    printf("Parse threads: %d\n", st.parse_threads);
    printf("Comandos: %u | OK: %u | FAIL: %u\n", st.total_cmds, st.ok_cmds, st.fail_cmds);

    {
        int q;
        for (q = 1; q <= 6; q++) {
            double avg = st.per_query_count[q] ? st.per_query_time[q] / st.per_query_count[q] : 0.0;
            double pct = st.query_time_s > 0.0 ? (100.0 * st.per_query_time[q] / st.query_time_s) : 0.0;
            printf("Q%d -> execucoes:%u fail:%u tempo_total:%.6f s tempo_medio:%.6f s (%.2f%%)\n",
                   q, st.per_query_count[q], st.per_query_fail[q], st.per_query_time[q], avg, pct);
        }
    }

    if (st.fail_cmds > 0) {
        printf("Primeira discrepancia: comando %d", st.first_failed_cmd);
        if (st.first_failed_line > 0) printf(", linha %d", st.first_failed_line);
        printf(" -> %s\n", st.first_failed_reason);
    }

    benchmark_write_json(&st);
    printf("Benchmark JSON: resultados/benchmark.json\n");

    gestor_programa_destruir(gp);
    return st.fail_cmds == 0 ? 0 : 1;
}
