#include "engine.h"
#include "game.h"

int main(int argc, char *argv[])
{
    engine_init();
    game_run();
    engine_quit();

    return 0;
}