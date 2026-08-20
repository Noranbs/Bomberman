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

/**
 * @brief Base class for one game screen.
 */
class State {
public:
    /**
     * @brief Creates a state that can access the state manager.
     * @param manager Owner state manager.
     */
    explicit State(StateManager& manager);
    virtual ~State() = default;

    /**
     * @brief Handles SFML input for this state.
     */
    virtual void processEvent(const sf::Event& event) = 0;

    /**
     * @brief Updates this state.
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Draws this state.
     */
    virtual void render() = 0;

protected:
    StateManager& manager; ///< State manager that owns this state.
};

/**
 * @brief Screens that the game can show.
 */
enum class StateId {
    Menu,         ///< Start menu.
    Instructions, ///< Instructions screen.
    Playing,      ///< Active gameplay.
    GameOver,     ///< Game-over screen.
    Victory       ///< Level-clear or game-complete screen.
};

/**
 * @brief Switches between menu, playing, game-over, and victory states.
 */
class StateManager final {
public:
    /**
     * @brief Creates the state manager and starts in the menu.
     * @param window Window used for drawing.
     * @param font Font used for text.
     * @param factory Factory used for drawing entity views.
     * @param world Logic world controlled by the states.
     */
    StateManager(std::shared_ptr<sf::RenderWindow> window,
                 const sf::Font& font,
                 std::shared_ptr<SfmlFactory> factory,
                 logic::World& world);

    /**
     * @brief Sends an SFML event to the current state.
     * @param event Input/window event.
     */
    void processEvent(const sf::Event& event);

    /**
     * @brief Updates the current state.
     * @param deltaTime Time since the last frame.
     */
    void update(float deltaTime);

    /**
     * @brief Draws the current state.
     */
    void render();

    /**
     * @brief Changes to another state.
     * @param stateId State to switch to.
     */
    void transitionTo(StateId stateId);

    /**
     * @brief Starts the next level after a victory screen.
     */
    void continueToNextLevel();

    std::shared_ptr<sf::RenderWindow> window() const;
    const sf::Font& font() const;
    std::shared_ptr<SfmlFactory> factory() const;
    logic::World& world() const;

private:
    std::shared_ptr<sf::RenderWindow> renderWindow; ///< Window used by all states.
    const sf::Font& fontRef;                        ///< Font used by all states.
    std::shared_ptr<SfmlFactory> entityFactory;     ///< Factory with drawable entity views.
    logic::World& gameWorld;                        ///< Logic world used by the states.
    std::unique_ptr<State> currentState;            ///< Current active state.
};

}

#endif //BOMBERMAN_AP_SFML_STATE_MANAGER_H
