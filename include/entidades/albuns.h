#ifndef ALBUNS_H
#define ALBUNS_H

#include <glib.h>

typedef struct album album_t;

album_t *album_criar(const char *id, const char *title, const char *year, GPtrArray *artists_ids);
void album_destruir(album_t *a);
const char *album_id(const album_t *a);
const char *album_title(const album_t *a);
const GPtrArray *album_artists_ids(const album_t *a);

#endif
