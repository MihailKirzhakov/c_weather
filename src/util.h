#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

#define SAFE_FREE(ptr) \
    do {               \
        free(ptr);     \
        (ptr) = NULL;  \
    } while (0)

#endif
