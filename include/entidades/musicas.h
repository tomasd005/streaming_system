#ifndef MUSICAS_H
#define MUSICAS_H

#include <glib.h>

typedef struct musica musica_t;

musica_t *musica_criar(const char *id, const char *title, GPtrArray *artist_ids, const char *album_id,
                       const char *genre, const char *year, int duration_seconds);
void musica_destruir(musica_t *m);
const char *musica_id(const musica_t *m);
const char *musica_title(const musica_t *m);
const char *musica_genre(const musica_t *m);
const char *musica_album_id(const musica_t *m);
const GPtrArray *musica_artist_ids(const musica_t *m);
int musica_duration_seconds(const musica_t *m);

#endif
