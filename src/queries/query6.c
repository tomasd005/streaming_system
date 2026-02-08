#include "queries/query6.h"
#include <stdio.h>

void query6_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    (void)gp;
    (void)args;
    fprintf(out ? out : stdout, "\n");
}
