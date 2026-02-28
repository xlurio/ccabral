cmake_minimum_required(VERSION 3.10)

project(ccabral
    VERSION 1.0.1
    DESCRIPTION "C library for parser implementation"
    LANGUAGES C
)

option(CCB_BUILD_EXAMPLES "Build the example executable" OFF)
option(CCB_BUILD_TESTING "Build the testing tree" OFF)

# Disable testing in external dependencies
set(CLN_BUILD_TESTING OFF CACHE BOOL "Build clinschoten tests" FORCE)

add_subdirectory(external/cbarroso)
add_subdirectory(external/clinschoten)

add_library(ccabral STATIC)

set(CCABRAL_SOURCES_LIST
    ${PROJECT_SOURCE_DIR}/src/_first.c
    ${PROJECT_SOURCE_DIR}/src/_follow.c
    ${PROJECT_SOURCE_DIR}/src/_frstfllw.c
    ${PROJECT_SOURCE_DIR}/src/_grmrdata.c
    ${PROJECT_SOURCE_DIR}/src/_prdcdata.c
    ${PROJECT_SOURCE_DIR}/src/_prdcprsntble.c
    ${PROJECT_SOURCE_DIR}/src/_prdsmap.c
    ${PROJECT_SOURCE_DIR}/src/_prsrstck.c
    ${PROJECT_SOURCE_DIR}/src/parser.c
    ${PROJECT_SOURCE_DIR}/src/prdcdata.c
    ${PROJECT_SOURCE_DIR}/src/prdsmap.c
    ${PROJECT_SOURCE_DIR}/src/tknsq.c
)

target_sources(ccabral
    PRIVATE
        ${CCABRAL_SOURCES_LIST}
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

if(CCB_BUILD_EXAMPLES)
    add_executable(ll1_example example/ll1_example.c ${CCABRAL_SOURCES_LIST})
    target_include_directories(ll1_example
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(ll1_example PRIVATE cbarroso::cbarroso CLN::clinschoten)
    target_compile_definitions(ll1_example PRIVATE CCB_NUM_OF_PRODUCTIONS=3)
    target_compile_definitions(ll1_example PRIVATE CCB_NUM_OF_NONTERMINALS=1)
    target_compile_definitions(ll1_example PRIVATE CCB_NUM_OF_TERMINALS=5)

    add_executable(ll2_example example/ll2_example.c ${CCABRAL_SOURCES_LIST})
    target_include_directories(ll2_example
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(ll2_example PRIVATE cbarroso::cbarroso CLN::clinschoten)
    target_compile_definitions(ll2_example PRIVATE CCB_NUM_OF_PRODUCTIONS=5)
    target_compile_definitions(ll2_example PRIVATE CCB_NUM_OF_NONTERMINALS=3)
    target_compile_definitions(ll2_example PRIVATE CCB_NUM_OF_TERMINALS=4)
endif()

if(CCB_BUILD_TESTING)
    enable_testing()

    add_executable(test_runner 
        tests/test_runner.c
        tests/test_tknsq.c
        tests/test_prsrstck.c
        tests/test_auxds.c
        tests/test_parser.c
        ${CCABRAL_SOURCES_LIST}
    )
    
    target_include_directories(test_runner PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/ccauchy/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_link_libraries(test_runner PRIVATE cbarroso::cbarroso CLN::clinschoten)
    
    # Define test grammar constants — applied to all sources including ccabral's
    target_compile_definitions(test_runner PRIVATE
        CCB_NUM_OF_PRODUCTIONS=1
        CCB_NUM_OF_NONTERMINALS=1
        CCB_NUM_OF_TERMINALS=256
    )
    
    add_test(NAME AllTests COMMAND test_runner)
endif()
