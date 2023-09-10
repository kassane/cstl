cmake_minimum_required(VERSION 3.12)

project(facil.io VERSION 0.8.0)

# Set the source directories
set(LIB_ROOT lib)
set(LIB_CONCAT_FOLDER fio-stl)

include_directories(".")

option(ENABLE_SHARED "Build facil.io as shared library" OFF)
option(EXAMPLES_BUILD "Build Examples" OFF)
option(TESTS_BUILD "Build Tests" OFF)

# Detect endianess
include(CheckTypeSize)
check_type_size("void*" SIZE_OF_POINTER)
if(${SIZE_OF_POINTER} EQUAL 8)
    set(FLAGS "__BIG_ENDIAN__")
else()
    set(FLAGS "__BIG_ENDIAN__=0")
endif()

find_package(Threads REQUIRED)
# Detect SSL/TLS Libraries
find_package(OpenSSL 3.0)

if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found.")
else()
    message(WARNING "OpenSSL not found. SSL/TLS support will be disabled.")
endif()

# Detect Sodium Library
find_package(Sodium)

if(Sodium_FOUND)
    message(STATUS "Sodium found.")
else()
    message(WARNING "Sodium not found. Sodium support will be disabled.")
endif()

# Detect 'struct tm' fields
include(CheckStructHasMember)
check_struct_has_member("struct tm" tm_zone time.h HAVE_TM_TM_ZONE)

# Detect SystemV socket libraries
include(CheckLibraryExists)
check_library_exists(socket connect "" HAVE_SOCKET)
check_library_exists(nsl gethostname "" HAVE_NSL)

if(HAVE_SOCKET AND HAVE_NSL)
    link_libraries(socket nsl)
endif()
if(WIN32)
    link_libraries(ws2_32)
endif()

# Variable to track whether any of the checks have succeeded
set(HAVE_SENDFILE FALSE)

# Detect the `sendfile` system call for Linux
if(NOT HAVE_SENDFILE)
    include(CheckCSourceCompiles)
    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <stdlib.h>
        #include <stdio.h>
        #include <sys/sendfile.h>
        int main(void) {
            off_t offset = 0;
            ssize_t result = sendfile(2, 1, (off_t *)&offset, 300);
        }
    "        HAVE_SENDFILE_LINUX)

    if(HAVE_SENDFILE_LINUX)
        set(FLAGS "${FLAGS} USE_SENDFILE_LINUX HAVE_SENDFILE")
        set(HAVE_SENDFILE TRUE)
    endif()
endif()

# Detect the `sendfile` system call for BSD
if(NOT HAVE_SENDFILE)
    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <stdlib.h>
        #include <stdio.h>
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <sys/uio.h>
        int main(void) {
            off_t sent = 0;
            off_t offset = 0;
            ssize_t result = sendfile(2, 1, offset, (size_t)sent, NULL, &sent, 0);
        }
    "        HAVE_SENDFILE_BSD)

    if(HAVE_SENDFILE_BSD)
        set(FLAGS "${FLAGS} USE_SENDFILE_BSD HAVE_SENDFILE")
        set(HAVE_SENDFILE TRUE)
    endif()
endif()

# Detect the `sendfile` system call for Apple
if(NOT HAVE_SENDFILE)
    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <stdlib.h>
        #include <stdio.h>
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <sys/uio.h>
        int main(void) {
            off_t sent = 0;
            off_t offset = 0;
            ssize_t result = sendfile(2, 1, offset, &sent, NULL, 0);
        }
    "        HAVE_SENDFILE_APPLE)

    if(HAVE_SENDFILE_APPLE)
        set(FLAGS "${FLAGS} USE_SENDFILE_APPLE HAVE_SENDFILE")
        set(HAVE_SENDFILE TRUE)
    endif()
endif()

# Detect IOCP support for Windows
if(WIN32)
    if(NOT HAVE_IOCP)
        check_c_source_compiles("
        #include <winsock2.h>
        #include <windows.h>
        int main(void) {
            HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
            if (iocp == NULL) {
                return 1;
            }
            CloseHandle(iocp);
            return 0;
        }
    "            HAVE_IOCP)

        if(HAVE_IOCP)
            set(FLAGS "${FLAGS} FIO_ENGINE_IOCP HAVE_IOCP")
        else()
            message(FATAL_ERROR "No supported polling engine detected. Unable to compile facil.io.")
        endif()
    endif()
else()
    # Detect polling mechanisms (kqueue / epoll / poll)
    if(NOT HAVE_KQUEUE AND NOT HAVE_EPOLL AND NOT HAVE_POLL)
        check_c_source_compiles("
        #define _GNU_SOURCE
        #include <stdlib.h>
        #include <sys/event.h>
        int main(void) {
            int fd = kqueue();
        }
    "            HAVE_KQUEUE)

        if(HAVE_KQUEUE)
            set(FLAGS "${FLAGS} FIO_ENGINE_KQUEUE HAVE_KQUEUE")
        else()
            check_c_source_compiles("
            #define _GNU_SOURCE
            #include <stdlib.h>
            #include <stdio.h>
            #include <sys/types.h>
            #include <sys/stat.h>
            #include <fcntl.h>
            #include <sys/epoll.h>
            int main(void) {
                int fd = epoll_create1(EPOLL_CLOEXEC);
            }
        "                HAVE_EPOLL)

            if(HAVE_EPOLL)
                set(FLAGS "${FLAGS} FIO_ENGINE_EPOLL HAVE_EPOLL")
            else()
                check_c_source_compiles("
                #define _GNU_SOURCE
                #include <stdlib.h>
                #include <poll.h>
                int main(void) {
                    struct pollfd plist[18];
                    memset(plist, 0, sizeof(plist[0]) * 18);
                    poll(plist, 1, 1);
                }
            "                    HAVE_POLL)

                if(HAVE_POLL)
                    set(FLAGS "${FLAGS} FIO_ENGINE_POLL HAVE_POLL")
                else()
                    message(FATAL_ERROR "No supported polling engine detected. Unable to compile facil.io.")
                endif()
            endif()
        endif()
    endif()
endif()


# Define the source files for the library
file(GLOB_RECURSE LIB_SOURCES ${LIB_ROOT}/*.c ${LIB_ROOT}/*.cpp ${LIB_ROOT}/*.cxx ${LIB_ROOT}/*.c++)
set(SOURCES ${LIB_SOURCES})

# Optimization level
set(OPTIMIZATION "-O3")
# Optimization level in debug mode
set(OPTIMIZATION_DEBUG "-O0 -g -coverage -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-builtin")
# Warning flags
set(WARNINGS "-Wno-missing-field-initializers -Wformat-security")
# Preprocessor definitions
set(FLAGS "-DFIO_LEAK_COUNTER -DFIO_FIOBJ -DFIOBJ_MALLOC")
if(WIN32 AND NOT MSVC)
    set(FLAGS "${FLAGS} -D_GNU_SOURCE -D__MINGW32__")
endif()
# C specific compiler options
set(C_EXTRA_OPT "")
# C++ specific compiler options
set(CXX_EXTRA_OPT "-Wno-keyword-macro -Wno-vla-extension -Wno-c99-extensions -Wno-zero-length-array -Wno-variadic-macros -Wno-missing-braces")

# Create the library target
if(ENABLE_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    if(OpenSSL_FOUND)
        target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    elseif(Sodium_FOUND)
        target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads Sodium::Sodium)
    else()
        target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)
    endif()
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ".")
# Add the definitions to the target
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${OPTIMIZATION_DEBUG} ${WARNINGS} ${CXX_EXTRA_OPT}> "-DDEBUG=1"
    $<$<NOT:$<CONFIG:Debug>>:${OPTIMIZATION} ${WARNINGS}>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${FLAGS})

set(PKG_CONFIG_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc")
configure_file(${PROJECT_NAME}.pc.in ${PKG_CONFIG_FILE} @ONLY)

# Install the generated .pc file
install(FILES ${PKG_CONFIG_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

if(EXAMPLES_BUILD)
    # Define a function to build and run an example
    function(build_and_run_example EXAMPLE_NAME)
        add_executable(${EXAMPLE_NAME} examples/${EXAMPLE_NAME}.c)
        if(${EXAMPLE_NAME} STREQUAL server OR ${EXAMPLE_NAME} STREQUAL client)
            if(Sodium_FOUND)
                target_link_libraries(${EXAMPLE_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads Sodium::Sodium)
            elseif(OpenSSL_FOUND)
                target_link_libraries(${EXAMPLE_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
            else()
                target_link_libraries(${EXAMPLE_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads)
            endif()
        else()
            target_link_libraries(${EXAMPLE_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads)
        endif()
        target_compile_definitions(${EXAMPLE_NAME} PRIVATE ${FLAGS})
        add_custom_target(run_${EXAMPLE_NAME}
            COMMAND ${EXAMPLE_NAME}
            DEPENDS ${EXAMPLE_NAME}
        )
    endfunction()

    # List of example files to build and run
    # comment all examples w/ issue build
    set(EXAMPLES
        array
        bstr
        chat
        client
        # fiobj
        map
        server
        string
    )

    # Build and run each example
    foreach(EXAMPLE ${EXAMPLES})
        build_and_run_example(${EXAMPLE})
    endforeach()
endif()

if(TESTS_BUILD)
    # Define a function to build and run a test
    function(build_and_run_test TEST_NAME)
        if(${TEST_NAME} STREQUAL cpp)
            add_executable(${TEST_NAME} tests/${TEST_NAME}.cpp)
        else()
            add_executable(${TEST_NAME} tests/${TEST_NAME}.c)
        endif()
        if(${TEST_NAME} STREQUAL base64 OR ${TEST_NAME} STREQUAL stl-mutex)
            if(Sodium_FOUND)
                target_link_libraries(${TEST_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads Sodium::Sodium)
            elseif(OpenSSL_FOUND)
                target_link_libraries(${TEST_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
            endif()
        else()
            target_link_libraries(${TEST_NAME} PUBLIC ${PROJECT_NAME} PRIVATE Threads::Threads)
        endif()
        target_compile_definitions(${TEST_NAME} PRIVATE ${FLAGS} "-DTESTS=1")
        add_custom_target(run_${TEST_NAME}
            COMMAND ${TEST_NAME}
            DEPENDS ${TEST_NAME}
        )
    endfunction()

    # List of test files to build and run
    # comment all tests w/ issue build
    set(TESTS
        # array
        base64
        # cpp
        http1-parser-old
        http1-parser
        # json
        # json_find
        # json_minify
        malloc
        mempool
        # mustache
        noop
        # random
        slowloris
        # stl-mutex
        # stl
        url
    )

    # Build and run each test
    foreach(TEST ${TESTS})
        build_and_run_test(${TEST})
    endforeach()
endif()
