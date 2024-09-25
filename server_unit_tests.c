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

char* test_kvstr_parse_get_request() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing request failed", result == 0);
    cmunit_assert("operation not parsed", strcmp(req->operation, "GET") == 0);
    cmunit_assert("key not parsed", strcmp(req->key, "key") == 0);
    cmunit_assert("value not NULL", req->value == NULL);

    free_kvstr_request(req);

    return NULL;
}

char* test_kvstr_parse_del_request() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "DEL 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing request failed", result == 0);
    cmunit_assert("operation not parsed", strcmp(req->operation, "DEL") == 0);
    cmunit_assert("key not parsed", strcmp(req->key, "key") == 0);
    cmunit_assert("value not NULL", req->value == NULL);

    free_kvstr_request(req);

    return NULL;
}

char* test_kvstr_parse_put_request() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 5:value";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing request failed", result == 0);
    cmunit_assert("operation not parsed", strcmp(req->operation, "PUT") == 0);
    cmunit_assert("key not parsed", strcmp(req->key, "key") == 0);
    cmunit_assert("value not parsed", strcmp(req->value, "value") == 0);

    free_kvstr_request(req);

    return NULL;
}

char* test_kvstr_parse_put_missing_value() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing key error", result == -4);

    free_kvstr_request(req);

    return NULL;
}

char* test_parse_kvstr_with_empty_string_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing operation error", result == -2);

    free_kvstr_request(req);

    return NULL;
}

char* test_parse_kvstr_with_null_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    int result = kvstr_parse_request(NULL, req);
    cmunit_assert("parsing should result in invalid input error", result == -1);

    free_kvstr_request(req);

    return NULL;
}

char* test_parse_kvstr_with_invalid_operation_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "INVALID 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid operation error", result == -2);
    cmunit_assert("invalid operation should be NULL", req->operation == NULL);

    free_kvstr_request(req);

    return NULL;
}

char* test_parse_kvstr_with_invalid_key_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 3:";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key error", result == -3);

    free_kvstr_request(req);

    return NULL;
}

char* test_parse_kvstr_with_missing_value_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing value error", result == -4);

    free_kvstr_request(req);

    return NULL;
}

char* test_parse_kvstr_with_invalid_value_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 1:";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing value error", result == -4);

    free_kvstr_request(req);

    return NULL;
}

char* test_kvstr_parse_zero_length_key() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 0:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key error", result == -3);

    free_kvstr_request(req);
    return NULL;
}

char* test_kvstr_parse_zero_length_value() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 0:value";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid value error", result == -4);

    free_kvstr_request(req);
    return NULL;
}

char* test_kvstr_parse_mismatched_key_length() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 5:key";  // Declared length is 5, but actual key is "key" (length 3)
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key length error", result == -3);

    free_kvstr_request(req);
    return NULL;
}

char* test_kvstr_parse_mismatched_value_length() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 10:value";  // Declared length is 10, but actual value is "value" (length 5)
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid value length error", result == -4);

    free_kvstr_request(req);
    return NULL;
}

char* test_kvstr_parse_request_with_junk_data() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 5:value EXTRA";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid request format error", result == -5);

    free_kvstr_request(req);
    return NULL;
}

char* test_kvstr_parse_del_request_with_junk_data() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "DEL 3:key EXTRA";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid request format error", result == -5);

    free_kvstr_request(req);
    return NULL;
}

char* test_kvstr_parse_request_with_short_key() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 10:key";  // Declared length is 10, but key is "key" (length 3)
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key length error", result == -3);

    free_kvstr_request(req);
    return NULL;
}



#ifdef UNIT_TEST
// unit test execution
int main(void) {
    cmunit_init();

    // test request parsing from string which is a key function
    cmunit_run_test(test_create_and_free_kvstr_request);
    cmunit_run_test(test_kvstr_parse_get_request);
    cmunit_run_test(test_kvstr_parse_del_request);
    cmunit_run_test(test_kvstr_parse_put_request);
    cmunit_run_test(test_kvstr_parse_put_missing_value);
    cmunit_run_test(test_parse_kvstr_with_empty_string_fails);
    cmunit_run_test(test_parse_kvstr_with_null_fails);
    cmunit_run_test(test_parse_kvstr_with_invalid_operation_fails);
    cmunit_run_test(test_parse_kvstr_with_invalid_key_fails);
    cmunit_run_test(test_parse_kvstr_with_missing_value_fails);
    cmunit_run_test(test_parse_kvstr_with_invalid_value_fails);
    cmunit_run_test(test_kvstr_parse_zero_length_key);
    cmunit_run_test(test_kvstr_parse_zero_length_value);
    cmunit_run_test(test_kvstr_parse_mismatched_key_length);
    cmunit_run_test(test_kvstr_parse_mismatched_value_length);
    cmunit_run_test(test_kvstr_parse_request_with_junk_data);
    cmunit_run_test(test_kvstr_parse_del_request_with_junk_data);
    cmunit_run_test(test_kvstr_parse_request_with_short_key);

    cmunit_summary();

    return _cmunit_test_errors;
}
#endif