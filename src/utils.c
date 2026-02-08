#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void utils_strip_newline(char *s) {
    size_t n;
    if (!s) return;
    n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

bool utils_join_path(char *out, int out_size, const char *pasta, const char *ficheiro) {
    int n;
    if (!out || out_size <= 0 || !pasta || !ficheiro) return false;
    n = snprintf(out, (size_t)out_size, "%s/%s", pasta, ficheiro);
    return n > 0 && n < out_size;
}

static char *trim_token(char *s) {
    char *e;
    while (*s && isspace((unsigned char)*s)) s++;
    e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) e--;
    *e = '\0';
    if (*s == '\'' || *s == '"') s++;
    e = s + strlen(s);
    if (e > s && (e[-1] == '\'' || e[-1] == '"')) e[-1] = '\0';
    return s;
}

GPtrArray *utils_parse_list_ids(const char *s) {
    GPtrArray *out = g_ptr_array_new_with_free_func(g_free);
    char *buf;
    char *tok;
    char *saveptr;
    size_t n;

    if (!out) return NULL;
    if (!s) return out;
    n = strlen(s);
    if (n < 2 || s[0] != '[' || s[n - 1] != ']') return out;

    buf = g_strndup(s + 1, n - 2);
    if (!buf) return out;

    tok = strtok_r(buf, ",", &saveptr);
    while (tok) {
        char *t = trim_token(tok);
        if (*t) g_ptr_array_add(out, g_strdup(t));
        tok = strtok_r(NULL, ",", &saveptr);
    }

    g_free(buf);
    return out;
}

int utils_duration_to_seconds(const char *s) {
    int hh, mm, ss;
    if (!s) return -1;
    if (sscanf(s, "%2d:%2d:%2d", &hh, &mm, &ss) != 3) return -1;
    if (hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) return -1;
    return hh * 3600 + mm * 60 + ss;
}

void utils_seconds_to_hhmmss(int total_seconds, char *out, size_t out_size) {
    int h, m, s;
    if (!out || out_size == 0) return;
    if (total_seconds < 0) total_seconds = 0;
    h = total_seconds / 3600;
    m = (total_seconds % 3600) / 60;
    s = total_seconds % 60;
    snprintf(out, out_size, "%02d:%02d:%02d", h, m, s);
}

int utils_age_on_2024_09_09(const char *birth_date) {
    int y, m, d;
    int age;
    if (!birth_date) return 0;
    if (sscanf(birth_date, "%4d/%2d/%2d", &y, &m, &d) != 3) return 0;
    age = 2024 - y;
    if (m > 9 || (m == 9 && d > 9)) age--;
    if (age < 0) age = 0;
    return age;
}
