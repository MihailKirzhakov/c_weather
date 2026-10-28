#include "weather.h"

#include <cjson/cJSON.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static const cJSON *array_item(const cJSON *obj, const char *key, int idx)
{
    const cJSON *arr = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (!cJSON_IsArray(arr)) {
        return NULL;
    }
    return cJSON_GetArrayItem(arr, idx);
}

static const char *string_field(const cJSON *obj, const char *key)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
    return cJSON_IsString(item) ? item->valuestring : NULL;
}

static int int_field(const cJSON *obj, const char *key, int *out)
{
    const char *s = string_field(obj, key);
    if (s == NULL || *s == '\0') {
        return 0;
    }

    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || *end != '\0' || v < INT_MIN || v > INT_MAX) {
        return 0;
    }
    *out = (int)v;
    return 1;
}

static void copy_string(char *dst, size_t dst_size, const char *src)
{
    snprintf(dst, dst_size, "%s", src != NULL ? src : "");
}

static weather_result_t fail(char *err, size_t err_size, const char *what)
{
    snprintf(err, err_size, "missing or invalid field: %s", what);
    return WEATHER_ERR_FORMAT;
}

weather_result_t weather_parse(const char *json, weather_t *out, char *err, size_t err_size)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        snprintf(err, err_size, "response is not valid JSON");
        return WEATHER_ERR_NOT_JSON;
    }

    weather_result_t result = WEATHER_OK;
    const cJSON *cur = array_item(root, "current_condition", 0);
    const cJSON *day = array_item(root, "weather", 0);

    if (!cJSON_IsObject(cur)) {
        result = fail(err, err_size, "current_condition");
    } else if (!cJSON_IsObject(day)) {
        result = fail(err, err_size, "weather");
    } else {
        const cJSON *desc = array_item(cur, "weatherDesc", 0);
        const char *desc_text = cJSON_IsObject(desc) ? string_field(desc, "value") : NULL;
        const char *dir = string_field(cur, "winddir16Point");

        if (desc_text == NULL) {
            result = fail(err, err_size, "weatherDesc");
        } else if (dir == NULL) {
            result = fail(err, err_size, "winddir16Point");
        } else if (!int_field(cur, "temp_C", &out->temp_c)) {
            result = fail(err, err_size, "temp_C");
        } else if (!int_field(cur, "windspeedKmph", &out->wind_kmh)) {
            result = fail(err, err_size, "windspeedKmph");
        } else if (!int_field(day, "mintempC", &out->min_c)) {
            result = fail(err, err_size, "mintempC");
        } else if (!int_field(day, "maxtempC", &out->max_c)) {
            result = fail(err, err_size, "maxtempC");
        } else {
            copy_string(out->description, sizeof out->description, desc_text);
            copy_string(out->wind_dir, sizeof out->wind_dir, dir);
        }
    }

    cJSON_Delete(root);
    return result;
}

void weather_print(const weather_t *w, const char *city)
{
    printf("Погода для города    | %s\n", city);
    printf("-----------------------------------------\n");
    printf("Погодные условия     | %s\n", w->description);
    printf("-----------------------------------------\n");
    printf("Температура          | сейчас %d градусов C\n", w->temp_c);
    printf("Мин <-> макс         | %d <-> %d градусов C\n", w->min_c, w->max_c);
    printf("-----------------------------------------\n");
    printf("Направление, V ветра | %s, %d км/ч\n", w->wind_dir, w->wind_kmh);
}
