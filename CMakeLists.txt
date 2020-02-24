cmake_minimum_required(VERSION 3.11)
project(cifer C)

# C compiler settings
set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Wredundant-decls \
-Wnested-externs -Wmissing-include-dirs -Wstrict-prototypes -Wwrite-strings \
-Wno-unused-parameter -Wshadow -Wold-style-definition")

# GCC warnings that Clang doesn't provide
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wjump-misses-init -Wlogical-op")
endif ()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# Header files
include_directories(
        include
        external/munit
        external/uthash
        )

# Library sources
set(library_SOURCES
        src/data/mat.c
        src/data/mat_curve.c
        src/data/vec.c
        src/data/vec_float.c
        src/data/vec_curve.c
        src/internal/big.c
        src/internal/common.c
        src/internal/dlog.c
        src/internal/hash.c
        src/internal/keygen.c
        src/internal/prime.c
        src/internal/str.c
        src/innerprod/simple/ddh.c
        src/innerprod/simple/ddh_multi.c
        src/innerprod/simple/lwe.c
        src/innerprod/simple/ring_lwe.c
        src/innerprod/fullysec/damgard.c
        src/innerprod/fullysec/damgard_multi.c
        src/innerprod/fullysec/lwe_fs.c
        src/innerprod/fullysec/paillier.c
        src/innerprod/fullysec/dmcfe.c
        src/innerprod/fullysec/damgard_dec_multi.c
        src/innerprod/fullysec/fhipe.c
        src/innerprod/fullysec/fh_multi_ipe.c
        src/sample/normal.c
        src/sample/normal_cumulative.c
        src/sample/normal_double.c
        src/sample/normal_double_constant.c
        src/sample/normal_cdt.c
        src/sample/normal_negative.c
        src/sample/uniform.c
        src/abe/policy.c
        src/abe/gpsw.c
        src/abe/fame.c
        src/abe/dippe.c
        src/quadratic/sgp.c
        )

add_library(cifer SHARED ${library_SOURCES})

# Link libraries that are used in our library
target_link_libraries(cifer gmp sodium m amcl)

# Install library and copy header to install dir
install(TARGETS cifer DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# Create an executable
set(binary_SOURCES
        test/test.c
        test/data/mat.c
        test/data/vec.c
        test/internal/dlog.c
        test/internal/keygen.c
        test/internal/prime.c
        test/internal/str.c
        test/internal/big.c
        test/innerprod/simple/ddh.c
        test/innerprod/simple/ddh_multi.c
        test/innerprod/simple/lwe.c
        test/innerprod/simple/ring_lwe.c
        test/innerprod/fullysec/damgard.c
        test/innerprod/fullysec/damgard_multi.c
        test/innerprod/fullysec/lwe_fs.c
        test/innerprod/fullysec/paillier.c
        test/innerprod/fullysec/dmcfe.c
        test/innerprod/fullysec/damgard_dec_multi.c
        test/innerprod/fullysec/fhipe.c
        test/innerprod/fullysec/fh_multi_ipe.c
        test/sample/normal.c
        test/sample/normal_cumulative.c
        test/sample/normal_double.c
        test/sample/normal_double_constant.c
        test/sample/normal_cdt.c
        test/sample/normal_negative.c
        test/sample/uniform.c
        test/abe/policy.c
        test/abe/gpsw.c
        test/abe/fame.c
        test/abe/dippe.c
        external/munit/munit.c
        test/quadratic/sgp.c
        )

add_executable(cifer_test ${binary_SOURCES})

target_link_libraries(cifer_test PRIVATE cifer)

add_custom_target(test COMMAND cifer_test VERBATIM)

add_custom_target(docs COMMAND doxygen WORKING_DIRECTORY .. VERBATIM)
