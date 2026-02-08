#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

typedef bool (*parser_row_validate_cb)(char **colunas, int n_colunas, const char *raw_line,
                                       void *ctx);

int parser_csv_stream_with_errors(const char *ficheiro_csv, char separador,
                                  parser_row_validate_cb cb, void *ctx);

#endif
