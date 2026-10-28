#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "weather.h"

enum {
    EXIT_USAGE = 2,
    EXIT_NETWORK = 3,
    EXIT_UNKNOWN_CITY = 4,
    EXIT_BAD_RESPONSE = 5
};

static int is_unknown_location(const http_response_t *resp)
{
    if (resp->status == 404) {
        return 1;
    }
    return resp->body != NULL &&
           (strstr(resp->body, "not found") != NULL ||
            strstr(resp->body, "nknown location") != NULL);
}

static int run(const char *city)
{
    http_response_t resp;
    http_result_t hr = http_fetch_weather(city, &resp);
    int code = EXIT_SUCCESS;

    if (hr == HTTP_ERR_NOMEM) {
        fprintf(stderr, "Error: out of memory\n");
        code = EXIT_FAILURE;
    } else if (hr == HTTP_ERR_INIT || hr == HTTP_ERR_NETWORK) {
        fprintf(stderr, "Network error: %s\n", resp.error);
        code = EXIT_NETWORK;
    } else if (hr == HTTP_ERR_STATUS && !is_unknown_location(&resp)) {
        fprintf(stderr, "Service error: %s\n", resp.error);
        code = EXIT_NETWORK;
    } else if (hr == HTTP_ERR_STATUS || resp.body == NULL) {
        fprintf(stderr, "Location \"%s\" not found\n", city);
        code = EXIT_UNKNOWN_CITY;
    } else {
        weather_t w;
        char err[128];
        weather_result_t wr = weather_parse(resp.body, &w, err, sizeof err);

        if (wr == WEATHER_ERR_NOT_JSON) {
            fprintf(stderr, "Location \"%s\" not found\n", city);
            code = EXIT_UNKNOWN_CITY;
        } else if (wr == WEATHER_ERR_FORMAT) {
            fprintf(stderr, "Unexpected response format: %s\n", err);
            code = EXIT_BAD_RESPONSE;
        } else {
            weather_print(&w, city);
        }
    }

    http_response_free(&resp);
    return code;
}

int main(int argc, char *argv[])
{
    if (argc != 2 || argv[1][0] == '\0') {
        fprintf(stderr, "Usage: %s <city>\n", argc > 0 ? argv[0] : "weather");
        return EXIT_USAGE;
    }

    if (http_global_init() != HTTP_OK) {
        fprintf(stderr, "Error: cannot initialise libcurl\n");
        return EXIT_FAILURE;
    }

    int code = run(argv[1]);
    http_global_cleanup();
    return code;
}
