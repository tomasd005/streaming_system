#include "queries/query4.h"

#include "gestor_programa/gestor_programa.h"
#include "utils.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *artist_id;
    int seconds;
} q4_pair_t;

static void q4_pair_destroy(gpointer p) {
    q4_pair_t *x = p;
    if (!x) return;
    g_free(x->artist_id);
    g_free(x);
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

static void incr_int_hash(GHashTable *h, const char *key, int delta) {
    int cur;
    gpointer val;
    val = g_hash_table_lookup(h, key);
    cur = (int)GPOINTER_TO_INT(val);
    cur += delta;
    g_hash_table_insert(h, g_strdup(key), GINT_TO_POINTER(cur));
}

static void incr_week_artist(GHashTable *weeks, int week_start, const char *artist_id, int secs) {
    GHashTable *artist_map = g_hash_table_lookup(weeks, GINT_TO_POINTER(week_start));
    if (!artist_map) {
        artist_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_hash_table_insert(weeks, GINT_TO_POINTER(week_start), artist_map);
    }
    incr_int_hash(artist_map, artist_id, secs);
}

static gint cmp_pairs(gconstpointer a, gconstpointer b) {
    const q4_pair_t *pa = a;
    const q4_pair_t *pb = b;
    if (pa->seconds != pb->seconds) return pb->seconds - pa->seconds;
    return strcmp(pa->artist_id, pb->artist_id);
}

static void hash_to_pairs_cb(gpointer key, gpointer value, gpointer user_data) {
    GPtrArray *arr = user_data;
    q4_pair_t *p = g_new0(q4_pair_t, 1);
    if (!p) return;
    p->artist_id = g_strdup((const char *)key);
    p->seconds = (int)GPOINTER_TO_INT(value);
    g_ptr_array_add(arr, p);
}

static void count_top10_for_week(GHashTable *artist_map, GHashTable *counts) {
    GPtrArray *pairs = g_ptr_array_new_with_free_func(q4_pair_destroy);
    guint i, lim;
    g_hash_table_foreach(artist_map, hash_to_pairs_cb, pairs);
    g_ptr_array_sort(pairs, cmp_pairs);
    lim = pairs->len < 10 ? pairs->len : 10;
    for (i = 0; i < lim; i++) {
        q4_pair_t *p = g_ptr_array_index(pairs, i);
        incr_int_hash(counts, p->artist_id, 1);
    }
    g_ptr_array_free(pairs, TRUE);
}

static bool parse_optional_range(const char *args, int *has_range, int *begin_day, int *end_day) {
    char *copy = strdup(args ? args : "");
    char *tok;
    char *d1;
    char *d2;
    int y, m, d;
    if (!copy) return false;

    tok = strtok(copy, " \t\r\n");
    d1 = strtok(NULL, " \t\r\n");
    d2 = strtok(NULL, " \t\r\n");
    (void)tok;

    if (!d1 || !d2) {
        *has_range = 0;
        free(copy);
        return true;
    }

    if (!utils_parse_date_ymd(d1, &y, &m, &d)) {
        free(copy);
        return false;
    }
    *begin_day = utils_days_from_civil(y, m, d);

    if (!utils_parse_date_ymd(d2, &y, &m, &d)) {
        free(copy);
        return false;
    }
    *end_day = utils_days_from_civil(y, m, d);

    if (*begin_day > *end_day) {
        free(copy);
        return false;
    }

    *has_range = 1;
    free(copy);
    return true;
}

typedef struct {
    const gestor_programa_t *gp;
    GHashTable *weeks;
} q4_build_ctx_t;

static void q4_build_week_cb(const historico_t *h, void *ctxp) {
    q4_build_ctx_t *b = ctxp;
    const musica_t *m;
    const GPtrArray *artists;
    int y, mo, d, hh, mi, ss;
    int day, weekday, week_start;
    int dur;
    guint i;

    if (!utils_parse_datetime(historico_timestamp(h), &y, &mo, &d, &hh, &mi, &ss)) return;
    day = utils_days_from_civil(y, mo, d);
    weekday = utils_weekday_sun0(day);
    week_start = day - weekday;

    m = gestor_musicas_obter(b->gp->musicas, historico_music_id(h));
    if (!m) return;
    artists = musica_artist_ids(m);
    dur = historico_duration_seconds(h);

    for (i = 0; artists && i < artists->len; i++) {
        const char *artist_id = g_ptr_array_index((GPtrArray *)artists, i);
        incr_week_artist(b->weeks, week_start, artist_id, dur);
    }
}

static void build_week_maps(const gestor_programa_t *gp, GHashTable *weeks) {
    q4_build_ctx_t b = { .gp = gp, .weeks = weeks };
    gestor_historico_para_cada(gp->historico, q4_build_week_cb, &b);
}

void query4_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    GHashTable *weeks;
    GHashTable *counts;
    GHashTableIter it;
    gpointer k, v;
    int has_range, begin_day, end_day;
    const char *best_artist_id = NULL;
    int best_count = -1;
    const char *sep;

    if (!gp || !args || !out) return;

    has_range = 0;
    begin_day = 0;
    end_day = 0;
    if (!parse_optional_range(args, &has_range, &begin_day, &end_day)) {
        fprintf(out, "\n");
        return;
    }

    weeks = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)g_hash_table_destroy);
    counts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    build_week_maps(gp, weeks);

    g_hash_table_iter_init(&it, weeks);
    while (g_hash_table_iter_next(&it, &k, &v)) {
        int week_start = GPOINTER_TO_INT(k);
        int week_end = week_start + 6;
        GHashTable *artist_map = v;

        if (has_range) {
            if (week_start > end_day || week_end < begin_day) continue;
        }
        count_top10_for_week(artist_map, counts);
    }

    g_hash_table_iter_init(&it, counts);
    while (g_hash_table_iter_next(&it, &k, &v)) {
        const char *artist_id = k;
        int c = (int)GPOINTER_TO_INT(v);
        if (c > best_count || (c == best_count && best_artist_id && strcmp(artist_id, best_artist_id) < 0) ||
            (c == best_count && !best_artist_id)) {
            best_count = c;
            best_artist_id = artist_id;
        }
    }

    if (!best_artist_id) {
        fprintf(out, "\n");
        g_hash_table_destroy(weeks);
        g_hash_table_destroy(counts);
        return;
    }

    {
        const artista_t *a = gestor_artistas_obter(gp->artistas, best_artist_id);
        if (!a) {
            fprintf(out, "\n");
        } else {
            sep = use_alt_separator(args) ? "=" : ";";
            fprintf(out, "%s%s%s%s%d\n", artista_name(a), sep, artista_type(a), sep, best_count);
        }
    }

    g_hash_table_destroy(weeks);
    g_hash_table_destroy(counts);
}
