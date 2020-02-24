CC := gcc
CFLAGS := -ggdb -Iextern/libtcod/src -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
CPPFLAGS :=
LDFLAGS := -Lextern/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG
LDLIBS := -ltcod

SRC	:= \
	src/main.c \
	src/game/actor.c \
	src/game/assets.c \
	src/game/corpse.c \
	src/game/item.c \
	src/game/map.c \
	src/game/message.c \
	src/game/object.c \
	src/game/projectile.c \
	src/game/room.c \
	src/game/tile.c \
	src/game/util.c \
	src/game/world.c \
	src/platform/config.c \
	src/platform/platform.c \
	src/platform/sys.c \
	src/platform/scenes/scene_about.c \
	src/platform/scenes/scene_game.c \
	src/platform/scenes/scene_menu.c
TARGET := bin/undermountain

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC:src/%.c=obj/%.o)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	cp extern/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG/libtcod.dll bin

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
	rm -rf extern/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG
	rm -rf extern/libtcod/buildsys/scons/.sconsign.dblite
