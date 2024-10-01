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

    const key  = "key";
    const value: [*:0]const u8 = "value";

    const result = ckvstore.kv_store_put(store, key, value);
    try std.testing.expect(result == 0);

    const retrieved_value = std.mem.span(ckvstore.kv_store_get(store, key));
    try std.testing.expectEqualStrings(std.mem.span(value), retrieved_value);

    ckvstore.free_kv_store(store);
}

test "kvstore: put overwrites existing value" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);

    const key  = "key";
    var result = ckvstore.kv_store_put(store, key, "initial value");
    try std.testing.expect(result == 0);

    result = ckvstore.kv_store_put(store, key, "new value");
    try std.testing.expect(result == 0);

    const retrieved_value = ckvstore.kv_store_get(store, key);
    try std.testing.expectEqualStrings(std.mem.span(retrieved_value), "new value");

    ckvstore.free_kv_store(store);
}

test "kvstore: get returns null for nonexistent key" {
    const store = ckvstore.create_kv_store(1);
    try std.testing.expect(store != null);

    const key = "nonexistent";
    const retrieved_value = ckvstore.kv_store_get(store, key);
    try std.testing.expect(retrieved_value == null);

    ckvstore.free_kv_store(store);
}

test "kvstore: handles large number of entries" {
    const nr: u32 = 10000;
    const store = ckvstore.create_kv_store(nr);
    try std.testing.expect(store != null);

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

    ckvstore.free_kv_store(store);
}