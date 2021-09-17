cmake_minimum_required(VERSION 3.0.0)

message(STATUS ${CMAKE_TOOLCHAIN_FILE})

project(undermountain)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} 	
    src/config.c
    src/main.c
    src/sys.c
    src/game/actor.c
    src/game/assets.c
    src/game/corpse.c
    src/game/explosion.c
    src/game/faction.c
    src/game/item.c
    src/game/map.c
    src/game/message.c
    src/game/object.c
    src/game/projectile.c
    src/game/room.c
    src/game/tile.c
    src/game/util.c
    src/game/world.c
    src/scenes/scene_about.c
    src/scenes/scene_create.c
    src/scenes/scene_game.c
    src/scenes/scene_menu.c)

find_package(libtcod CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libtcod::libtcod)
# find_package(libtcod REQUIRED)
# target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIRS})
# target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
