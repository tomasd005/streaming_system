#include "queries/query1.h"

#include "gestor_programa/gestor_programa.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void query1_executar(struct gestor_programa *gp, const char *args, FILE *out) {
    char *copy;
    char *tok;
    char *id;
    const user_t *u;
    const artista_t *a;
    const char *sep;

    if (!gp || !args || !out) return;

    copy = strdup(args);
    if (!copy) {
        fprintf(out, "\n");
        return;
    }

    sep = use_alt_separator(args) ? "=" : ";";

    tok = strtok(copy, " \t\r\n");
    id = strtok(NULL, " \t\r\n");
    (void)tok;

    if (!id) {
        fprintf(out, "\n");
        free(copy);
        return;
    }

    u = gestor_users_obter(gp->users, id);
    if (u) {
        fprintf(out, "%s%s%s%s%s%s%d%s%s\n",
                user_email(u), sep,
                user_first_name(u), sep,
                user_last_name(u), sep,
                utils_age_on_2024_09_09(user_birth_date(u)), sep,
                user_country(u));
        free(copy);
        return;
    }

    a = gestor_artistas_obter(gp->artistas, id);
    if (a) {
        fprintf(out, "%s%s%s%s%s%s%d%s%.2f\n",
                artista_name(a), sep,
                artista_type(a), sep,
                artista_country(a), sep,
                artista_num_albums_individual(a), sep,
                artista_total_recipe(a));
        free(copy);
        return;
    }

    fprintf(out, "\n");
    free(copy);
}
