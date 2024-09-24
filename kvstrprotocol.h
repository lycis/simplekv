#ifndef _SKVSTR_PROTOCOL_H
#define _SKVSTR_PROTOCOL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* kvstr_build_get_request(const char* key) {
    // Validate input
    if (key == NULL) {
        return NULL;
    }

    // Calculate key length and total size for the request string
    int key_len = strlen(key);
    int buffer_size = strlen("GET ") + 10 + key_len + 2;  // +10 for key_len (max 10 digits), +2 for colon and '\0'
    
    // Allocate memory for the request
    char* request = (char*)malloc(buffer_size);
    if (!request) {
        return NULL;
    }

    // Build the request string
    snprintf(request, buffer_size, "GET %d:%s", key_len, key);

    return request;  // Caller is responsible for freeing the memory
}

char* kvstr_build_put_request(const char* key, const char* value) {
    // Validate input
    if (key == NULL || value == NULL) {
        return NULL;
    }

    // Calculate lengths
    int key_len = strlen(key);
    int value_len = strlen(value);

    // Calculate the buffer size: "PUT " + key_len + value_len + space + colons + null terminator
    int buffer_size = strlen("PUT ") + 10 + key_len + 1 + 10 + value_len + 2; // max 10 digits for key_len and value_len, 1 space, 2 colons

    // Allocate memory for the request string
    char* request = (char*)malloc(buffer_size);
    if (!request) {
        return NULL;  // Memory allocation failure
    }

    // Build the request string
    snprintf(request, buffer_size, "PUT %d:%s %d:%s", key_len, key, value_len, value);

    return request;  // Caller is responsible for freeing the memory
}

char* kvstr_build_del_request(const char* key) {
    if (key == NULL) {
        return NULL;
    }

    // Calculate key length and total size for the request string
    int key_len = strlen(key);
    int buffer_size = strlen("DEL ") + 10 + key_len + 2;  // +10 for key_len (max 10 digits), +2 for colon and '\0'
    
    // Allocate memory for the request
    char* request = (char*)malloc(buffer_size);
    if (!request) {
        return NULL;
    }

    // Build the request string
    snprintf(request, buffer_size, "DEL %d:%s", key_len, key);

    return request;  // Caller is responsible for freeing the memory
}



struct kvstr_request {
    char* key;
    char* value;
    char* operation;
};

void free_kvstr_request(struct kvstr_request* req) {
    if (req->operation != NULL) {
        free(req->operation);
        req->operation = NULL;
    }
    if (req->key != NULL) {
        free(req->key);
        req->key = NULL;
    }
    if (req->value != NULL) {
        free(req->value);
        req->value = NULL;
    }
}

struct kvstr_request* create_kvstr_request() {
    struct kvstr_request* req = (struct kvstr_request*)malloc(sizeof(struct kvstr_request));
    if (req == NULL) {
        return NULL;  // Memory allocation failure
    }

    req->operation = NULL;
    req->key = NULL;
    req->value = NULL;

    return req;
}

#endif