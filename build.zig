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
    const test_step = b.step("test", "Run unit tests");

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

        const unit_tests = b.addTest(.{
        .root_source_file = b.path("test/server_test.zig"),
        .target = b.resolveTargetQuery(t),
        });
        unit_tests.linkLibC();                          // Link against libc.
        unit_tests.addIncludePath(.{
            .cwd_relative = "src",
        });
        unit_tests.addCSourceFiles(.{
            .files = &.{
                "src/server.c",
                "src/kvstore.c",
                "src/utilfuns.c"
            },
            .flags = &.{
                "-Wall", 
                "-std=c23",
                "-DUNIT_TEST"
            },
        });
        if (t.os_tag == .windows) {
            unit_tests.linkSystemLibrary("ws2_32");
        }

        test_step.dependOn(&b.addRunArtifact(unit_tests).step);
    }
}
