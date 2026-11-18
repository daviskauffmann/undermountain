cmake_minimum_required(VERSION 3.15)

project(undermountain LANGUAGES C)

find_package(libtcod CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
if(WIN32)
    find_package(unofficial-pdcurses CONFIG REQUIRED)
    set(TERMINAL_LIBRARIES unofficial::pdcurses::pdcurses)
else()
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)
endif()

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)

if(NOT WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${CURSES_CFLAGS})
    set(TERMINAL_LIBRARIES ${CURSES_LIBRARIES})
endif()

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
    $<TARGET_NAME_IF_EXISTS:SDL3::SDL3main>
    $<IF:$<TARGET_EXISTS:SDL3::SDL3-shared>,SDL3::SDL3-shared,SDL3::SDL3-static>
    ${TERMINAL_LIBRARIES}
)
