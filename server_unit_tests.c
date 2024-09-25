#include "cmunit.h"
#include <string.h>
#include <stdio.h>

#ifndef SKVS_SERVER
#include "server.c"
#endif

char* test_create_and_free_kvstr_request() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    char* key = malloc(9);
    memset(key, '0', 9);
    strcpy_s(key, 8, "some key");
    req->key = key;

    char* operation = malloc(4);
    memset(operation, '0', 4);
    strcpy_s(operation, 3, "GET");  
    req->operation = operation;

    char* value = malloc(11);
    memset(value, '0', 11);
    strcpy_s(value, 10, "some value");
    req->value = value;

    free_kvstr_request(req);
    cmunit_assert("operation not freed", req->operation == NULL);
    cmunit_assert("key not freed", req->key == NULL);
    cmunit_assert("value not freed", req->value == NULL);
    
    return NULL;
}

char* test_kvstr_parse_request() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing request failed", result == 0);
    cmunit_assert("operation not parsed", strcmp(req->operation, "GET") == 0);
    cmunit_assert("key not parsed", strcmp(req->key, "key") == 0);
    cmunit_assert("value not NULL", req->value == NULL);

    return NULL;
}

#ifdef UNIT_TEST
// unit test execution
int main(void) {
    cmunit_init();

    cmunit_run_test(test_create_and_free_kvstr_request);
    cmunit_run_test(test_kvstr_parse_request);

    cmunit_summary();

    return _cmunit_test_errors;
}
#endif