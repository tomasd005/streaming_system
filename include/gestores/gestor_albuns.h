#ifndef GESTOR_ALBUNS_H
#define GESTOR_ALBUNS_H

#include "entidades/albuns.h"
#include "gestores/gestor_artistas.h"

typedef struct gestor_albuns gestor_albuns_t;
typedef void (*gestor_albuns_iter_cb)(const album_t *a, void *ctx);

gestor_albuns_t *gestor_albuns_criar(void);
void gestor_albuns_destruir(gestor_albuns_t *g);
void gestor_albuns_carregar_com_validacao(gestor_albuns_t *g, const char *csv,
                                          const gestor_artistas_t *artistas);
const album_t *gestor_albuns_obter(const gestor_albuns_t *g, const char *id);
void gestor_albuns_para_cada(const gestor_albuns_t *g, gestor_albuns_iter_cb cb, void *ctx);

#endif
