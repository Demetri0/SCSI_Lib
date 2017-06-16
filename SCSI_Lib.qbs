import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    cpp.cxxLanguageVersion: "c++11"
    consoleApplication: true

    files: [
        "main.cpp",
        "myfs.cpp",
        "myfs.h",
        "network.cpp",
        "network.h",
        "sgdevice.h",
        "sgdevice.cpp",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }

    cpp.dynamicLibraries: "pthread"
}

