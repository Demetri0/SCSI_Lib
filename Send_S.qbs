import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    cpp.cxxLanguageVersion: "c++11"
    consoleApplication: true

    files: [
        "*.h",
        "*.cpp"
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}

