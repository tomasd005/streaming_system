#include "queries/query5.h"

#include "gestor_programa/gestor_programa.h"

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *username;
    double dist;
} q5_item_t;

typedef struct {
    const gestor_programa_t *gp;
    GHashTable *user_genre_counts; /* username -> (genre -> int count) */
} q5_cache_t;

static q5_cache_t g_q5_cache = {0};

typedef struct {
    const gestor_programa_t *gp;
    GHashTable *outer;
} q5_build_ctx_t;

static void q5_item_destroy(gpointer p) {
    q5_item_t *x = p;
    if (!x) return;
    g_free(x->username);
    g_free(x);
}

static void q5_destroy_nested_map(gpointer p) {
    GHashTable *h = p;
    if (h) g_hash_table_destroy(h);
}

static GHashTable *q5_ensure_user_map(GHashTable *m, const char *username) {
    GHashTable *h = g_hash_table_lookup(m, username);
    if (!h) {
        h = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_hash_table_insert(m, g_strdup(username), h);
    }
    return h;
}

static void q5_incr_genre(GHashTable *genre_map, const char *genre, int delta) {
    int cur = (int)GPOINTER_TO_INT(g_hash_table_lookup(genre_map, genre));
    cur += delta;
    g_hash_table_insert(genre_map, g_strdup(genre), GINT_TO_POINTER(cur));
}

static void q5_build_user_cb(const user_t *u, void *ctxp) {
    q5_build_ctx_t *b = ctxp;
    q5_ensure_user_map(b->outer, user_username(u));
}

static void q5_build_hist_cb(const historico_t *h, void *ctxp) {
    q5_build_ctx_t *b = ctxp;
    const musica_t *m = gestor_musicas_obter(b->gp->musicas, historico_music_id(h));
    GHashTable *user_map;
    if (!m) return;
    user_map = q5_ensure_user_map(b->outer, historico_user_id(h));
    q5_incr_genre(user_map, musica_genre(m), 1);
}

static void q5_build_cache(const gestor_programa_t *gp) {
    if (g_q5_cache.user_genre_counts) {
        g_hash_table_destroy(g_q5_cache.user_genre_counts);
        g_q5_cache.user_genre_counts = NULL;
    }

    g_q5_cache.user_genre_counts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                                          q5_destroy_nested_map);
    g_q5_cache.gp = gp;
    {
        q5_build_ctx_t b = { .gp = gp, .outer = g_q5_cache.user_genre_counts };
        gestor_users_para_cada(gp->users, q5_build_user_cb, &b);
        gestor_historico_para_cada(gp->historico, q5_build_hist_cb, &b);
    }
}

static GHashTable *q5_get_user_map(const char *username) {
    if (!g_q5_cache.user_genre_counts) return NULL;
    return g_hash_table_lookup(g_q5_cache.user_genre_counts, username);
}

static double q5_distance(GHashTable *a, GHashTable *b) {
    GHashTableIter it;
    gpointer k, v;
    double sum = 0.0;

    if (!a || !b) return 1e18;

    g_hash_table_iter_init(&it, a);
    while (g_hash_table_iter_next(&it, &k, &v)) {
        const char *genre = k;
        int va = (int)GPOINTER_TO_INT(v);
        int vb = (int)GPOINTER_TO_INT(g_hash_table_lookup(b, genre));
        double d = (double)(va - vb);
        sum += d * d;
    }

    g_hash_table_iter_init(&it, b);
    while (g_hash_table_iter_next(&it, &k, &v)) {
        const char *genre = k;
        if (!g_hash_table_contains(a, genre)) {
            int vb = (int)GPOINTER_TO_INT(v);
            double d = (double)vb;
            sum += d * d;
        }
    }

    return sqrt(sum);
}

static gint q5_cmp_items(gconstpointer ap, gconstpointer bp) {
    const q5_item_t *a = ap;
    const q5_item_t *b = bp;
    if (a->dist < b->dist) return -1;
    if (a->dist > b->dist) return 1;
    return strcmp(a->username, b->username);
}

void query5_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    char *copy;
    char *tok;
    char *target;
    char *n_tok;
    int n;
    GHashTable *target_map;
    GPtrArray *items;
    guint i;

    if (!gp || !args || !out) return;

    if (g_q5_cache.gp != gp || !g_q5_cache.user_genre_counts) {
        q5_build_cache(gp);
    }

    copy = strdup(args);
    if (!copy) {
        fprintf(out, "\n");
        return;
    }

    tok = strtok(copy, " \t\r\n");
    target = strtok(NULL, " \t\r\n");
    n_tok = strtok(NULL, " \t\r\n");
    (void)tok;

    if (!target || !n_tok) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    n = atoi(n_tok);
    if (n <= 0 || !gestor_users_obter(gp->users, target)) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    target_map = q5_get_user_map(target);
    if (!target_map) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    items = g_ptr_array_new_with_free_func(q5_item_destroy);

    {
        GHashTableIter it;
        gpointer k, v;
        g_hash_table_iter_init(&it, g_q5_cache.user_genre_counts);
        while (g_hash_table_iter_next(&it, &k, &v)) {
            const char *uname = k;
            GHashTable *map = v;
            q5_item_t *item;
            if (strcmp(uname, target) == 0) continue;
            item = g_new0(q5_item_t, 1);
            if (!item) continue;
            item->username = g_strdup(uname);
            item->dist = q5_distance(target_map, map);
            g_ptr_array_add(items, item);
        }
    }

    g_ptr_array_sort(items, q5_cmp_items);

    for (i = 0; i < items->len && (int)i < n; i++) {
        const q5_item_t *it = g_ptr_array_index(items, i);
        fprintf(out, "%s\n", it->username);
    }

    g_ptr_array_free(items, TRUE);
    free(copy);
}
