#include "gestores/gestor_artistas.h"

#include "parsers/parser.h"
#include "utils.h"
#include "validacoes/validacao_artistas.h"

#include <glib.h>
#include <stdlib.h>

struct gestor_artistas { GHashTable *by_id; };
typedef struct { gestor_artistas_t *g; } load_ctx_artistas_t;

static void on_row(char **c, int n, void *ctxp) {
    load_ctx_artistas_t *ctx = ctxp;
    GPtrArray *consts;
    double recipe;
    artista_t *a;
    char *type_norm;

    if (!artista_validar_sintatica(c, n)) return;

    consts = utils_parse_list_ids(c[4]);
    if (!consts) return;

    type_norm = g_ascii_strdown(c[6], -1);
    if (!type_norm) {
        g_ptr_array_free(consts, TRUE);
        return;
    }

    recipe = g_ascii_strtod(c[3], NULL);
    a = artista_criar(c[0], c[1], c[5], type_norm, recipe, consts);
    g_free(type_norm);
    if (!a) return;
    g_hash_table_insert(ctx->g->by_id, g_strdup(artista_id(a)), a);
}

gestor_artistas_t *gestor_artistas_criar(void) {
    gestor_artistas_t *g = g_new0(gestor_artistas_t, 1);
    if (!g) return NULL;
    g->by_id = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)artista_destruir);
    return g;
}

void gestor_artistas_destruir(gestor_artistas_t *g) {
    if (!g) return;
    g_hash_table_destroy(g->by_id);
    g_free(g);
}

void gestor_artistas_carregar(gestor_artistas_t *g, const char *csv) {
    load_ctx_artistas_t ctx = { .g = g };
    parser_csv_stream(csv, ';', 1, on_row, &ctx);
}

const artista_t *gestor_artistas_obter(const gestor_artistas_t *g, const char *id) {
    if (!g || !id) return NULL;
    return g_hash_table_lookup(g->by_id, id);
}

artista_t *gestor_artistas_obter_mut(gestor_artistas_t *g, const char *id) {
    if (!g || !id) return NULL;
    return g_hash_table_lookup(g->by_id, id);
}

void gestor_artistas_para_cada(gestor_artistas_t *g, gestor_artistas_iter_cb cb, void *ctx) {
    GHashTableIter it;
    gpointer key;
    gpointer value;
    if (!g || !cb) return;
    g_hash_table_iter_init(&it, g->by_id);
    while (g_hash_table_iter_next(&it, &key, &value)) {
        (void)key;
        cb((artista_t *)value, ctx);
    }
}

unsigned int gestor_artistas_total(const gestor_artistas_t *g) {
    if (!g) return 0;
    return (unsigned int)g_hash_table_size(g->by_id);
}
