#include "queries/query6.h"

#include "gestor_programa/gestor_programa.h"
#include "utils.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *artist_id;
    int seconds;
    int distinct_musics;
} q6_artist_row_t;

static void q6_artist_row_destroy(gpointer p) {
    q6_artist_row_t *r = p;
    if (!r) return;
    g_free(r->artist_id);
    g_free(r);
}

static int use_alt_separator(const char *cmd) {
    char *copy;
    char *tok;
    int alt = 0;
    copy = strdup(cmd ? cmd : "");
    if (!copy) return 0;
    tok = strtok(copy, " \t\r\n");
    if (tok && strchr(tok, 'S')) alt = 1;
    free(copy);
    return alt;
}

static void incr_map_int(GHashTable *h, const char *key, int delta) {
    int cur = (int)GPOINTER_TO_INT(g_hash_table_lookup(h, key));
    cur += delta;
    g_hash_table_insert(h, g_strdup(key), GINT_TO_POINTER(cur));
}

static void add_artist_music_seen(GHashTable *seen, GHashTable *artist_distinct,
                                  const char *artist_id, const char *music_id) {
    char pair[512];
    snprintf(pair, sizeof(pair), "%s|%s", artist_id, music_id);
    if (!g_hash_table_contains(seen, pair)) {
        g_hash_table_insert(seen, g_strdup(pair), GINT_TO_POINTER(1));
        incr_map_int(artist_distinct, artist_id, 1);
    }
}

static void choose_best_key(GHashTable *h, int tie_mode, const char **best_key, int *best_val) {
    GHashTableIter it;
    gpointer k, v;
    *best_key = NULL;
    *best_val = -1;

    g_hash_table_iter_init(&it, h);
    while (g_hash_table_iter_next(&it, &k, &v)) {
        const char *key = k;
        int val = (int)GPOINTER_TO_INT(v);
        if (val > *best_val) {
            *best_val = val;
            *best_key = key;
        } else if (val == *best_val && *best_key) {
            if (tie_mode == 0) {
                if (strcmp(key, *best_key) < 0) *best_key = key; /* lex asc */
            } else if (tie_mode == 1) {
                if (strcmp(key, *best_key) > 0) *best_key = key; /* date mais recente */
            }
        }
    }
}

static gint cmp_artist_rows(gconstpointer ap, gconstpointer bp) {
    const q6_artist_row_t *a = ap;
    const q6_artist_row_t *b = bp;
    if (a->seconds != b->seconds) return b->seconds - a->seconds;
    return strcmp(a->artist_id, b->artist_id);
}

void query6_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    char *copy;
    char *tok;
    char *user_id;
    char *year_tok;
    char *n_tok;
    int year;
    int top_n = 0;
    const GPtrArray *rows;
    GHashTable *distinct_musics;
    GHashTable *artist_secs;
    GHashTable *artist_distinct;
    GHashTable *artist_music_seen;
    GHashTable *day_counts;
    GHashTable *genre_secs;
    GHashTable *album_secs;
    int hour_secs[24] = {0};
    int total_seconds = 0;
    guint i;
    const char *best_artist_id;
    int best_artist_secs;
    const char *best_day;
    int best_day_count;
    const char *best_genre;
    int best_genre_secs;
    const char *best_album;
    int best_album_secs;
    int best_hour = 0;
    const char *sep;
    char total_time[32];

    if (!gp || !args || !out) return;

    copy = strdup(args);
    if (!copy) {
        fprintf(out, "\n");
        return;
    }

    tok = strtok(copy, " \t\r\n");
    user_id = strtok(NULL, " \t\r\n");
    year_tok = strtok(NULL, " \t\r\n");
    n_tok = strtok(NULL, " \t\r\n");
    (void)tok;

    if (!user_id || !year_tok) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    year = atoi(year_tok);
    if (n_tok) top_n = atoi(n_tok);
    if (top_n < 0) top_n = 0;

    rows = gestor_historico_obter_por_user_ano(gp->historico, user_id, year);
    if (!rows || rows->len == 0) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    distinct_musics = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    artist_secs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    artist_distinct = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    artist_music_seen = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    day_counts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    genre_secs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    album_secs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for (i = 0; i < rows->len; i++) {
        const historico_t *h = g_ptr_array_index((GPtrArray *)rows, i);
        const musica_t *m = gestor_musicas_obter(gp->musicas, historico_music_id(h));
        const GPtrArray *artists;
        int dur;
        int y, mo, d, hh, mi, ss;
        char day_key[16];
        const char *album_key;
        guint j;

        if (!m) continue;

        dur = historico_duration_seconds(h);
        total_seconds += dur;

        g_hash_table_insert(distinct_musics, g_strdup(historico_music_id(h)), GINT_TO_POINTER(1));

        if (utils_parse_datetime(historico_timestamp(h), &y, &mo, &d, &hh, &mi, &ss)) {
            snprintf(day_key, sizeof(day_key), "%04d/%02d/%02d", y, mo, d);
            incr_map_int(day_counts, day_key, 1);
            if (hh >= 0 && hh < 24) hour_secs[hh] += dur;
        }

        incr_map_int(genre_secs, musica_genre(m), dur);

        {
            const album_t *al = gestor_albuns_obter(gp->albuns, musica_album_id(m));
            album_key = al ? album_title(al) : musica_album_id(m);
            incr_map_int(album_secs, album_key, dur);
        }

        artists = musica_artist_ids(m);
        for (j = 0; artists && j < artists->len; j++) {
            const char *artist_id = g_ptr_array_index((GPtrArray *)artists, j);
            incr_map_int(artist_secs, artist_id, dur);
            add_artist_music_seen(artist_music_seen, artist_distinct, artist_id, musica_id(m));
        }
    }

    choose_best_key(artist_secs, 0, &best_artist_id, &best_artist_secs);
    choose_best_key(day_counts, 1, &best_day, &best_day_count);
    choose_best_key(genre_secs, 0, &best_genre, &best_genre_secs);
    choose_best_key(album_secs, 0, &best_album, &best_album_secs);

    for (i = 1; i < 24; i++) {
        if (hour_secs[i] > hour_secs[best_hour]) best_hour = (int)i;
    }

    sep = use_alt_separator(args) ? "=" : ";";
    utils_seconds_to_hhmmss(total_seconds, total_time, sizeof(total_time));

    {
        const artista_t *best_artist = best_artist_id ? gestor_artistas_obter(gp->artistas, best_artist_id) : NULL;
        const char *artist_name = best_artist ? artista_name(best_artist) : "";
        fprintf(out, "%s%s%u%s%s%s%s%s%s%s%s%s%02d\n",
                total_time, sep,
                (unsigned)g_hash_table_size(distinct_musics), sep,
                artist_name, sep,
                best_day ? best_day : "", sep,
                best_genre ? best_genre : "", sep,
                best_album ? best_album : "", sep,
                best_hour);
    }

    if (top_n > 0) {
        GPtrArray *arr = g_ptr_array_new_with_free_func(q6_artist_row_destroy);
        GHashTableIter it;
        gpointer k, v;

        g_hash_table_iter_init(&it, artist_secs);
        while (g_hash_table_iter_next(&it, &k, &v)) {
            q6_artist_row_t *r = g_new0(q6_artist_row_t, 1);
            if (!r) continue;
            r->artist_id = g_strdup((const char *)k);
            r->seconds = (int)GPOINTER_TO_INT(v);
            r->distinct_musics = (int)GPOINTER_TO_INT(g_hash_table_lookup(artist_distinct, k));
            g_ptr_array_add(arr, r);
        }

        g_ptr_array_sort(arr, cmp_artist_rows);

        for (i = 0; i < arr->len && (int)i < top_n; i++) {
            q6_artist_row_t *r = g_ptr_array_index(arr, i);
            const artista_t *a = gestor_artistas_obter(gp->artistas, r->artist_id);
            const char *name = a ? artista_name(a) : r->artist_id;
            char tbuf[32];
            utils_seconds_to_hhmmss(r->seconds, tbuf, sizeof(tbuf));
            fprintf(out, "%s%s%d%s%s\n", name, sep, r->distinct_musics, sep, tbuf);
        }

        g_ptr_array_free(arr, TRUE);
    }

    g_hash_table_destroy(distinct_musics);
    g_hash_table_destroy(artist_secs);
    g_hash_table_destroy(artist_distinct);
    g_hash_table_destroy(artist_music_seen);
    g_hash_table_destroy(day_counts);
    g_hash_table_destroy(genre_secs);
    g_hash_table_destroy(album_secs);
    free(copy);
}
