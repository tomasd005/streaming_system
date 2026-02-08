#ifndef GESTOR_ARTISTAS_H
#define GESTOR_ARTISTAS_H

#include "entidades/artistas.h"

typedef struct gestor_artistas gestor_artistas_t;
typedef void (*gestor_artistas_iter_cb)(artista_t *a, void *ctx);

gestor_artistas_t *gestor_artistas_criar(void);
void gestor_artistas_destruir(gestor_artistas_t *g);
void gestor_artistas_carregar(gestor_artistas_t *g, const char *csv);
const artista_t *gestor_artistas_obter(const gestor_artistas_t *g, const char *id);
artista_t *gestor_artistas_obter_mut(gestor_artistas_t *g, const char *id);
void gestor_artistas_para_cada(gestor_artistas_t *g, gestor_artistas_iter_cb cb, void *ctx);
unsigned int gestor_artistas_total(const gestor_artistas_t *g);

#endif
