#include "cmunit.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "kvstore.h"

#ifndef SKVS_SERVER
#include "server.c"
#endif

#ifndef UNIT_TEST
char* _mock_lastMessage = NULL;
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

    free_kvstr_request(&req);
    cmunit_assert("request not freed", req == NULL);
    
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

    free_kvstr_request(&req);

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

    free_kvstr_request(&req);

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

    free_kvstr_request(&req);

    return NULL;
}

char* test_kvstr_parse_put_missing_value() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing key error", result == -4);

    free_kvstr_request(&req);

    return NULL;
}

char* test_parse_kvstr_with_empty_string_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing operation error", result == -2);

    free_kvstr_request(&req);

    return NULL;
}

char* test_parse_kvstr_with_null_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    int result = kvstr_parse_request(NULL, req);
    cmunit_assert("parsing should result in invalid input error", result == -1);

    free_kvstr_request(&req);

    return NULL;
}

char* test_parse_kvstr_with_invalid_operation_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "INVALID 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid operation error", result == -2);
    cmunit_assert("invalid operation should be NULL", req->operation == NULL);

    free_kvstr_request(&req);

    return NULL;
}

char* test_parse_kvstr_with_invalid_key_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 3:";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key error", result == -3);

    free_kvstr_request(&req);

    return NULL;
}

char* test_parse_kvstr_with_missing_value_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing value error", result == -4);

    free_kvstr_request(&req);

    return NULL;
}

char* test_parse_kvstr_with_invalid_value_fails() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 1:";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in missing value error", result == -4);

    free_kvstr_request(&req);

    return NULL;
}

char* test_kvstr_parse_zero_length_key() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 0:key";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key error", result == -3);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kvstr_parse_zero_length_value() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 0:value";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid value error", result == -4);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kvstr_parse_mismatched_key_length() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 5:key";  // Declared length is 5, but actual key is "key" (length 3)
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key length error", result == -3);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kvstr_parse_mismatched_value_length() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 10:value";  // Declared length is 10, but actual value is "value" (length 5)
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid value length error", result == -4);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kvstr_parse_request_with_junk_data() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "PUT 3:key 5:value EXTRA";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid request format error", result == -5);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kvstr_parse_del_request_with_junk_data() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "DEL 3:key EXTRA";
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid request format error", result == -5);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kvstr_parse_request_with_short_key() {
    struct kvstr_request* req = create_kvstr_request();
    cmunit_assert("allocating kvstr request failed", req != NULL);

    const char* request_str = "GET 10:key";  // Declared length is 10, but key is "key" (length 3)
    int result = kvstr_parse_request(request_str, req);
    cmunit_assert("parsing should result in invalid key length error", result == -3);

    free_kvstr_request(&req);
    return NULL;
}

char* test_kv_store_put_and_retrieve_a_value() {
    kv_store* store = create_kv_store(1);
    cmunit_assert("allocating kv_store failed", store != NULL);

    const char* key = "key";
    const char* value = "value";
    int result = kv_store_put(store, key, value);
    cmunit_assert("putting value failed", result == 0);

    const char* retrieved_value = kv_store_get(store, key);
    cmunit_assert("retrieved value does not equal put value", strcmp(retrieved_value, value) == 0);

    free_kv_store(store); // Free the store after the test

    return NULL;
}

char* test_kv_store_put_overwrites_existing_value() {
    kv_store* store = create_kv_store(1);
    cmunit_assert("allocating kv_store failed", store != NULL);

    const char* key = "key";
    const char* initial_value = "initial_value";
    int result = kv_store_put(store, key, initial_value);
    cmunit_assert("putting initial value failed", result == 0);

    const char* new_value = "new_value";
    result = kv_store_put(store, key, new_value);
    cmunit_assert("putting new value failed", result == 0);

    const char* retrieved_value = kv_store_get(store, key);
    cmunit_assert("retrieved value does not equal new value", strcmp(retrieved_value, new_value) == 0);

    free_kv_store(store); // Free the store after the test

    return NULL;
}

char* test_kv_store_get_returns_null_for_non_existent_key() {
    kv_store* store = create_kv_store(1);
    cmunit_assert("allocating kv_store failed", store != NULL);

    const char* key = "non_existent_key";
    const char* retrieved_value = kv_store_get(store, key);
    cmunit_assert("retrieved value for non-existent key should be NULL", retrieved_value == NULL);

    free_kv_store(store); // Free the store after the test

    return NULL;
}

char* test_kv_store_handles_large_number_of_entries() {
    kv_store* store = create_kv_store(1000);
    cmunit_assert("allocating kv_store failed", store != NULL);

    const int num_entries = 1000;
    for (int i = 0; i < num_entries; i++) {
        char key[16], value[16];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);

        int result = kv_store_put(store, key, value);
        cmunit_assert("putting value in large store failed", result == 0);
    }

    for (int i = 0; i < num_entries; i++) {
        char key[16], expected_value[16];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(expected_value, sizeof(expected_value), "value_%d", i);

        const char* retrieved_value = kv_store_get(store, key);
        cmunit_assert("retrieved value does not equal expected value", strcmp(retrieved_value, expected_value) == 0);
    }

    free_kv_store(store); // Free the store after the test

    return NULL;
}

char* test_kv_store_put_null_key_or_value() {
    kv_store* store = create_kv_store(10);
    cmunit_assert("allocating kv_store failed", store != NULL);

    int result = kv_store_put(store, NULL, "value");
    cmunit_assert("putting NULL key should fail", result == -1);

    result = kv_store_put(store, "key", NULL);
    cmunit_assert("putting NULL value should fail", result == -1);

    free_kv_store(store);

    return NULL;
}

char* test_kv_store_resizable() {
    kv_store* store = create_kv_store(2); // Start with a small capacity
    cmunit_assert("allocating kv_store failed", store != NULL);

    kv_store_put(store, "key1", "value1");
    kv_store_put(store, "key2", "value2");

    // Store should expand when adding a third key
    kv_store_put(store, "key3", "value3");
    const char* retrieved_value = kv_store_get(store, "key3");
    cmunit_assert("store did not expand properly", strcmp(retrieved_value, "value3") == 0);

    free_kv_store(store);

    return NULL;
}

char* test_kv_store_case_sensitivity() {
    kv_store* store = create_kv_store(5);
    cmunit_assert("allocating kv_store failed", store != NULL);

    kv_store_put(store, "key", "value1");
    kv_store_put(store, "KEY", "value2");

    const char* retrieved_value1 = kv_store_get(store, "key");
    const char* retrieved_value2 = kv_store_get(store, "KEY");

    cmunit_assert("case-sensitive key retrieval failed for 'key'", strcmp(retrieved_value1, "value1") == 0);
    cmunit_assert("case-sensitive key retrieval failed for 'KEY'", strcmp(retrieved_value2, "value2") == 0);

    return NULL;
}

char* test_handlePutRequest_validInput() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1;
    const char *key = "testKey";
    const char *value = "testValue";

    handlePutRequest(mockSocket, key, value);
    
    const char* retrieved = kv_store_get(gl_kvStore, key);
    cmunit_assert("value not stored in database", strcmp(retrieved, value) == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handlePutRequest_nullKey() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1; // Mock socket
    const char *key = NULL; // Invalid key
    const char *value = "testValue";

    handlePutRequest(mockSocket, key, value);

    const char* retrieved = kv_store_get(gl_kvStore, key);
    cmunit_assert("Null key should not store value", retrieved == NULL);
    cmunit_assert("wrong error message sent.'", strcmp(_mock_lastMessage, "500 Internal Server Error: Key and value must not be NULL.") == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handlePutRequest_emptyKey() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1; // Mock socket
    const char *key = ""; // Invalid key
    const char *value = "testValue";

    handlePutRequest(mockSocket, key, value);

    const char* retrieved = kv_store_get(gl_kvStore, key);
    cmunit_assert("Empty key should not store value", retrieved == NULL);
    cmunit_assert("wrong error message sent.'", strcmp(_mock_lastMessage, "400 Bad Request: Key and value must not be empty.") == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handlePutRequest_nullValue() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1; // Mock socket
    const char *key = "testKey";
    const char *value = NULL; // Invalid value

    handlePutRequest(mockSocket, key, value);

    const char* retrieved = kv_store_get(gl_kvStore, key);
    cmunit_assert("Null value should not store in database", retrieved == NULL);
    cmunit_assert("wrong error message sent.'", strcmp(_mock_lastMessage, "500 Internal Server Error: Key and value must not be NULL.") == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handlePutRequest_emptyValue() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1; // Mock socket
    const char *key = "testKey";
    const char *value = ""; // Invalid value

    handlePutRequest(mockSocket, key, value);

    const char* retrieved = kv_store_get(gl_kvStore, key);
    cmunit_assert("Empty value should not store in database", retrieved == NULL);
    cmunit_assert("wrong error message sent.'", strcmp(_mock_lastMessage, "400 Bad Request: Key and value must not be empty.") == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handleGetRequest_validKey() {
    gl_kvStore = create_kv_store(1);

    const char *key = "testKey";
    const char *value = "testValue";
    kv_store_put(gl_kvStore, key, value);

    SOCKET mockSocket = 1;

    handleGetRequest(mockSocket, key);

    cmunit_assert("wrong response message sent.", strcmp(_mock_lastMessage, "200 testValue") == 0);

    free_kv_store(gl_kvStore);

    return NULL;
}

char* test_handleGetRequest_nonexistentKey() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1;

    const char *key = "nonExistentKey";

    handleGetRequest(mockSocket, key);

    cmunit_assert("wrong response message sent for non-existent key.", strcmp(_mock_lastMessage, "404 Not Found") == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handleGetRequest_nullKey() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1;

    const char *key = NULL; // invalid key

    handleGetRequest(mockSocket, key);

    cmunit_assert("wrong response message sent for NULL key.", strcmp(_mock_lastMessage, "400 Bad Request: No key") == 0);

    free_kv_store(gl_kvStore);
    return NULL;
}

char* test_handleGetRequest_emptyKey() {
    gl_kvStore = create_kv_store(100);

    SOCKET mockSocket = 1;

    const char *key = ""; // empty key

    handleGetRequest(mockSocket, key);

    // Check if the correct response was sent to the client socket (assuming 400 is returned for invalid requests)
    cmunit_assert("wrong response message sent for empty key.", strcmp(_mock_lastMessage, "400 Bad Request: No key") == 0);

    free_kv_store(gl_kvStore);
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

    // tests for the key value store basic operations
    cmunit_run_test(test_kv_store_put_and_retrieve_a_value);
    cmunit_run_test(test_kv_store_put_overwrites_existing_value);
    cmunit_run_test(test_kv_store_get_returns_null_for_non_existent_key);
    cmunit_run_test(test_kv_store_handles_large_number_of_entries);
    cmunit_run_test(test_kv_store_put_null_key_or_value);
    cmunit_run_test(test_kv_store_resizable);
    cmunit_run_test(test_kv_store_case_sensitivity);

    // testing server side request handling
    cmunit_run_test(test_handlePutRequest_validInput);
    cmunit_run_test(test_handlePutRequest_nullKey);
    cmunit_run_test(test_handlePutRequest_emptyKey);
    cmunit_run_test(test_handlePutRequest_nullValue);
    cmunit_run_test(test_handlePutRequest_emptyValue);
    cmunit_run_test(test_handleGetRequest_validKey);
    cmunit_run_test(test_handleGetRequest_nonexistentKey);
    cmunit_run_test(test_handleGetRequest_nullKey);
    cmunit_run_test(test_handleGetRequest_emptyKey);

    cmunit_summary();

    return _cmunit_test_errors;
}
#endif