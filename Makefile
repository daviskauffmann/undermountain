CXX 		:= gcc
CXX_FLAGS	:= -Wall -Wextra -Wno-unused-parameter -std=gnu99 -ggdb

BIN			:= bin
SRC			:= src
INCLUDE 	:= include
LIB			:= lib

LIBRARIES	:= -ltcod
EXECUTABLE 	:= main

all: $(BIN)/$(EXECUTABLE)

run: clean all
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/main.c \
					  $(SRC)/platform/config.c \
					  $(SRC)/platform/platform.c \
					  $(SRC)/platform/sys.c \
					  $(SRC)/platform/scenes/scene_about.c \
					  $(SRC)/platform/scenes/scene_game.c \
					  $(SRC)/platform/scenes/scene_menu.c \
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
					  $(SRC)/game/world.c
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
