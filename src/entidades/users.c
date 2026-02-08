#include "entidades/users.h"

#include <glib.h>

struct user {
    char *username;
    char *email;
    char *first_name;
    char *last_name;
    char *birth_date;
    char *country;
    char *subscription_type;
    GPtrArray *liked_music_ids;
};

user_t *user_criar(const char *username, const char *email, const char *first_name,
                   const char *last_name, const char *birth_date, const char *country,
                   const char *subscription_type, GPtrArray *liked_music_ids) {
    user_t *u = g_new0(user_t, 1);
    if (!u) {
        if (liked_music_ids) g_ptr_array_free(liked_music_ids, TRUE);
        return NULL;
    }
    u->username = g_strdup(username ? username : "");
    u->email = g_strdup(email ? email : "");
    u->first_name = g_strdup(first_name ? first_name : "");
    u->last_name = g_strdup(last_name ? last_name : "");
    u->birth_date = g_strdup(birth_date ? birth_date : "");
    u->country = g_strdup(country ? country : "");
    u->subscription_type = g_strdup(subscription_type ? subscription_type : "");
    u->liked_music_ids = liked_music_ids ? liked_music_ids : g_ptr_array_new_with_free_func(g_free);
    return u;
}

void user_destruir(user_t *u) {
    if (!u) return;
    g_free(u->username);
    g_free(u->email);
    g_free(u->first_name);
    g_free(u->last_name);
    g_free(u->birth_date);
    g_free(u->country);
    g_free(u->subscription_type);
    g_ptr_array_free(u->liked_music_ids, TRUE);
    g_free(u);
}

const char *user_username(const user_t *u) { return u ? u->username : NULL; }
const char *user_email(const user_t *u) { return u ? u->email : NULL; }
const char *user_first_name(const user_t *u) { return u ? u->first_name : NULL; }
const char *user_last_name(const user_t *u) { return u ? u->last_name : NULL; }
const char *user_birth_date(const user_t *u) { return u ? u->birth_date : NULL; }
const char *user_country(const user_t *u) { return u ? u->country : NULL; }
const GPtrArray *user_liked_music_ids(const user_t *u) { return u ? u->liked_music_ids : NULL; }
