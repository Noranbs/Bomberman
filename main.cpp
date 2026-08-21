#include "sfml/Game.h"

#include <exception>
#include <iostream>

int main()
{
    try {
        bomberman::sfml::Game game;
        game.run();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
