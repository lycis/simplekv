#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

kv_store* create_kv_store(int initialCapcity) {
    kv_store* store = malloc(sizeof(kv_store));
    if (store == NULL) {
        return NULL;
    }

    store->entries = malloc(initialCapcity * sizeof(kv_entry));
    if (store->entries == NULL) {
        free(store);
        return NULL;
    }

    store->capacity = initialCapcity;
    store->size = 0;
    return store;
}

int kv_store_resize( kv_store* store) {
    size_t new_capacity = store->capacity * 2;
    struct kv_entry* new_entries = realloc(store->entries, new_capacity * sizeof(struct kv_entry));
    if (new_entries == NULL) {
        return -1;
    }

    store->entries = new_entries;
    store->capacity = new_capacity;
    return 0;
}

int kv_store_put(kv_store* store, const char* key, const char* value) {

    if(key == NULL || value == NULL) {
        return -1;
    }
    
    // Search for the key, if found, update the value
    for (size_t i = 0; i < store->size; i++) {
        if (strcmp(store->entries[i].key, key) == 0) {
            free(store->entries[i].value);
            store->entries[i].value = strdup(value);
            return 0;
        }
    }

    // If not found, insert new key-value pair
    if (store->size == store->capacity) {
        // Resize if necessary
        if (kv_store_resize(store) != 0) {
            return -1;
        }
    }

    store->entries[store->size].key = strdup(key);
    store->entries[store->size].value = strdup(value);
    store->size++;
    return 0; 
}

const char* kv_store_get(kv_store* store, const char* key) {
    for (size_t i = 0; i < store->size; i++) {
        if (strcmp(store->entries[i].key, key) == 0) {
            return store->entries[i].value;  // Return the associated value
        }
    }

    return NULL;  // Key not found
}

void free_kv_store(kv_store* store) {
    for (size_t i = 0; i < store->size; i++) {
        free(store->entries[i].key);
        free(store->entries[i].value);
    }
    free(store->entries);
    free(store);
}
