#include "gestores/gestor_users.h"

#include "parsers/parser.h"
#include "utils.h"
#include "validacoes/validacao_users.h"

#include <glib.h>

struct gestor_users { GHashTable *by_id; };
typedef struct {
    gestor_users_t *g;
    const gestor_musicas_t *musicas;
} load_ctx_users_t;

static gboolean liked_valid(const GPtrArray *ids, const gestor_musicas_t *musicas) {
    guint i;
    if (!ids) return FALSE;
    for (i = 0; i < ids->len; i++) {
        const char *id = g_ptr_array_index((GPtrArray *)ids, i);
        if (!gestor_musicas_obter(musicas, id)) return FALSE;
    }
    return TRUE;
}

static bool on_row(char **c, int n, const char *raw_line, void *ctxp) {
    load_ctx_users_t *ctx = ctxp;
    GPtrArray *liked;
    user_t *u;
    (void)raw_line;

    if (!user_validar_sintatica(c, n)) return false;

    liked = utils_parse_list_ids(c[7]);
    if (!liked) return false;
    if (!liked_valid(liked, ctx->musicas)) {
        g_ptr_array_free(liked, TRUE);
        return false;
    }

    u = user_criar(c[0], c[1], c[2], c[3], c[4], c[5], c[6], liked);
    if (!u) return false;

    if (g_hash_table_contains(ctx->g->by_id, user_username(u))) {
        user_destruir(u);
        return false;
    }

    g_hash_table_insert(ctx->g->by_id, g_strdup(user_username(u)), u);
    return true;
}

gestor_users_t *gestor_users_criar(void) {
    gestor_users_t *g = g_new0(gestor_users_t, 1);
    if (!g) return NULL;
    g->by_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)user_destruir);
    return g;
}

void gestor_users_destruir(gestor_users_t *g) {
    if (!g) return;
    g_hash_table_destroy(g->by_id);
    g_free(g);
}

void gestor_users_carregar_com_validacao(gestor_users_t *g, const char *csv,
                                         const gestor_musicas_t *musicas) {
    load_ctx_users_t ctx = { .g = g, .musicas = musicas };
    parser_csv_stream_with_errors(csv, ';', on_row, &ctx);
}

const user_t *gestor_users_obter(const gestor_users_t *g, const char *username) {
    if (!g || !username) return NULL;
    return g_hash_table_lookup(g->by_id, username);
}

void gestor_users_para_cada(const gestor_users_t *g, gestor_users_iter_cb cb, void *ctx) {
    GHashTableIter it;
    gpointer key;
    gpointer value;
    if (!g || !cb) return;
    g_hash_table_iter_init(&it, g->by_id);
    while (g_hash_table_iter_next(&it, &key, &value)) {
        (void)key;
        cb((const user_t *)value, ctx);
    }
}
