LIBTCOD_VERSION=libtcod-1.18.1-x86_64-mingw-DEBUG

CC = gcc
CFLAGS = -ggdb -Iextern/libtcod/src -std=c99 -Wall -Wextra -Wpedantic -Wno-attributes -Wno-deprecated-declarations -Wno-unused-parameter -Wno-unknown-pragmas
CPPFLAGS =
LDFLAGS = -Lextern/libtcod/buildsys/scons/$(LIBTCOD_VERSION)
LDLIBS = -ltcod

SRC	= \
	src/config.c \
	src/main.c \
	src/sys.c \
	src/game/actor.c \
	src/game/assets.c \
	src/game/corpse.c \
	src/game/explosion.c \
	src/game/faction.c \
	src/game/item.c \
	src/game/map.c \
	src/game/message.c \
	src/game/object.c \
	src/game/projectile.c \
	src/game/room.c \
	src/game/tile.c \
	src/game/util.c \
	src/game/world.c \
	src/scenes/scene_about.c \
	src/scenes/scene_create.c \
	src/scenes/scene_game.c \
	src/scenes/scene_menu.c
TARGET = bin/undermountain

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC:src/%.c=obj/%.o)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	cp extern/libtcod/buildsys/scons/$(LIBTCOD_VERSION)/libtcod.dll bin

obj/%.o: src/%.c
	@mkdir -p $(@D)
	@mkdir -p $(@D:obj%=dep%)
	$(CC) -c $< -o $@ -MMD -MF $(@:obj/%.o=dep/%.d) $(CFLAGS) $(CPPFLAGS)

-include $(SRC:src/%.c=dep/%.d)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: clean
clean:
	rm -rf bin obj dep

.PHONY: build_libtcod
build_libtcod:
	cd extern/libtcod/buildsys/scons && scons build TOOLSET=mingw ARCH=x86_64

.PHONY: clean_libtcod
clean_libtcod:
	rm -rf extern/libtcod/buildsys/scons/__pycache__
	rm -rf extern/libtcod/buildsys/scons/dependencies
	rm -rf extern/libtcod/buildsys/scons/$(LIBTCOD_VERSION)
	rm -rf extern/libtcod/buildsys/scons/.sconsign.dblite
