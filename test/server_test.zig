const std = @import("std");

const ckvstore = @cImport({
    @cInclude("kvstore.h");
});

const C = @cImport({
    @cInclude("stdlib.h");
    @cInclude("strings.h");
});

test "kvstore: put and get a value" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const key  = "key";
    const value: [*:0]const u8 = "value";

    const result = ckvstore.kv_store_put(store, key, value);
    try std.testing.expect(result == 0);

    const retrieved_value = std.mem.span(ckvstore.kv_store_get(store, key));
    try std.testing.expectEqualStrings(std.mem.span(value), retrieved_value);
}

test "kvstore: put overwrites existing value" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const key  = "key";
    var result = ckvstore.kv_store_put(store, key, "initial value");
    try std.testing.expect(result == 0);

    result = ckvstore.kv_store_put(store, key, "new value");
    try std.testing.expect(result == 0);

    const retrieved_value = ckvstore.kv_store_get(store, key);
    try std.testing.expectEqualStrings(std.mem.span(retrieved_value), "new value");
}

test "kvstore: get returns null for nonexistent key" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const key = "nonexistent";
    const retrieved_value = ckvstore.kv_store_get(store, key);
    try std.testing.expect(retrieved_value == null);
}

test "kvstore: handles large number of entries" {
    const nr: u32 = 10000;
    const store = ckvstore.create_kv_store(nr);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const test_allocator = std.testing.allocator;
    var i: u32 = 0;
    while(i < nr) {
        const key = std.fmt.allocPrintZ(test_allocator, "key-{d}", .{i}) catch |err| {
            return err;
        };
        defer test_allocator.free(key);
        const value = std.fmt.allocPrintZ(test_allocator, "value-{d}", .{i})  catch |err| {
            return err;
        };
        defer test_allocator.free(value);

        const result = ckvstore.kv_store_put(store, key, value);
        try std.testing.expect(result == 0);
        i += 1;
    }

    i = 0;
    while(i<nr) {
         const key = std.fmt.allocPrintZ(test_allocator, "key-{d}", .{i}) catch |err| {
            return err;
        };
        defer test_allocator.free(key);
        const value = std.fmt.allocPrintZ(test_allocator, "value-{d}", .{i})  catch |err| {
            return err;
        };
        defer test_allocator.free(value);

        const retrieved_value = ckvstore.kv_store_get(store, key);
        try std.testing.expect(C.strcmp(retrieved_value, value) == 0);

        i += 1;
    }
}

test "kvstore: put null key" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const value: [*:0]const u8 = "value";

    const result = ckvstore.kv_store_put(store, null, value);
    try std.testing.expect(result == -1);
}

test "kvstore: put null value" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const key = "key";
    const result = ckvstore.kv_store_put(store, key, null);
    try std.testing.expect(result == -1);
}

test "kvstore: resize key value store when growing over capacity" {
    const nr: u32 = 2;

    const store = ckvstore.create_kv_store(nr);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    const key1 = "key1";
    const value1: [*:0]const u8 = "value1";
    const result1 = ckvstore.kv_store_put(store, key1, value1);
    try std.testing.expect(result1 == 0);

    const key2 = "key2";
    const value2: [*:0]const u8 = "value2";
    const result2 = ckvstore.kv_store_put(store, key2, value2);
    try std.testing.expect(result2 == 0);

    
    const key3 = "key3";
    const value3: [*:0]const u8 = "valu32";
    const result3 = ckvstore.kv_store_put(store, key3, value3);
    try std.testing.expect(result3 == 0);

    const capacity = store.*.capacity;
    try std.testing.expectEqual(4, capacity);
} 

test "kvstore: keys are case sensitive" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);
    defer ckvstore.free_kv_store(store);

    _ = ckvstore.kv_store_put(store, "key", "value");
    _ = ckvstore.kv_store_put(store, "KEY", "VALUE");

    const lowercase = ckvstore.kv_store_get(store, "key");
    try std.testing.expectEqualStrings(std.mem.span(lowercase), "value");

    const uppercase = ckvstore.kv_store_get(store, "KEY");
    try std.testing.expectEqualStrings(std.mem.span(uppercase), "VALUE");
}