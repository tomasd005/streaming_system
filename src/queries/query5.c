#include "queries/query5.h"
#include <stdio.h>

void query5_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    (void)gp;
    (void)args;
    fprintf(out ? out : stdout, "\n");
}
