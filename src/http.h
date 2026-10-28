#ifndef HTTP_H
#define HTTP_H

typedef enum {
    HTTP_OK = 0,
    HTTP_ERR_INIT,
    HTTP_ERR_NOMEM,
    HTTP_ERR_NETWORK,
    HTTP_ERR_STATUS
} http_result_t;

typedef struct {
    char *body;
    long status;
    char error[256];
} http_response_t;

http_result_t http_fetch_weather(const char *city, http_response_t *resp);

void http_response_free(http_response_t *resp);

http_result_t http_global_init(void);
void http_global_cleanup(void);

#endif
