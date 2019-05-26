set(TARGET_NAME check_progress)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB_RECURSE SOURCES *.cpp)

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} aufw_core ${CONAN_LIBS})
