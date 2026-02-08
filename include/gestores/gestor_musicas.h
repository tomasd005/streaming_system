#ifndef GESTOR_MUSICAS_H
#define GESTOR_MUSICAS_H

#include "entidades/musicas.h"
#include "gestores/gestor_albuns.h"
#include "gestores/gestor_artistas.h"

typedef struct gestor_musicas gestor_musicas_t;
typedef void (*gestor_musicas_iter_cb)(const musica_t *m, void *ctx);

gestor_musicas_t *gestor_musicas_criar(void);
void gestor_musicas_destruir(gestor_musicas_t *g);
void gestor_musicas_carregar_com_validacao(gestor_musicas_t *g, const char *csv,
                                           const gestor_artistas_t *artistas,
                                           const gestor_albuns_t *albuns);
const musica_t *gestor_musicas_obter(const gestor_musicas_t *g, const char *id);
void gestor_musicas_para_cada(const gestor_musicas_t *g, gestor_musicas_iter_cb cb, void *ctx);

#endif
