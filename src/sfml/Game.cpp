#include "bomberman/sfml/Game.h"
#include "bomberman/logic/Stopwatch.h"
#include <SFML/Window/Event.hpp>
#include <stdexcept>

namespace bomberman::sfml {

Game::Game()
    : window(std::make_shared<sf::RenderWindow>(sf::VideoMode(960, 832), "Bomberman AP")),
      texture(std::make_shared<sf::Texture>()),
      factory(std::make_shared<SfmlFactory>(window, texture)),
      world(factory),
      stateManager(window, font, factory, world)
{
    window->setFramerateLimit(60);

    if (!texture->loadFromFile("assets/bomberman.png")) {
        throw std::runtime_error("Unable to load assets/bomberman.png");
    }
    if (!font.loadFromFile("assets/LiberationSans-Bold.ttf")) {
        throw std::runtime_error("Unable to load assets/LiberationSans-Bold.ttf");
    }
}

void Game::run()
{
    logic::Stopwatch::instance().reset();
    while (window->isOpen()) {
        processEvents();
        update(logic::Stopwatch::instance().updateFrameTime());
        render();
    }
}

void Game::processEvents()
{
    sf::Event event{};
    while (window->pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window->close();
        }

        stateManager.processEvent(event);
    }
}

void Game::update(float deltaTime)
{
    stateManager.update(deltaTime);
}

void Game::render()
{
    window->clear(sf::Color(29, 37, 45));
    stateManager.render();
    window->display();
}

}
