#include "queries/query2.h"

#include "gestor_programa/gestor_programa.h"
#include "utils.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *country;
    GPtrArray *arr;
} q2_collect_ctx_t;

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

static void collect_artists_cb(artista_t *a, void *ctxp) {
    q2_collect_ctx_t *ctx = ctxp;
    if (!ctx->country || strcmp(ctx->country, artista_country(a)) == 0) {
        g_ptr_array_add(ctx->arr, a);
    }
}

static gint cmp_artistas(gconstpointer ap, gconstpointer bp) {
    const artista_t *a = *(const artista_t * const *)ap;
    const artista_t *b = *(const artista_t * const *)bp;
    int da = artista_discografia_seconds(a);
    int db = artista_discografia_seconds(b);
    if (da != db) return (db - da);
    return strcmp(artista_id(a), artista_id(b));
}

void query2_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    char *copy;
    char *tok;
    char *n_tok;
    char *country;
    int n;
    guint i;
    GPtrArray *arr;
    q2_collect_ctx_t ctx;
    const char *sep;

    if (!gp || !args || !out) return;

    copy = strdup(args);
    if (!copy) {
        fprintf(out, "\n");
        return;
    }

    sep = use_alt_separator(args) ? "=" : ";";

    tok = strtok(copy, " \t\r\n");
    n_tok = strtok(NULL, " \t\r\n");
    country = strtok(NULL, " \t\r\n");
    (void)tok;

    if (!n_tok) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    n = atoi(n_tok);
    if (n <= 0) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    arr = g_ptr_array_new();
    if (!arr) {
        free(copy);
        return;
    }

    ctx.country = country;
    ctx.arr = arr;
    gestor_artistas_para_cada(gp->artistas, collect_artists_cb, &ctx);
    g_ptr_array_sort(arr, cmp_artistas);

    for (i = 0; i < arr->len && (int)i < n; i++) {
        const artista_t *a = g_ptr_array_index(arr, i);
        char dur[32];
        utils_seconds_to_hhmmss(artista_discografia_seconds(a), dur, sizeof(dur));
        fprintf(out, "%s%s%s%s%s%s%s\n",
                artista_name(a), sep,
                artista_type(a), sep,
                dur, sep,
                artista_country(a));
    }

    g_ptr_array_free(arr, TRUE);
    free(copy);
}
