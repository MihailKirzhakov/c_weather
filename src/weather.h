#ifndef WEATHER_H
#define WEATHER_H

#include <stddef.h>

typedef struct {
    char description[96];
    char wind_dir[8];
    int temp_c;
    int wind_kmh;
    int min_c;
    int max_c;
} weather_t;

typedef enum {
    WEATHER_OK = 0,
    WEATHER_ERR_NOT_JSON,
    WEATHER_ERR_FORMAT
} weather_result_t;

weather_result_t weather_parse(const char *json, weather_t *out, char *err, size_t err_size);

void weather_print(const weather_t *w, const char *city);

#endif
