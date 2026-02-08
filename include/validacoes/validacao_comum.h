#ifndef VALIDACAO_COMUM_H
#define VALIDACAO_COMUM_H

#include <stdbool.h>

bool validacao_data(const char *s);
bool validacao_datetime(const char *s);
bool validacao_duracao(const char *s);
bool validacao_email(const char *s);
bool validacao_lista(const char *s);

#endif
