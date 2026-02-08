#include "gestores/gestor_musicas.h"

#include "parsers/parser.h"
#include "utils.h"
#include "validacoes/validacao_musicas.h"

#include <glib.h>

struct gestor_musicas { GHashTable *by_id; };
typedef struct {
    gestor_musicas_t *g;
    const gestor_artistas_t *artistas;
    const gestor_albuns_t *albuns;
} load_ctx_musicas_t;

static gboolean list_artists_valid(const GPtrArray *ids, const gestor_artistas_t *artistas) {
    guint i;
    if (!ids || ids->len == 0) return FALSE;
    for (i = 0; i < ids->len; i++) {
        const char *id = g_ptr_array_index((GPtrArray *)ids, i);
        if (!gestor_artistas_obter(artistas, id)) return FALSE;
    }
    return TRUE;
}

static void on_row(char **c, int n, void *ctxp) {
    load_ctx_musicas_t *ctx = ctxp;
    GPtrArray *artists;
    int duration_seconds;
    musica_t *m;

    if (!musica_validar_sintatica(c, n)) return;

    if (!gestor_albuns_obter(ctx->albuns, c[3])) return;

    artists = utils_parse_list_ids(c[2]);
    if (!artists) return;
    if (!list_artists_valid(artists, ctx->artistas)) {
        g_ptr_array_free(artists, TRUE);
        return;
    }

    duration_seconds = utils_duration_to_seconds(c[4]);
    if (duration_seconds < 0) {
        g_ptr_array_free(artists, TRUE);
        return;
    }

    m = musica_criar(c[0], c[1], artists, c[3], c[5], c[6], duration_seconds);
    if (!m) return;
    g_hash_table_insert(ctx->g->by_id, g_strdup(musica_id(m)), m);
}

gestor_musicas_t *gestor_musicas_criar(void) {
    gestor_musicas_t *g = g_new0(gestor_musicas_t, 1);
    if (!g) return NULL;
    g->by_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)musica_destruir);
    return g;
}

void gestor_musicas_destruir(gestor_musicas_t *g) {
    if (!g) return;
    g_hash_table_destroy(g->by_id);
    g_free(g);
}

void gestor_musicas_carregar_com_validacao(gestor_musicas_t *g, const char *csv,
                                           const gestor_artistas_t *artistas,
                                           const gestor_albuns_t *albuns) {
    load_ctx_musicas_t ctx = { .g = g, .artistas = artistas, .albuns = albuns };
    parser_csv_stream(csv, ';', 1, on_row, &ctx);
}

const musica_t *gestor_musicas_obter(const gestor_musicas_t *g, const char *id) {
    if (!g || !id) return NULL;
    return g_hash_table_lookup(g->by_id, id);
}

void gestor_musicas_para_cada(const gestor_musicas_t *g, gestor_musicas_iter_cb cb, void *ctx) {
    GHashTableIter it;
    gpointer key;
    gpointer value;
    if (!g || !cb) return;
    g_hash_table_iter_init(&it, g->by_id);
    while (g_hash_table_iter_next(&it, &key, &value)) {
        (void)key;
        cb((const musica_t *)value, ctx);
    }
}
