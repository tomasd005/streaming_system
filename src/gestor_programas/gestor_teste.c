#include "gestor_programas/gestor_teste.h"

#include "gestor_programa/gestor_programa.h"
#include "gestor_programa/gestor_queries.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>

static double elapsed_s(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) / 1e9;
}

int gestor_teste_executar(const char *dataset_path, const char *input_file, const char *expected_dir) {
    struct timespec t0, t1;
    struct rusage r;
    gestor_programa_t *gp;

    (void)expected_dir;

    gp = gestor_programa_criar();
    if (!gp) return 1;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    if (gestor_programa_carregar_dataset(gp, dataset_path) != 0) {
        gestor_programa_destruir(gp);
        return 1;
    }
    if (gestor_queries_processar_ficheiro(gp, input_file) != 0) {
        gestor_programa_destruir(gp);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    getrusage(RUSAGE_SELF, &r);
    printf("Tempo total: %.3f s\n", elapsed_s(t0, t1));
    printf("Memoria utilizada: %.1f MB\n", (double)r.ru_maxrss / 1024.0);

    gestor_programa_destruir(gp);
    return 0;
}
