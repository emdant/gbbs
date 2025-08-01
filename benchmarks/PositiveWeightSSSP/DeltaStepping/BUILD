licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "DeltaStepping",
    hdrs = ["DeltaStepping.h"],
    deps = [
        "//gbbs",
        "//gbbs:bucket",
    ],
)

cc_binary(
    name = "DeltaStepping_main-int32",
    srcs = ["DeltaStepping.cc"],
    deps = [":DeltaStepping"],
)

cc_binary(
    name = "DeltaStepping_main-float",
    srcs = ["DeltaStepping.cc"],
    deps = [":DeltaStepping"],
    copts=["-DUSE_FLOAT"]
)

filegroup(
    name = "DeltaStepping_main",
    srcs = [
        ":DeltaStepping_main-int32",
        ":DeltaStepping_main-float",
    ],
)

