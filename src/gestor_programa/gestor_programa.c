#include "gestor_programa/gestor_programa.h"

#include "utils.h"

#include <glib.h>
#include <string.h>

typedef struct {
    gestor_programa_t *gp;
} agg_ctx_t;

typedef struct {
    artista_t *target;
    double extra;
} revenue_part_ctx_t;

static void reset_artist_cb(artista_t *a, void *ctx) {
    (void)ctx;
    artista_reset_estatisticas(a);
}

static void albums_individual_cb(const album_t *album, void *ctx) {
    agg_ctx_t *a = ctx;
    const GPtrArray *artists = album_artists_ids(album);
    guint i;
    for (i = 0; artists && i < artists->len; i++) {
        const char *artist_id = g_ptr_array_index((GPtrArray *)artists, i);
        artista_t *artist = gestor_artistas_obter_mut(a->gp->artistas, artist_id);
        if (artist && g_ascii_strcasecmp(artista_type(artist), "individual") == 0) {
            artista_set_num_albums_individual(artist, artista_num_albums_individual(artist) + 1);
        }
    }
}

static void discography_cb(const musica_t *m, void *ctx) {
    agg_ctx_t *a = ctx;
    const GPtrArray *artists = musica_artist_ids(m);
    guint i;
    for (i = 0; artists && i < artists->len; i++) {
        const char *artist_id = g_ptr_array_index((GPtrArray *)artists, i);
        artista_t *artist = gestor_artistas_obter_mut(a->gp->artistas, artist_id);
        if (artist) artista_incrementar_discografia(artist, musica_duration_seconds(m));
    }
}

static void reproductions_cb(const historico_t *h, void *ctx) {
    agg_ctx_t *a = ctx;
    const musica_t *m = gestor_musicas_obter(a->gp->musicas, historico_music_id(h));
    const GPtrArray *artists;
    guint i;
    if (!m) return;
    artists = musica_artist_ids(m);
    for (i = 0; artists && i < artists->len; i++) {
        const char *artist_id = g_ptr_array_index((GPtrArray *)artists, i);
        artista_t *artist = gestor_artistas_obter_mut(a->gp->artistas, artist_id);
        if (artist) artista_incrementar_reproducoes(artist, 1);
    }
}

static void group_participation_cb(artista_t *group, void *vctx) {
    revenue_part_ctx_t *lc = vctx;
    guint csz;
    if (g_ascii_strcasecmp(artista_type(group), "group") != 0) return;
    if (!artista_tem_constituinte(group, artista_id(lc->target))) return;
    csz = artista_constituents(group) ? artista_constituents(group)->len : 0;
    if (csz == 0) return;
    lc->extra += ((double)artista_reproducoes(group) * artista_recipe_per_stream(group)) / (double)csz;
}

static void compute_revenue_cb(artista_t *artist, void *ctx) {
    agg_ctx_t *a = ctx;
    double total = (double)artista_reproducoes(artist) * artista_recipe_per_stream(artist);

    if (g_ascii_strcasecmp(artista_type(artist), "individual") == 0) {
        revenue_part_ctx_t lctx = { .target = artist, .extra = 0.0 };
        gestor_artistas_para_cada(a->gp->artistas, group_participation_cb, &lctx);
        total += lctx.extra;
    }

    artista_set_total_recipe(artist, total);
}

static void gestor_programa_recalcular_agregados(gestor_programa_t *gp) {
    agg_ctx_t ctx = { .gp = gp };
    gestor_artistas_para_cada(gp->artistas, reset_artist_cb, NULL);
    gestor_albuns_para_cada(gp->albuns, albums_individual_cb, &ctx);
    gestor_musicas_para_cada(gp->musicas, discography_cb, &ctx);
    gestor_historico_para_cada(gp->historico, reproductions_cb, &ctx);
    gestor_artistas_para_cada(gp->artistas, compute_revenue_cb, &ctx);
}

gestor_programa_t *gestor_programa_criar(void) {
    gestor_programa_t *gp = g_new0(gestor_programa_t, 1);
    if (!gp) return NULL;

    gp->musicas = gestor_musicas_criar();
    gp->users = gestor_users_criar();
    gp->artistas = gestor_artistas_criar();
    gp->albuns = gestor_albuns_criar();
    gp->historico = gestor_historico_criar();

    if (!gp->musicas || !gp->users || !gp->artistas || !gp->albuns || !gp->historico) {
        gestor_programa_destruir(gp);
        return NULL;
    }
    return gp;
}

void gestor_programa_destruir(gestor_programa_t *gp) {
    if (!gp) return;
    gestor_musicas_destruir(gp->musicas);
    gestor_users_destruir(gp->users);
    gestor_artistas_destruir(gp->artistas);
    gestor_albuns_destruir(gp->albuns);
    gestor_historico_destruir(gp->historico);
    g_free(gp);
}

int gestor_programa_carregar_dataset(gestor_programa_t *gp, const char *pasta_dataset) {
    char path[512];
    if (!gp || !pasta_dataset) return -1;

    if (!utils_join_path(path, (int)sizeof(path), pasta_dataset, "artists.csv")) return -1;
    gestor_artistas_carregar(gp->artistas, path);

    if (!utils_join_path(path, (int)sizeof(path), pasta_dataset, "albums.csv")) return -1;
    gestor_albuns_carregar_com_validacao(gp->albuns, path, gp->artistas);

    if (!utils_join_path(path, (int)sizeof(path), pasta_dataset, "musics.csv")) return -1;
    gestor_musicas_carregar_com_validacao(gp->musicas, path, gp->artistas, gp->albuns);

    if (!utils_join_path(path, (int)sizeof(path), pasta_dataset, "users.csv")) return -1;
    gestor_users_carregar_com_validacao(gp->users, path, gp->musicas);

    if (!utils_join_path(path, (int)sizeof(path), pasta_dataset, "history.csv")) return -1;
    gestor_historico_carregar_com_validacao(gp->historico, path, gp->users, gp->musicas);

    gestor_programa_recalcular_agregados(gp);
    return 0;
}
