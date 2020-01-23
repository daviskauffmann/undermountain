CC := gcc
CFLAGS := -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LDFLAGS :=

SRC	:= src
EXTERN := extern
BUILD := build
BIN	:= bin

SOURCES	:= \
	$(SRC)/main.c \
	$(SRC)/game/actor.c \
	$(SRC)/game/assets.c \
	$(SRC)/game/item.c \
	$(SRC)/game/map.c \
	$(SRC)/game/message.c \
	$(SRC)/game/object.c \
	$(SRC)/game/projectile.c \
	$(SRC)/game/room.c \
	$(SRC)/game/tile.c \
	$(SRC)/game/util.c \
	$(SRC)/game/world.c \
	$(SRC)/platform/config.c \
	$(SRC)/platform/platform.c \
	$(SRC)/platform/sys.c \
	$(SRC)/platform/scenes/scene_about.c \
	$(SRC)/platform/scenes/scene_game.c \
	$(SRC)/platform/scenes/scene_menu.c
OBJECTS := $(SOURCES:$(SRC)/%.c=$(BUILD)/%.o)
DEPENDENCIES := $(OBJECTS:%.o=%.d)
INCLUDE := -I$(EXTERN)/libtcod/src
LIB := -L$(EXTERN)/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG
LIBRARIES := -ltcod
TARGET := $(BIN)/undermountain

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIB) $(LIBRARIES)
	cp $(EXTERN)/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG/libtcod.dll $(BIN)

$(BUILD)/%.o: $(SRC)/%.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ -MMD -MF $(@:.o=.d) $(CFLAGS) $(INCLUDE)

-include $(DEPENDENCIES)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: clean
clean:
	rm -rf $(BIN) $(BUILD)

.PHONY: build_libtcod
build_libtcod:
	cd $(EXTERN)/libtcod/buildsys/scons && scons build TOOLSET=mingw ARCH=x86_64

.PHONY: clean_libtcod
clean_libtcod:
	rm -rf $(EXTERN)/libtcod/buildsys/scons/__pycache__
	rm -rf $(EXTERN)/libtcod/buildsys/scons/dependencies
	rm -rf $(EXTERN)/libtcod/buildsys/scons/libtcod-1.15.1-x86_64-mingw-DEBUG
	rm -rf $(EXTERN)/libtcod/buildsys/scons/.sconsign.dblite
