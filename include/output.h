#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>

/**
 * @file output.h
 * @brief Escrita de resultados das queries.
 */

/**
 * @brief Abre ficheiro de output para um comando.
 * @param command_index Indice do comando (1..N).
 * @return FILE* aberto em escrita, ou NULL.
 */
FILE *output_open_command_file(int command_index);

#endif
