#ifndef BOMBERMAN_AP_SFML_STATE_MANAGER_H
#define BOMBERMAN_AP_SFML_STATE_MANAGER_H

#include "bomberman/logic/World.h"
#include "bomberman/sfml/SfmlFactory.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

namespace bomberman::sfml {

class StateManager;

class State {
public:
    explicit State(StateManager& manager);
    virtual ~State() = default;

    virtual void processEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;

protected:
    StateManager& manager;
};

enum class StateId {
    Menu,
    Playing,
    GameOver,
    Victory
};

class StateManager final {
public:
    StateManager(std::shared_ptr<sf::RenderWindow> window,
                 const sf::Font& font,
                 std::shared_ptr<SfmlFactory> factory,
                 logic::World& world);

    void processEvent(const sf::Event& event);
    void update(float deltaTime);
    void render();
    void transitionTo(StateId stateId);

    std::shared_ptr<sf::RenderWindow> window() const;
    const sf::Font& font() const;
    std::shared_ptr<SfmlFactory> factory() const;
    logic::World& world() const;

private:
    std::shared_ptr<sf::RenderWindow> renderWindow;
    const sf::Font& fontRef;
    std::shared_ptr<SfmlFactory> entityFactory;
    logic::World& gameWorld;
    std::unique_ptr<State> currentState;
};

}

#endif //BOMBERMAN_AP_SFML_STATE_MANAGER_H
