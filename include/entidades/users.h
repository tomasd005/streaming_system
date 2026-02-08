#ifndef USERS_H
#define USERS_H

#include <glib.h>

typedef struct user user_t;

user_t *user_criar(const char *username, const char *email, const char *first_name,
                   const char *last_name, const char *birth_date, const char *country,
                   const char *subscription_type, GPtrArray *liked_music_ids);
void user_destruir(user_t *u);
const char *user_username(const user_t *u);
const char *user_email(const user_t *u);
const char *user_first_name(const user_t *u);
const char *user_last_name(const user_t *u);
const char *user_birth_date(const user_t *u);
const char *user_country(const user_t *u);
const GPtrArray *user_liked_music_ids(const user_t *u);

#endif
