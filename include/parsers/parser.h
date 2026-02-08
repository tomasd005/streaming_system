#ifndef PARSER_H
#define PARSER_H

/**
 * @file parser.h
 * @brief Parser CSV generico para carregar entidades.
 */

typedef void (*parser_row_cb)(char **colunas, int n_colunas, void *ctx);

/**
 * @brief Processa um CSV em streaming linha a linha.
 * @param ficheiro_csv Caminho do CSV.
 * @param separador Separador de colunas.
 * @param has_header Se true, ignora primeira linha.
 * @param cb Callback por linha.
 * @param ctx Contexto de callback.
 * @return Numero de linhas processadas, ou -1 em erro.
 */
int parser_csv_stream(const char *ficheiro_csv, char separador, int has_header, parser_row_cb cb, void *ctx);

#endif
