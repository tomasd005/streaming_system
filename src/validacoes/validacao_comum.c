#include "validacoes/validacao_comum.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static bool all_digits(const char *s, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

bool validacao_data(const char *s) {
    int y, m, d;
    if (!s || strlen(s) != 10) return false;
    if (s[4] != '/' || s[7] != '/') return false;
    if (!all_digits(s, 4) || !all_digits(s + 5, 2) || !all_digits(s + 8, 2)) return false;
    if (sscanf(s, "%4d/%2d/%2d", &y, &m, &d) != 3) return false;
    if (m < 1 || m > 12 || d < 1 || d > 31) return false;
    if (y > 2024) return false;
    if (y == 2024 && (m > 9 || (m == 9 && d > 9))) return false;
    return true;
}

bool validacao_datetime(const char *s) {
    int y, m, d, hh, mm, ss;
    if (!s || strlen(s) != 19) return false;
    if (s[4] != '/' || s[7] != '/' || s[10] != ' ' || s[13] != ':' || s[16] != ':') return false;
    if (sscanf(s, "%4d/%2d/%2d %2d:%2d:%2d", &y, &m, &d, &hh, &mm, &ss) != 6) return false;
    if (m < 1 || m > 12 || d < 1 || d > 31) return false;
    if (y > 2024) return false;
    if (y == 2024 && (m > 9 || (m == 9 && d > 9))) return false;
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59) return false;
    return true;
}

bool validacao_duracao(const char *s) {
    int hh, mm, ss;
    if (!s || strlen(s) != 8) return false;
    if (s[2] != ':' || s[5] != ':') return false;
    if (sscanf(s, "%2d:%2d:%2d", &hh, &mm, &ss) != 3) return false;
    if (hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) return false;
    return true;
}

bool validacao_email(const char *s) {
    const char *at;
    const char *dot;
    if (!s) return false;
    at = strchr(s, '@');
    if (!at || at == s || *(at + 1) == '\0') return false;
    dot = strrchr(at + 1, '.');
    if (!dot || dot == at + 1) return false;
    if (strlen(dot + 1) < 2 || strlen(dot + 1) > 3) return false;
    return true;
}

bool validacao_lista(const char *s) {
    size_t n;
    if (!s) return false;
    n = strlen(s);
    return n >= 2 && s[0] == '[' && s[n - 1] == ']';
}
