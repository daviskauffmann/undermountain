CC := gcc
CFLAGS := -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter

BIN	:= bin
SRC	:= src
BUILD := build
INCLUDE := include
LIB	:= lib

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
OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(SOURCES:.c=.o))
DEPENDENCIES := $(OBJECTS:.o=.d)
LIBRARIES := -ltcod
EXECUTABLE := main

.PHONY: all
all: $(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ -L$(LIB) $(LIBRARIES)

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ -MMD -MF $(@:.o=.d) $(CFLAGS) -I$(INCLUDE)

-include $(DEPENDENCIES)

.PHONY: run
run: all
	./$(BIN)/$(EXECUTABLE)

.PHONY: clean
clean:
	rm -rf $(BIN) $(BUILD)
