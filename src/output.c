#include "output.h"

#include <stdio.h>

FILE *output_open_command_file(int command_index) {
    char path[256];
    snprintf(path, sizeof(path), "resultados/command%d_output.txt", command_index);
    return fopen(path, "w");
}
