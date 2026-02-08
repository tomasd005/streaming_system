#include "validacoes/validacao_artistas.h"

#include "validacoes/validacao_comum.h"

#include <glib.h>
#include <string.h>

bool artista_validar_sintatica(char **cols, int n) {
    char *type;
    if (!cols || n < 7) return false;
    if (cols[0][0] == '\0') return false;
    if (!validacao_lista(cols[4])) return false;

    type = g_ascii_strdown(cols[6], -1);
    if (!type) return false;
    if (strcmp(type, "individual") != 0 && strcmp(type, "group") != 0) {
        g_free(type);
        return false;
    }
    g_free(type);
    return true;
}
