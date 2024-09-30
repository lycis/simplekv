const std = @import("std");

const targets: []const std.Target.Query = &.{
    .{ .cpu_arch = .x86_64, .os_tag = .windows },
    // linux not yet supported due to a lot of windows native function
    // .{ .cpu_arch = .x86_64, .os_tag = .linux }, 
};

pub fn buildDefault(b: *std.Build, name: []const u8, targetQuery: std.Target.Query, cFiles: []const []const u8, buildOpts: []const []const u8) void {
    const exe = b.addExecutable(.{
        .name = name,
        .target = b.resolveTargetQuery(targetQuery)
    });

    if (targetQuery.os_tag == .windows) {
        exe.linkSystemLibrary("ws2_32");
    }

    exe.linkLibC();
    exe.addIncludePath(.{
        .cwd_relative = "src",
    });



    exe.addCSourceFiles(.{ 
        .files = cFiles, 
        .flags = buildOpts, 
    });
    b.installArtifact(exe);
}


pub fn build(b: *std.Build) void {
    for (targets) |t| {
        buildDefault(b, "server", t, &.{
            "src/server.c",
            "src/kvstore.c",
            "src/utilfuns.c"
            }, &.{
                "-Wall", 
                "-std=c23"
            }
        );

        buildDefault(b, "client", t, &.{
            "src/client.c"
            }, &.{
                "-Wall", 
                "-std=c23"
            }
        );

        buildDefault(b, "server_test", t, &.{
            "src/utilfuns.c",
            "src/kvstore.c",
            "src/server.c",
            "src/server_unit_tests.c"
            }, &.{
                "-Wall", 
                "-std=c23",
                "-DUNIT_TEST"
            }
        );
    }
}
