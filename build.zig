const std = @import("std");

pub fn build(b: *std.Build) void
{
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const storm = b.dependency("storm", .{});
    const stormlib = storm.artifact("storm");

    const exe = b.addExecutable(.{
        .name = "mpqlite",
        .target = target,
        .optimize = optimize,
    });
    exe.addCSourceFiles(.{
        .files = &[_][]const u8 {
            "src/main.c",
        },
        .flags = &[_][]const u8 {},
    });
    exe.defineCMacro("APP_LINUX", null);
    exe.addIncludePath(.{.path = "libs/stb_sprintf"});
    exe.linkLibrary(stormlib);
    exe.linkLibCpp();
    if (b.host.target.os.tag == .windows) {
        exe.linkSystemLibrary("Wininet");
    }
    b.installArtifact(exe);

    // const main_tests = b.addTest(.{
    //     .root_source_file = .{ .path = "src/main.zig" },
    //     .target = target,
    //     .optimize = optimize,
    // });

    // const run_main_tests = b.addRunArtifact(main_tests);

    // const test_step = b.step("test", "Run library tests");
    // test_step.dependOn(&run_main_tests.step);
}
