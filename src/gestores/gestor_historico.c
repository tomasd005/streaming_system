#include "gestores/gestor_historico.h"

#include "parsers/parser.h"
#include "utils.h"
#include "validacoes/validacao_historico.h"

#include <glib.h>
#include <stdio.h>

struct gestor_historico {
    GPtrArray *rows;
    GHashTable *by_user;
    GHashTable *by_user_year;
};

typedef struct {
    gestor_historico_t *g;
    const gestor_users_t *users;
    const gestor_musicas_t *musicas;
} load_ctx_hist_t;

static GPtrArray *ensure_index_array(GHashTable *h, const char *key) {
    GPtrArray *arr = g_hash_table_lookup(h, key);
    if (!arr) {
        arr = g_ptr_array_new();
        g_hash_table_insert(h, g_strdup(key), arr);
    }
    return arr;
}

static void add_index(gestor_historico_t *g, const historico_t *h) {
    char key_year[256];
    int year = 0;
    GPtrArray *arr_user;
    GPtrArray *arr_year;
    const char *uid = historico_user_id(h);

    arr_user = ensure_index_array(g->by_user, uid);
    g_ptr_array_add(arr_user, (gpointer)h);

    sscanf(historico_timestamp(h), "%4d/", &year);
    snprintf(key_year, sizeof(key_year), "%s#%04d", uid, year);
    arr_year = ensure_index_array(g->by_user_year, key_year);
    g_ptr_array_add(arr_year, (gpointer)h);
}

static bool on_row(char **c, int n, const char *raw_line, void *ctxp) {
    load_ctx_hist_t *ctx = ctxp;
    historico_t *h;
    int duration_seconds;
    (void)raw_line;

    if (!historico_validar_sintatica(c, n)) return false;
    if (!gestor_users_obter(ctx->users, c[1])) return false;
    if (!gestor_musicas_obter(ctx->musicas, c[2])) return false;

    duration_seconds = utils_duration_to_seconds(c[4]);
    if (duration_seconds < 0) return false;

    h = historico_criar(c[0], c[1], c[2], c[3], duration_seconds, c[5]);
    if (!h) return false;
    g_ptr_array_add(ctx->g->rows, h);
    add_index(ctx->g, h);
    return true;
}

gestor_historico_t *gestor_historico_criar(void) {
    gestor_historico_t *g = g_new0(gestor_historico_t, 1);
    if (!g) return NULL;
    g->rows = g_ptr_array_new_with_free_func((GDestroyNotify)historico_destruir);
    g->by_user = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_ptr_array_unref);
    g->by_user_year = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_ptr_array_unref);
    return g;
}

void gestor_historico_destruir(gestor_historico_t *g) {
    if (!g) return;
    g_ptr_array_free(g->rows, TRUE);
    g_hash_table_destroy(g->by_user);
    g_hash_table_destroy(g->by_user_year);
    g_free(g);
}

void gestor_historico_carregar_com_validacao(gestor_historico_t *g, const char *csv,
                                             const gestor_users_t *users,
                                             const gestor_musicas_t *musicas) {
    load_ctx_hist_t ctx = { .g = g, .users = users, .musicas = musicas };
    parser_csv_stream_with_errors(csv, ';', on_row, &ctx);
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

const GPtrArray *gestor_historico_obter_por_user(const gestor_historico_t *g, const char *user_id) {
    if (!g || !user_id) return NULL;
    return g_hash_table_lookup(g->by_user, user_id);
}

const GPtrArray *gestor_historico_obter_por_user_ano(const gestor_historico_t *g,
                                                     const char *user_id, int year) {
    char key[256];
    if (!g || !user_id) return NULL;
    snprintf(key, sizeof(key), "%s#%04d", user_id, year);
    return g_hash_table_lookup(g->by_user_year, key);
}
