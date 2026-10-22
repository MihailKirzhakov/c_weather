#ifndef HTTP_H
#define HTTP_H

typedef enum {
    HTTP_OK = 0,
    HTTP_ERR_INIT,
    HTTP_ERR_NOMEM,
    HTTP_ERR_NETWORK,
    HTTP_ERR_STATUS
} HttpResult;

typedef struct {
    char *body;
    long status;
    char error[256];
} HttpResponse;

HttpResult http_fetch_weather(const char *city, HttpResponse *resp);

void http_response_free(HttpResponse *resp);

HttpResult http_global_init(void);
void http_global_cleanup(void);

#endif
