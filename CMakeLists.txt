cmake_minimum_required(VERSION 3.10)

project(ccabral
    VERSION 1.0.0
    DESCRIPTION "C library for parser implementation"
    LANGUAGES C
)

option(CCB_BUILD_EXAMPLE "Build the example executable" OFF)
option(CCB_BUILD_TESTING "Build the testing tree" OFF)

# Disable testing in external dependencies
set(CLN_BUILD_TESTING OFF CACHE BOOL "Build clinschoten tests" FORCE)

add_subdirectory(external/cbarroso)
add_subdirectory(external/clinschoten)

add_library(ccabral STATIC)

target_sources(ccabral
    PRIVATE src/_first.c
    PRIVATE src/_follow.c
    PRIVATE src/_frstfllw.c
    PRIVATE src/_grmrdata.c
    PRIVATE src/_prdcdata.c
    PRIVATE src/_prdcprsntble.c
    PRIVATE src/_prdsmap.c
    PRIVATE src/_prsrstck.c
    PRIVATE src/parser.c
    PRIVATE src/prdcdata.c
    PRIVATE src/prdsmap.c
    PRIVATE src/tknsq.c
)

target_include_directories(ccabral
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ccabral
    PUBLIC cbarroso::cbarroso
)
target_link_libraries(ccabral
    PUBLIC CLN::clinschoten
)

target_compile_features(ccabral PUBLIC c_std_99)

add_library(ccabral::ccabral ALIAS ccabral)

include(GNUInstallDirs)

install(TARGETS ccabral
    EXPORT ccabralTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ccabralTargets
    FILE ccabralTargets.cmake
    NAMESPACE ccabral::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ccabral
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ccabralConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ccabralConfig.cmake"
"include(CMakeFindDependencyMacro)
find_dependency(cbarroso)
include(\"\${CMAKE_CURRENT_LIST_DIR}/ccabralTargets.cmake\")
"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ccabralConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ccabralConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ccabral
)

if(CCB_BUILD_EXAMPLE)
    add_executable(ccabral_example example/main.c)
    target_compile_definitions(ccabral PRIVATE
        CCB_NUM_OF_PRODUCTIONS=3
        CCB_NUM_OF_NONTERMINALS=1
        CCB_NUM_OF_TERMINALS=5
    )
    target_link_libraries(ccabral_example PRIVATE ccabral::ccabral)
endif()

if(CCB_BUILD_TESTING)
    enable_testing()

    add_executable(test_runner 
        tests/test_runner.c
        tests/test_tknsq.c
        tests/test_prsrstck.c
        tests/test_auxds.c
        tests/test_parser.c
    )
    
    target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/ccauchy/include)
    target_link_libraries(test_runner PRIVATE ccabral::ccabral)
    
    # Define test grammar constants to enable parser tests
    target_compile_definitions(test_runner PRIVATE
        CCB_NUM_OF_PRODUCTIONS=1
        CCB_NUM_OF_NONTERMINALS=1
        CCB_NUM_OF_TERMINALS=2
    )
    
    add_test(NAME AllTests COMMAND test_runner)
endif()
