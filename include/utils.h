#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

#include <glib.h>

void utils_strip_newline(char *s);
bool utils_join_path(char *out, int out_size, const char *pasta, const char *ficheiro);

GPtrArray *utils_parse_list_ids(const char *s);
int utils_duration_to_seconds(const char *s);
void utils_seconds_to_hhmmss(int total_seconds, char *out, size_t out_size);
int utils_age_on_2024_09_09(const char *birth_date);
bool utils_parse_date_ymd(const char *s, int *y, int *m, int *d);
bool utils_parse_datetime(const char *s, int *y, int *m, int *d, int *hh, int *mm, int *ss);
int utils_days_from_civil(int y, int m, int d);
int utils_weekday_sun0(int day_ordinal);

#endif
