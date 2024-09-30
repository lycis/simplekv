#include <stdlib.h>
#include <string.h>
#include "kvstore.h"
#include "utilfuns.h"

kv_store* create_kv_store(int initialCapcity) {
    kv_store* store = calloc(1, sizeof(kv_store));
    if (store == NULL) {
        return NULL;
    }

    store->entries = calloc(initialCapcity, sizeof(kv_entry));
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
    struct kv_entry* new_entries = realloc(store->entries, new_capacity * sizeof(kv_entry));
    if (new_entries == NULL) {
        return -1;
    }

    size_t old_capacity = store->capacity;
    memset(new_entries + old_capacity, 0, (new_capacity - old_capacity) * sizeof(kv_entry));

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
        if(store->entries[i].key == NULL) {
            // there is a free slot we can use
            store->entries[i].key = duplicate_string(key);
            store->entries[i].value = duplicate_string(value);
            return 0;
        }
        
        if (strcmp(store->entries[i].key, key) == 0) {
            free(store->entries[i].value);
            store->entries[i].value = duplicate_string(value);
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

    store->entries[store->size].key = duplicate_string(key);
    store->entries[store->size].value = duplicate_string(value);
    store->size++;
    return 0; 
}

const char* kv_store_get(kv_store* store, const char* key) {
    for (size_t i = 0; i < store->size; i++) {
        if(store->entries[i].key == NULL) continue;
        if (strcmp(store->entries[i].key, key) == 0) {
            return store->entries[i].value;  // Return the associated value
        }
    }

    return NULL;  // Key not found
}

int kv_store_delete(kv_store* store, const char* key) {
    for (size_t i = 0; i < store->size; i++) {
        if (strcmp(store->entries[i].key, key) == 0) {
            free(store->entries[i].key);
            free(store->entries[i].value);
            store->entries[i].key = NULL;
            store->entries[i].value = NULL;
            return 0;
        }
    }

    return -1;
}

void free_kv_store(kv_store* store) {
    for (size_t i = 0; i < store->size; i++) {
        if(store->entries[i].key != NULL) free(store->entries[i].key);
        if(store->entries[i].value != NULL) free(store->entries[i].value);
    }
    free(store->entries);
    free(store);
}
