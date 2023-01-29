cmake_minimum_required(VERSION 3.0.0)

project(undermountain LANGUAGES C)

find_package(libtcod CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

file(
    GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS
    SOURCES ${PROJECT_SOURCE_DIR}/src/*.c
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    libtcod::libtcod
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
