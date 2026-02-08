#include "validacoes/validacao_musicas.h"

#include "validacoes/validacao_comum.h"

bool musica_validar_sintatica(char **cols, int n) {
    if (!cols || n < 8) return false;
    if (cols[0][0] == '\0' || cols[3][0] == '\0') return false;
    if (!validacao_lista(cols[2])) return false;
    if (!validacao_duracao(cols[4])) return false;
    return true;
}
