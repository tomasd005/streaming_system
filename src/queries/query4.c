#include "queries/query4.h"
#include <stdio.h>

void query4_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    (void)gp;
    (void)args;
    fprintf(out ? out : stdout, "\n");
}
