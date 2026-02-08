#include "entidades/artistas.h"

#include <glib.h>
#include <string.h>

struct artista {
    char *id;
    char *name;
    char *country;
    char *type;
    double recipe_per_stream;
    GPtrArray *constituents;

    int discografia_seconds;
    int reproducoes;
    int num_albums_individual;
    double total_recipe;
};

artista_t *artista_criar(const char *id, const char *name, const char *country, const char *type,
                         double recipe_per_stream, GPtrArray *constituents) {
    artista_t *a = g_new0(artista_t, 1);
    if (!a) {
        if (constituents) g_ptr_array_free(constituents, TRUE);
        return NULL;
    }
    a->id = g_strdup(id ? id : "");
    a->name = g_strdup(name ? name : "");
    a->country = g_strdup(country ? country : "");
    a->type = g_strdup(type ? type : "");
    a->recipe_per_stream = recipe_per_stream;
    a->constituents = constituents ? constituents : g_ptr_array_new_with_free_func(g_free);
    return a;
}

void artista_destruir(artista_t *a) {
    if (!a) return;
    g_free(a->id);
    g_free(a->name);
    g_free(a->country);
    g_free(a->type);
    g_ptr_array_free(a->constituents, TRUE);
    g_free(a);
}

const char *artista_id(const artista_t *a) { return a ? a->id : NULL; }
const char *artista_name(const artista_t *a) { return a ? a->name : NULL; }
const char *artista_country(const artista_t *a) { return a ? a->country : NULL; }
const char *artista_type(const artista_t *a) { return a ? a->type : NULL; }
double artista_recipe_per_stream(const artista_t *a) { return a ? a->recipe_per_stream : 0.0; }
const GPtrArray *artista_constituents(const artista_t *a) { return a ? a->constituents : NULL; }

bool artista_tem_constituinte(const artista_t *a, const char *artist_id) {
    guint i;
    if (!a || !artist_id) return false;
    for (i = 0; i < a->constituents->len; i++) {
        const char *id = g_ptr_array_index(a->constituents, i);
        if (id && strcmp(id, artist_id) == 0) return true;
    }
    return false;
}

void artista_reset_estatisticas(artista_t *a) {
    if (!a) return;
    a->discografia_seconds = 0;
    a->reproducoes = 0;
    a->num_albums_individual = 0;
    a->total_recipe = 0.0;
}

void artista_incrementar_discografia(artista_t *a, int seconds) {
    if (!a || seconds < 0) return;
    a->discografia_seconds += seconds;
}

void artista_incrementar_reproducoes(artista_t *a, int count) {
    if (!a || count < 0) return;
    a->reproducoes += count;
}

void artista_set_num_albums_individual(artista_t *a, int value) {
    if (!a) return;
    a->num_albums_individual = value;
}

void artista_set_total_recipe(artista_t *a, double value) {
    if (!a) return;
    a->total_recipe = value;
}

int artista_discografia_seconds(const artista_t *a) { return a ? a->discografia_seconds : 0; }
int artista_reproducoes(const artista_t *a) { return a ? a->reproducoes : 0; }
int artista_num_albums_individual(const artista_t *a) { return a ? a->num_albums_individual : 0; }
double artista_total_recipe(const artista_t *a) { return a ? a->total_recipe : 0.0; }
