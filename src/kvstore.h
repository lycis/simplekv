#ifndef _KVSTORE_H_
#define _KVSTORE_H_

#include <stddef.h>

typedef struct kv_entry {
    char* key;
    char* value;
} kv_entry;

typedef struct kv_store {
    kv_entry* entries;   // Dynamic array of entries
    size_t capacity;            // Maximum number of entries before resizing
    size_t size;                // Current number of entries
} kv_store;

// prototypes
kv_store* create_kv_store(int initialCapacity); // initialize a new key value store
int kv_store_resize(kv_store* store);  // resize an existing key value store by doubling its capacity
int kv_store_put(kv_store* store, const char* key, const char* value) ; // add or overwrite a key value pair in the store
const char* kv_store_get(kv_store* store, const char* key); // retrieve the value associated with a key from the store
void free_kv_store(kv_store* store); // free the memory allocated for the key value store (incl. all values)
char* duplicate_string(const char* str); // duplicate a string in memory
int kv_store_delete(kv_store* store, const char* key); // delete a key value pair from the store

#endif