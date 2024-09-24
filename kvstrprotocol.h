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
    // Format: DEL <key_len:string><key:string>\0
    int key_len = strlen(key);
    int msg_len = 4 + 4 + key_len + 1;  // "DEL " (4 chars) + key_len + '\0'
    
    char* request = malloc(msg_len);
    if (request == NULL) {
        return NULL;  // Handle malloc failure
    }
    
    // Format the string as "DEL <key>"
    snprintf(request, msg_len, "DEL %s", key);
    
    return request;  // Return the serialized request
}



struct kvstr_request {
    char* key;
    char* value;
    char* operation;
};

int kvstr_parse_request(const char* request_str, struct kvstr_request* result) {
    if (request_str == NULL || result == NULL) {
        return -1;  // Invalid input
    }

    // Initialize result fields to NULL
    result->operation = NULL;
    result->key = NULL;
    result->value = NULL;

    // Parse the operation (first part before space)
    const char* space_ptr = strchr(request_str, ' ');
    if (!space_ptr) {
        return -1;  // Malformed request (no space found)
    }

    // Allocate and copy operation string
    int operation_len = space_ptr - request_str;
    result->operation = (char*)malloc(operation_len + 1);
    if (!result->operation) {
        return -1;  // Memory allocation failure
    }
    strncpy(result->operation, request_str, operation_len);
    result->operation[operation_len] = '\0';  // Null-terminate operation string

    // Move pointer to the part after the operation (key length and key)
    const char* after_op_ptr = space_ptr + 1;

    // Parse the key length (number before colon)
    char* colon_ptr = strchr(after_op_ptr, ':');
    if (!colon_ptr) {
        free(result->operation);
        return -1;  // Malformed request (no colon found)
    }

    int key_len = atoi(after_op_ptr);  // Convert key length string to integer
    if (key_len <= 0) {
        free(result->operation);
        return -1;  // Invalid key length
    }

    // Move to the key (after colon)
    const char* key_ptr = colon_ptr + 1;

    // Allocate and copy key string
    result->key = (char*)malloc(key_len + 1);
    if (!result->key) {
        free(result->operation);
        return -1;  // Memory allocation failure
    }
    strncpy(result->key, key_ptr, key_len);
    result->key[key_len] = '\0';  // Null-terminate key string

    // Check if this is a PUT request, which includes a value
    if (strcmp(result->operation, "PUT") == 0) {
        // Move to the part after the key (look for the space)
        const char* after_key_ptr = key_ptr + key_len;
        if (*after_key_ptr != ' ') {
            free(result->operation);
            free(result->key);
            return -1;  // Malformed request (no space after key)
        }
        after_key_ptr++;  // Move past the space

        // Parse the value length (number before colon)
        char* value_colon_ptr = strchr(after_key_ptr, ':');
        if (!value_colon_ptr) {
            free(result->operation);
            free(result->key);
            return -1;  // Malformed request (no colon after value length)
        }

        int value_len = atoi(after_key_ptr);  // Convert value length string to integer
        if (value_len <= 0) {
            free(result->operation);
            free(result->key);
            return -1;  // Invalid value length
        }

        // Move to the value (after colon)
        const char* value_ptr = value_colon_ptr + 1;

        // Allocate and copy value string
        result->value = (char*)malloc(value_len + 1);
        if (!result->value) {
            free(result->operation);
            free(result->key);
            return -1;  // Memory allocation failure
        }
        strncpy(result->value, value_ptr, value_len);
        result->value[value_len] = '\0';  // Null-terminate value string
    }

    return 0;  // Success
}

void free_kvstr_request(struct kvstr_request* req) {
    if (req->operation != NULL) {
        free(req->operation);
    }
    if (req->key != NULL) {
        free(req->key);
    }
    if (req->value != NULL) {
        free(req->value);
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