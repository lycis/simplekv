const std = @import("std");

pub fn main() void {
    const nr: u32 = 99999999;
    var i: u32 = 0;
    while(i < nr) {
      //  std.debug.print("i: {d}\n", .{i});
        i += 1;
    }
}