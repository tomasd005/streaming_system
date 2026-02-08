#ifndef GESTOR_QUERIES_H
#define GESTOR_QUERIES_H

#include "gestor_programa/gestor_programa.h"

/**
 * @file gestor_queries.h
 * @brief Interpretacao e execucao de comandos de queries.
 */

int gestor_queries_processar_ficheiro(gestor_programa_t *gp, const char *input_file);
int gestor_queries_executar_comando(gestor_programa_t *gp, const char *comando, int command_index);

#endif
