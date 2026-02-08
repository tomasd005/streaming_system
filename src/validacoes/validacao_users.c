#include "validacoes/validacao_users.h"

#include "validacoes/validacao_comum.h"

#include <string.h>

bool user_validar_sintatica(char **cols, int n) {
    if (!cols || n < 8) return false;
    if (cols[0][0] == '\0') return false;
    if (!validacao_email(cols[1])) return false;
    if (!validacao_data(cols[4])) return false;
    if (strcmp(cols[6], "normal") != 0 && strcmp(cols[6], "premium") != 0) return false;
    if (!validacao_lista(cols[7])) return false;
    return true;
}
