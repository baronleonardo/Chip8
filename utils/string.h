#ifndef STRING_H
#define STRING_H

#include <string.h>

typedef struct {
    char* data;
    size_t len;
} String;

#define String_from_char_ptr(data) ((String) { data, sizeof(data) })

#endif // STRING_H