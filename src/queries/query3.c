#include "queries/query3.h"

#include "gestor_programa/gestor_programa.h"
#include "utils.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const gestor_programa_t *gp;
    int min_age;
    int max_age;
    GHashTable *genre_likes;
} q3_ctx_t;

typedef struct {
    char *genre;
    int likes;
} q3_row_t;

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

static void hash_add_genre(GHashTable *h, const char *genre, int delta) {
    gpointer val;
    int current;
    if (!h || !genre) return;
    val = g_hash_table_lookup(h, genre);
    current = (int)GPOINTER_TO_INT(val);
    current += delta;
    g_hash_table_insert(h, g_strdup(genre), GINT_TO_POINTER(current));
}

static void q3_collect_user(const user_t *u, void *ctxp) {
    q3_ctx_t *ctx = ctxp;
    int age;
    guint i;
    const GPtrArray *liked;

    if (!u || !ctx) return;
    age = utils_age_on_2024_09_09(user_birth_date(u));
    if (age < ctx->min_age || age > ctx->max_age) return;

    liked = user_liked_music_ids(u);
    for (i = 0; liked && i < liked->len; i++) {
        const char *music_id = g_ptr_array_index((GPtrArray *)liked, i);
        const musica_t *m = gestor_musicas_obter(ctx->gp->musicas, music_id);
        if (!m) continue;
        hash_add_genre(ctx->genre_likes, musica_genre(m), 1);
    }
}

static void q3_collect_hash_item(gpointer key, gpointer value, gpointer user_data) {
    GPtrArray *arr = user_data;
    q3_row_t *row = g_new0(q3_row_t, 1);
    if (!row) return;
    row->genre = g_strdup((const char *)key);
    row->likes = (int)GPOINTER_TO_INT(value);
    g_ptr_array_add(arr, row);
}

static void q3_row_destroy(gpointer p) {
    q3_row_t *r = p;
    if (!r) return;
    g_free(r->genre);
    g_free(r);
}

static gint q3_cmp_rows(gconstpointer a, gconstpointer b) {
    const q3_row_t *ra = *(const q3_row_t * const *)a;
    const q3_row_t *rb = *(const q3_row_t * const *)b;
    if (ra->likes != rb->likes) return rb->likes - ra->likes;
    return strcmp(ra->genre, rb->genre);
}

void query3_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    char *copy;
    char *tok;
    char *min_tok;
    char *max_tok;
    int min_age;
    int max_age;
    const char *sep;
    q3_ctx_t ctx;
    GPtrArray *rows;
    guint i;

    if (!gp || !args || !out) return;

    copy = strdup(args);
    if (!copy) {
        fprintf(out, "\n");
        return;
    }

    tok = strtok(copy, " \t\r\n");
    min_tok = strtok(NULL, " \t\r\n");
    max_tok = strtok(NULL, " \t\r\n");
    (void)tok;

    if (!min_tok || !max_tok) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    min_age = atoi(min_tok);
    max_age = atoi(max_tok);
    if (min_age > max_age || min_age < 0 || max_age < 0) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    ctx.gp = gp;
    ctx.min_age = min_age;
    ctx.max_age = max_age;
    ctx.genre_likes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    gestor_users_para_cada(gp->users, q3_collect_user, &ctx);

    rows = g_ptr_array_new_with_free_func(q3_row_destroy);
    g_hash_table_foreach(ctx.genre_likes, q3_collect_hash_item, rows);
    g_ptr_array_sort(rows, q3_cmp_rows);

    sep = use_alt_separator(args) ? "=" : ";";
    for (i = 0; i < rows->len; i++) {
        const q3_row_t *r = g_ptr_array_index(rows, i);
        fprintf(out, "%s%s%d\n", r->genre, sep, r->likes);
    }

    g_ptr_array_free(rows, TRUE);
    g_hash_table_destroy(ctx.genre_likes);
    free(copy);
}
