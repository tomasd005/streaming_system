#include "validacoes/validacao_albuns.h"

#include "validacoes/validacao_comum.h"

bool album_validar_sintatica(char **cols, int n) {
    if (!cols || n < 5) return false;
    if (cols[0][0] == '\0') return false;
    if (!validacao_lista(cols[2])) return false;
    if (!validacao_lista(cols[4])) return false;
    return true;
}
