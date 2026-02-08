#ifndef GESTOR_HISTORICO_H
#define GESTOR_HISTORICO_H

#include "entidades/historico.h"
#include "gestores/gestor_musicas.h"
#include "gestores/gestor_users.h"

typedef struct gestor_historico gestor_historico_t;
typedef void (*gestor_historico_iter_cb)(const historico_t *h, void *ctx);

gestor_historico_t *gestor_historico_criar(void);
void gestor_historico_destruir(gestor_historico_t *g);
void gestor_historico_carregar_com_validacao(gestor_historico_t *g, const char *csv,
                                             const gestor_users_t *users,
                                             const gestor_musicas_t *musicas);
unsigned int gestor_historico_total(const gestor_historico_t *g);
void gestor_historico_para_cada(const gestor_historico_t *g, gestor_historico_iter_cb cb, void *ctx);
const GPtrArray *gestor_historico_obter_por_user(const gestor_historico_t *g, const char *user_id);
const GPtrArray *gestor_historico_obter_por_user_ano(const gestor_historico_t *g,
                                                     const char *user_id, int year);

#endif
