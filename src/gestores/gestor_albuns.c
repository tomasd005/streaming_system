#include "gestores/gestor_albuns.h"

#include "parsers/parser.h"
#include "utils.h"
#include "validacoes/validacao_albuns.h"

#include <glib.h>

struct gestor_albuns { GHashTable *by_id; };
typedef struct {
    gestor_albuns_t *g;
    const gestor_artistas_t *artistas;
} load_ctx_albuns_t;

static gboolean list_artists_valid(const GPtrArray *ids, const gestor_artistas_t *artistas) {
    guint i;
    if (!ids) return FALSE;
    for (i = 0; i < ids->len; i++) {
        const char *id = g_ptr_array_index((GPtrArray *)ids, i);
        if (!gestor_artistas_obter(artistas, id)) return FALSE;
    }
    return TRUE;
}

static bool on_row(char **c, int n, const char *raw_line, void *ctxp) {
    load_ctx_albuns_t *ctx = ctxp;
    GPtrArray *artists;
    album_t *a;
    (void)raw_line;

    if (!album_validar_sintatica(c, n)) return false;
    artists = utils_parse_list_ids(c[2]);
    if (!artists) return false;
    if (!list_artists_valid(artists, ctx->artistas)) {
        g_ptr_array_free(artists, TRUE);
        return false;
    }

    a = album_criar(c[0], c[1], c[3], artists);
    if (!a) return false;

    if (g_hash_table_contains(ctx->g->by_id, album_id(a))) {
        album_destruir(a);
        return false;
    }

    g_hash_table_insert(ctx->g->by_id, g_strdup(album_id(a)), a);
    return true;
}

gestor_albuns_t *gestor_albuns_criar(void) {
    gestor_albuns_t *g = g_new0(gestor_albuns_t, 1);
    if (!g) return NULL;
    g->by_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)album_destruir);
    return g;
}

void gestor_albuns_destruir(gestor_albuns_t *g) {
    if (!g) return;
    g_hash_table_destroy(g->by_id);
    g_free(g);
}

void gestor_albuns_carregar_com_validacao(gestor_albuns_t *g, const char *csv,
                                          const gestor_artistas_t *artistas) {
    load_ctx_albuns_t ctx = { .g = g, .artistas = artistas };
    parser_csv_stream_with_errors(csv, ';', on_row, &ctx);
}

const album_t *gestor_albuns_obter(const gestor_albuns_t *g, const char *id) {
    if (!g || !id) return NULL;
    return g_hash_table_lookup(g->by_id, id);
}

void gestor_albuns_para_cada(const gestor_albuns_t *g, gestor_albuns_iter_cb cb, void *ctx) {
    GHashTableIter it;
    gpointer key;
    gpointer value;
    if (!g || !cb) return;
    g_hash_table_iter_init(&it, g->by_id);
    while (g_hash_table_iter_next(&it, &key, &value)) {
        (void)key;
        cb((const album_t *)value, ctx);
    }
}
