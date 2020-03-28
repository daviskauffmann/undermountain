CC := gcc
CFLAGS := -ggdb -Iextern/libtcod/src -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter `pkg-config --cflags libtcod`
CPPFLAGS :=
LDFLAGS := `pkg-config --libs libtcod`
LDLIBS :=

SRC	:= \
	src/config.c \
	src/main.c \
	src/sys.c \
	src/game/actor.c \
	src/game/assets.c \
	src/game/corpse.c \
	src/game/explosion.c \
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
	src/scenes/scene_game.c \
	src/scenes/scene_menu.c
TARGET := bin/undermountain

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC:src/%.c=obj/%.o)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

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
