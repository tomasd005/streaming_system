#include "gestor_programa/gestor_queries.h"

#include "output.h"
#include "queries/query1.h"
#include "queries/query2.h"
#include "queries/query3.h"
#include "queries/query4.h"
#include "queries/query5.h"
#include "queries/query6.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gestor_queries_executar_comando(gestor_programa_t *gp, const char *comando, int command_index) {
    char *copy;
    char *tok;
    FILE *out;

    if (!gp || !comando || command_index <= 0) return -1;

    out = output_open_command_file(command_index);
    if (!out) return -1;

    copy = strdup(comando);
    if (!copy) {
        fclose(out);
        return -1;
    }

    tok = strtok(copy, " \t\r\n");
    if (!tok) {
        fprintf(out, "\n");
        free(copy);
        fclose(out);
        return 0;
    }

    if (strcmp(tok, "1") == 0 || strcmp(tok, "1S") == 0) query1_executar(gp, comando, out);
    else if (strcmp(tok, "2") == 0 || strcmp(tok, "2S") == 0) query2_executar(gp, comando, out);
    else if (strcmp(tok, "3") == 0 || strcmp(tok, "3S") == 0) query3_executar(gp, comando, out);
    else if (strcmp(tok, "4") == 0 || strcmp(tok, "4S") == 0) query4_executar(gp, comando, out);
    else if (strcmp(tok, "5") == 0 || strcmp(tok, "5S") == 0) query5_executar(gp, comando, out);
    else if (strcmp(tok, "6") == 0 || strcmp(tok, "6S") == 0) query6_executar(gp, comando, out);
    else fprintf(out, "\n");

    free(copy);
    fclose(out);
    return 0;
}

int gestor_queries_processar_ficheiro(gestor_programa_t *gp, const char *input_file) {
    FILE *f;
    char *line;
    size_t cap;
    ssize_t n;
    int cmd_idx;

    if (!gp || !input_file) return -1;
    f = fopen(input_file, "r");
    if (!f) return -1;

    line = NULL;
    cap = 0;
    cmd_idx = 1;
    while ((n = getline(&line, &cap, f)) != -1) {
        (void)n;
        utils_strip_newline(line);
        gestor_queries_executar_comando(gp, line, cmd_idx);
        cmd_idx++;
    }

    free(line);
    fclose(f);
    return 0;
}
