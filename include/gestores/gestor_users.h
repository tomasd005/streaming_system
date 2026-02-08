#ifndef GESTOR_USERS_H
#define GESTOR_USERS_H

#include "entidades/users.h"
#include "gestores/gestor_musicas.h"

typedef struct gestor_users gestor_users_t;
typedef void (*gestor_users_iter_cb)(const user_t *u, void *ctx);

gestor_users_t *gestor_users_criar(void);
void gestor_users_destruir(gestor_users_t *g);
void gestor_users_carregar_com_validacao(gestor_users_t *g, const char *csv,
                                         const gestor_musicas_t *musicas);
const user_t *gestor_users_obter(const gestor_users_t *g, const char *username);
void gestor_users_para_cada(const gestor_users_t *g, gestor_users_iter_cb cb, void *ctx);

#endif
