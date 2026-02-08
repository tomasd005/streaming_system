#include "validacoes/validacao_historico.h"

#include "validacoes/validacao_comum.h"

#include <glib.h>
#include <string.h>

bool historico_validar_sintatica(char **cols, int n) {
    char *platform;
    if (!cols || n < 6) return false;
    if (cols[0][0] == '\0') return false;
    if (!validacao_datetime(cols[3])) return false;
    if (!validacao_duracao(cols[4])) return false;

    platform = g_ascii_strdown(cols[5], -1);
    if (!platform) return false;
    if (strcmp(platform, "mobile") != 0 && strcmp(platform, "desktop") != 0) {
        g_free(platform);
        return false;
    }
    g_free(platform);
    return true;
}
