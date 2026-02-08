#include "entidades/albuns.h"

#include <glib.h>

struct album {
    char *id;
    char *title;
    char *year;
    GPtrArray *artists_ids;
};

album_t *album_criar(const char *id, const char *title, const char *year, GPtrArray *artists_ids) {
    album_t *a = g_new0(album_t, 1);
    if (!a) {
        if (artists_ids) g_ptr_array_free(artists_ids, TRUE);
        return NULL;
    }
    a->id = g_strdup(id ? id : "");
    a->title = g_strdup(title ? title : "");
    a->year = g_strdup(year ? year : "");
    a->artists_ids = artists_ids ? artists_ids : g_ptr_array_new_with_free_func(g_free);
    return a;
}

void album_destruir(album_t *a) {
    if (!a) return;
    g_free(a->id);
    g_free(a->title);
    g_free(a->year);
    g_ptr_array_free(a->artists_ids, TRUE);
    g_free(a);
}

const char *album_id(const album_t *a) { return a ? a->id : NULL; }
const char *album_title(const album_t *a) { return a ? a->title : NULL; }
const GPtrArray *album_artists_ids(const album_t *a) { return a ? a->artists_ids : NULL; }
