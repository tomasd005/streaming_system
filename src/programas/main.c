#include "main.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <pasta_dataset> <input.txt>\n", argv[0]);
        return 1;
    }
    return executar_programa_principal(argv[1], argv[2]);
}
