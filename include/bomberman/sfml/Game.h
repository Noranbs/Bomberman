#ifndef BOMBERMAN_AP_SFML_GAME_H
#define BOMBERMAN_AP_SFML_GAME_H

#include "bomberman/logic/World.h"
#include "bomberman/sfml/SfmlFactory.h"
#include "bomberman/sfml/StateManager.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

namespace bomberman::sfml {

/**
 * @brief SFML application class.
 *
 * This class opens the window, loads assets, and runs the main game loop.
 */
class Game final {
public:
    /**
     * @brief Creates the window, textures, factory, world, and states.
     */
    Game();

    /**
     * @brief Runs the game loop until the window is closed.
     */
    void run();

private:
    /**
     * @brief Handles window and keyboard events.
     */
    void processEvents();

    /**
     * @brief Updates the active state.
     * @param deltaTime Time since the last frame.
     */
    void update(float deltaTime);

    /**
     * @brief Draws the active state.
     */
    void render();

    std::shared_ptr<sf::RenderWindow> window; ///< Main SFML window.
    std::shared_ptr<sf::Texture> texture;     ///< Sprite sheet for characters and items.
    std::shared_ptr<sf::Texture> tileTexture; ///< Sprite sheet for walls and blocks.
    sf::Font font;                            ///< Font used for text.
    std::shared_ptr<SfmlFactory> factory;     ///< Factory that creates entities and views.
    logic::World world;                       ///< Game logic world.
    StateManager stateManager;                ///< Current screen/state manager.
};

}

#endif //BOMBERMAN_AP_SFML_GAME_H
