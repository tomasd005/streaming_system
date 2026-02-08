#include "entidades/historico.h"

#include <glib.h>

struct historico {
    char *id;
    char *user_id;
    char *music_id;
    char *timestamp;
    int duration_seconds;
    char *platform;
};

historico_t *historico_criar(const char *id, const char *user_id, const char *music_id,
                             const char *timestamp, int duration_seconds, const char *platform) {
    historico_t *h = g_new0(historico_t, 1);
    if (!h) return NULL;
    h->id = g_strdup(id ? id : "");
    h->user_id = g_strdup(user_id ? user_id : "");
    h->music_id = g_strdup(music_id ? music_id : "");
    h->timestamp = g_strdup(timestamp ? timestamp : "");
    h->duration_seconds = duration_seconds;
    h->platform = g_strdup(platform ? platform : "");
    return h;
}

void historico_destruir(historico_t *h) {
    if (!h) return;
    g_free(h->id);
    g_free(h->user_id);
    g_free(h->music_id);
    g_free(h->timestamp);
    g_free(h->platform);
    g_free(h);
}

const char *historico_id(const historico_t *h) { return h ? h->id : NULL; }
const char *historico_user_id(const historico_t *h) { return h ? h->user_id : NULL; }
const char *historico_music_id(const historico_t *h) { return h ? h->music_id : NULL; }
const char *historico_timestamp(const historico_t *h) { return h ? h->timestamp : NULL; }
int historico_duration_seconds(const historico_t *h) { return h ? h->duration_seconds : 0; }
