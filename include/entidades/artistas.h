#ifndef ARTISTAS_H
#define ARTISTAS_H

#include <stdbool.h>
#include <glib.h>

typedef struct artista artista_t;

artista_t *artista_criar(const char *id, const char *name, const char *country, const char *type,
                         double recipe_per_stream, GPtrArray *constituents);
void artista_destruir(artista_t *a);

const char *artista_id(const artista_t *a);
const char *artista_name(const artista_t *a);
const char *artista_country(const artista_t *a);
const char *artista_type(const artista_t *a);
double artista_recipe_per_stream(const artista_t *a);
const GPtrArray *artista_constituents(const artista_t *a);
bool artista_tem_constituinte(const artista_t *a, const char *artist_id);

void artista_reset_estatisticas(artista_t *a);
void artista_incrementar_discografia(artista_t *a, int seconds);
void artista_incrementar_reproducoes(artista_t *a, int count);
void artista_set_num_albums_individual(artista_t *a, int value);
void artista_set_total_recipe(artista_t *a, double value);

int artista_discografia_seconds(const artista_t *a);
int artista_reproducoes(const artista_t *a);
int artista_num_albums_individual(const artista_t *a);
double artista_total_recipe(const artista_t *a);

#endif
