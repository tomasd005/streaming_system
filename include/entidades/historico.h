#ifndef HISTORICO_H
#define HISTORICO_H

typedef struct historico historico_t;

historico_t *historico_criar(const char *id, const char *user_id, const char *music_id,
                             const char *timestamp, int duration_seconds, const char *platform);
void historico_destruir(historico_t *h);
const char *historico_id(const historico_t *h);
const char *historico_user_id(const historico_t *h);
const char *historico_music_id(const historico_t *h);
const char *historico_timestamp(const historico_t *h);
int historico_duration_seconds(const historico_t *h);

#endif
