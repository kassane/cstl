const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addStaticLibrary(.{
        .name = "facil.io",
        .target = target,
        .optimize = optimize,
    });

    // Generate flags
    var flags = std.ArrayList([]const u8).init(b.allocator);
    if (lib.optimize != .Debug)
        lib.strip = true
    else
        lib.bundle_compiler_rt = true;
    try flags.append("-Wno-return-type-c-linkage");
    try flags.append("-fno-sanitize=undefined");

    try flags.append("-DFIO_HTTP_EXACT_LOGGING");
    if (target.getAbi() == .musl)
        try flags.append("-D_LARGEFILE64_SOURCE");

    // Include paths
    lib.addIncludePath(.{ .path = "." });
    lib.addIncludePath(.{ .path = "lib" });
    lib.addIncludePath(.{ .path = "fio-stl" });
    lib.addIncludePath(.{ .path = "extras" });
    lib.addIncludePath(.{ .path = "tests" });
    lib.addIncludePath(.{ .path = "examples" });

    // C source files
    lib.addCSourceFiles(&.{
        "lib/fio.c",
    }, flags.items);

    // link against libc
    lib.linkLibC();
    lib.installHeader("fio-stl.h", "fio-stl.h");

    b.installArtifact(lib);
}
