#ifndef GESTOR_PROGRAMA_H
#define GESTOR_PROGRAMA_H

#include "gestores/gestor_albuns.h"
#include "gestores/gestor_artistas.h"
#include "gestores/gestor_historico.h"
#include "gestores/gestor_musicas.h"
#include "gestores/gestor_users.h"

/**
 * @file gestor_programa.h
 * @brief Contexto global da aplicacao.
 */

typedef struct gestor_programa {
    gestor_musicas_t *musicas;
    gestor_users_t *users;
    gestor_artistas_t *artistas;
    gestor_albuns_t *albuns;
    gestor_historico_t *historico;
} gestor_programa_t;

gestor_programa_t *gestor_programa_criar(void);
void gestor_programa_destruir(gestor_programa_t *gp);
int gestor_programa_carregar_dataset(gestor_programa_t *gp, const char *pasta_dataset);

#endif
