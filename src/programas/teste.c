#include "gestor_programas/gestor_teste.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <pasta_dataset> <input.txt> <resultados_esperados>\n", argv[0]);
        return 1;
    }
    return gestor_teste_executar(argv[1], argv[2], argv[3]);
}
