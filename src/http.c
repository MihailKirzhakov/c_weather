#include "http.h"

#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WTTR_URL_PREFIX "https://wttr.in/"
#define WTTR_URL_SUFFIX "?format=j1"
#define USER_AGENT "weather-hw/1.0"
#define CONNECT_TIMEOUT_SEC 5L
#define TOTAL_TIMEOUT_SEC 10L

typedef struct {
    char *data;
    size_t size;
} Memory;

static size_t write_cb(void *chunk, size_t size, size_t nmemb, void *userp)
{
    Memory *mem = userp;

    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        return 0;
    }
    size_t realsize = size * nmemb;
    if (realsize > SIZE_MAX - mem->size - 1) {
        return 0;
    }

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }
    mem->data = ptr;
    memcpy(mem->data + mem->size, chunk, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0';
    return realsize;
}

static void set_error(HttpResponse *resp, const char *fmt, const char *arg)
{
    snprintf(resp->error, sizeof resp->error, fmt, arg);
}

static char *build_url(CURL *curl, const char *city)
{
    char *escaped = curl_easy_escape(curl, city, 0);
    if (escaped == NULL) {
        return NULL;
    }

    size_t len = strlen(WTTR_URL_PREFIX) + strlen(escaped) + strlen(WTTR_URL_SUFFIX) + 1;
    char *url = malloc(len);
    if (url != NULL) {
        snprintf(url, len, "%s%s%s", WTTR_URL_PREFIX, escaped, WTTR_URL_SUFFIX);
    }
    curl_free(escaped);
    return url;
}

HttpResult http_global_init(void)
{
    return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK ? HTTP_OK : HTTP_ERR_INIT;
}

void http_global_cleanup(void)
{
    curl_global_cleanup();
}

void http_response_free(HttpResponse *resp)
{
    free(resp->body);
    resp->body = NULL;
    resp->status = 0;
    resp->error[0] = '\0';
}

HttpResult http_fetch_weather(const char *city, HttpResponse *resp)
{
    resp->body = NULL;
    resp->status = 0;
    resp->error[0] = '\0';

    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        set_error(resp, "%s", "cannot initialise libcurl handle");
        return HTTP_ERR_INIT;
    }

    char *url = build_url(curl, city);
    if (url == NULL) {
        curl_easy_cleanup(curl);
        set_error(resp, "%s", "out of memory");
        return HTTP_ERR_NOMEM;
    }

    Memory mem = { NULL, 0 };
    char errbuf[CURL_ERROR_SIZE] = "";

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT_SEC);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TOTAL_TIMEOUT_SEC);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    HttpResult result = HTTP_OK;
    CURLcode rc = curl_easy_perform(curl);

    if (rc == CURLE_WRITE_ERROR && mem.data == NULL) {
        set_error(resp, "%s", "out of memory");
        result = HTTP_ERR_NOMEM;
    } else if (rc != CURLE_OK) {
        set_error(resp, "%s", errbuf[0] != '\0' ? errbuf : curl_easy_strerror(rc));
        result = HTTP_ERR_NETWORK;
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp->status);
        if (resp->status != 200) {
            snprintf(resp->error, sizeof resp->error, "server returned HTTP %ld", resp->status);
            result = HTTP_ERR_STATUS;
        }
    }

    if (result == HTTP_OK || result == HTTP_ERR_STATUS) {
        resp->body = mem.data;
    } else {
        free(mem.data);
    }

    free(url);
    curl_easy_cleanup(curl);
    return result;
}
