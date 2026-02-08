#include "entidades/musicas.h"

#include <glib.h>

struct musica {
    char *id;
    char *title;
    GPtrArray *artist_ids;
    char *album_id;
    char *genre;
    char *year;
    int duration_seconds;
};

musica_t *musica_criar(const char *id, const char *title, GPtrArray *artist_ids, const char *album_id,
                       const char *genre, const char *year, int duration_seconds) {
    musica_t *m = g_new0(musica_t, 1);
    if (!m) {
        if (artist_ids) g_ptr_array_free(artist_ids, TRUE);
        return NULL;
    }
    m->id = g_strdup(id ? id : "");
    m->title = g_strdup(title ? title : "");
    m->artist_ids = artist_ids ? artist_ids : g_ptr_array_new_with_free_func(g_free);
    m->album_id = g_strdup(album_id ? album_id : "");
    m->genre = g_strdup(genre ? genre : "");
    m->year = g_strdup(year ? year : "");
    m->duration_seconds = duration_seconds;
    return m;
}

void musica_destruir(musica_t *m) {
    if (!m) return;
    g_free(m->id);
    g_free(m->title);
    g_ptr_array_free(m->artist_ids, TRUE);
    g_free(m->album_id);
    g_free(m->genre);
    g_free(m->year);
    g_free(m);
}

const char *musica_id(const musica_t *m) { return m ? m->id : NULL; }
const char *musica_title(const musica_t *m) { return m ? m->title : NULL; }
const char *musica_genre(const musica_t *m) { return m ? m->genre : NULL; }
const char *musica_album_id(const musica_t *m) { return m ? m->album_id : NULL; }
const GPtrArray *musica_artist_ids(const musica_t *m) { return m ? m->artist_ids : NULL; }
int musica_duration_seconds(const musica_t *m) { return m ? m->duration_seconds : 0; }
