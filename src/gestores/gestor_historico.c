#include "gestores/gestor_historico.h"

#include "parsers/parser.h"
#include "utils.h"
#include "validacoes/validacao_historico.h"

#include <glib.h>

struct gestor_historico {
    GPtrArray *rows;
};

typedef struct {
    gestor_historico_t *g;
    const gestor_users_t *users;
    const gestor_musicas_t *musicas;
} load_ctx_hist_t;

static void on_row(char **c, int n, void *ctxp) {
    load_ctx_hist_t *ctx = ctxp;
    historico_t *h;
    int duration_seconds;

    if (!historico_validar_sintatica(c, n)) return;
    if (!gestor_users_obter(ctx->users, c[1])) return;
    if (!gestor_musicas_obter(ctx->musicas, c[2])) return;

    duration_seconds = utils_duration_to_seconds(c[4]);
    if (duration_seconds < 0) return;

    h = historico_criar(c[0], c[1], c[2], c[3], duration_seconds, c[5]);
    if (!h) return;
    g_ptr_array_add(ctx->g->rows, h);
}

gestor_historico_t *gestor_historico_criar(void) {
    gestor_historico_t *g = g_new0(gestor_historico_t, 1);
    if (!g) return NULL;
    g->rows = g_ptr_array_new_with_free_func((GDestroyNotify)historico_destruir);
    return g;
}

void gestor_historico_destruir(gestor_historico_t *g) {
    if (!g) return;
    g_ptr_array_free(g->rows, TRUE);
    g_free(g);
}

void gestor_historico_carregar_com_validacao(gestor_historico_t *g, const char *csv,
                                             const gestor_users_t *users,
                                             const gestor_musicas_t *musicas) {
    load_ctx_hist_t ctx = { .g = g, .users = users, .musicas = musicas };
    parser_csv_stream(csv, ';', 1, on_row, &ctx);
}

unsigned int gestor_historico_total(const gestor_historico_t *g) {
    if (!g) return 0;
    return g->rows->len;
}

void gestor_historico_para_cada(const gestor_historico_t *g, gestor_historico_iter_cb cb, void *ctx) {
    guint i;
    if (!g || !cb) return;
    for (i = 0; i < g->rows->len; i++) {
        cb((const historico_t *)g_ptr_array_index(g->rows, i), ctx);
    }
}
