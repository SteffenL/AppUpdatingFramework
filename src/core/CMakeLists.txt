set(TARGET_NAME aufw_core)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(${TARGET_NAME} ${SOURCES})

if(MSVC)
    target_compile_definitions(${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE UNICODE _UNICODE)
endif()

target_include_directories(${TARGET_NAME} PUBLIC include ../../3rd_party/jsoncpp-0.5.0 PRIVATE ../../3rd_party/nowide)
target_link_libraries(${TARGET_NAME} ${CONAN_LIBS})
