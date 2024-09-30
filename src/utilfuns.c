#include "utilfuns.h"
#include <stdlib.h>
#include <string.h>

char* duplicate_string(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);
    char* dup = malloc(len + 1);
    if (dup == NULL) {
        return NULL;
    }


    errno_t err = strcpy_s(dup, len + 1, str);
    if(err != 0) {
        free(dup);
        return NULL;
    }

    return dup;
}
