#include "gestor_programas/gestor_interativo.h"

#include "gestor_programa/gestor_programa.h"
#include "gestor_programa/gestor_queries.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gestor_interativo_executar(void) {
    char dataset[512] = {0};
    char comando[1024] = {0};
    int idx = 1;
    gestor_programa_t *gp;

    printf("Introduza o caminho dos ficheiros de dados: ");
    if (!fgets(dataset, sizeof(dataset), stdin)) return 1;
    utils_strip_newline(dataset);
    if (dataset[0] == '\0') strcpy(dataset, "dataset");

    gp = gestor_programa_criar();
    if (!gp) return 1;
    if (gestor_programa_carregar_dataset(gp, dataset) != 0) {
        fprintf(stderr, "Erro ao carregar dataset.\n");
        gestor_programa_destruir(gp);
        return 1;
    }

    printf("Dataset carregado. Escreva 'exit' para terminar.\n");
    while (1) {
        printf("> ");
        if (!fgets(comando, sizeof(comando), stdin)) break;
        utils_strip_newline(comando);
        if (strcmp(comando, "exit") == 0) break;
        if (comando[0] == '\0') continue;
        gestor_queries_executar_comando(gp, comando, idx++);
        printf("Comando executado. Resultado em resultados/command%d_output.txt\n", idx - 1);
    }

    gestor_programa_destruir(gp);
    return 0;
}
