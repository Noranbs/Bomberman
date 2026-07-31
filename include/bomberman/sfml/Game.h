#ifndef BOMBERMAN_AP_SFML_GAME_H
#define BOMBERMAN_AP_SFML_GAME_H

#include "bomberman/logic/World.h"
#include "bomberman/sfml/SfmlFactory.h"
#include "bomberman/sfml/StateManager.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

namespace bomberman::sfml {

class Game final {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();

    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<sf::Texture> texture;
    sf::Font font;
    std::shared_ptr<SfmlFactory> factory;
    logic::World world;
    StateManager stateManager;
};

}

#endif //BOMBERMAN_AP_SFML_GAME_H
