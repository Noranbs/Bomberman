#include "sfml/StateManager.h"
#include "logic/Stopwatch.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace bomberman::sfml {

namespace state {

const sf::FloatRect playButtonBounds{260.0F, 620.0F, 200.0F, 64.0F};
const sf::FloatRect instructionsButtonBounds{500.0F, 620.0F, 200.0F, 64.0F};
const sf::FloatRect instructionsBackButtonBounds{380.0F, 754.0F, 200.0F, 54.0F};
const sf::FloatRect playAgainButtonBounds{260.0F, 620.0F, 200.0F, 64.0F};
const sf::FloatRect menuButtonBounds{500.0F, 620.0F, 200.0F, 64.0F};

const logic::MoveLeftCommand moveLeftCommand{};
const logic::MoveRightCommand moveRightCommand{};
const logic::MoveUpCommand moveUpCommand{};
const logic::MoveDownCommand moveDownCommand{};
const logic::StopHorizontalCommand stopHorizontalCommand{};
const logic::StopVerticalCommand stopVerticalCommand{};
const logic::PlaceBombCommand placeBombCommand{};
const logic::KickBombCommand kickBombCommand{};

void executePlayerCommand(StateManager& manager, const logic::PlayerCommand& command, bool active = true)
{
    command.execute(manager.world(), active);
}

void drawButton(sf::RenderWindow& window,
                const sf::Font& font,
                const sf::FloatRect& bounds,
                const std::string& label,
                float textX)
{
    sf::RectangleShape playButton({bounds.width, bounds.height});
    playButton.setPosition(bounds.left, bounds.top);
    playButton.setFillColor(sf::Color(218, 76, 64));
    window.draw(playButton);

    sf::Text play(label, font, 28);
    play.setPosition(textX, bounds.top + 14.0F);
    window.draw(play);
}

void renderHud(StateManager& manager)
{
    sf::RectangleShape hud({936.0F, 42.0F});
    hud.setPosition(12.0F, 10.0F);
    hud.setFillColor(sf::Color(20, 26, 32, 210));
    manager.window()->draw(hud);

    const int totalSeconds = static_cast<int>(manager.world().elapsedTime());
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    const auto player = manager.world().player();

    std::ostringstream hudText;
    hudText << "Level: " << manager.world().currentLevel()
            << "   Score: " << manager.world().score()->getCurrentScore()
            << "   Time: " << std::setfill('0') << std::setw(2) << minutes << ':'
            << std::setw(2) << seconds << std::setfill(' ');

    if (player != nullptr) {
        hudText << "   Lives: " << manager.world().playerLives()
                << "   Bombs: " << player->getAvailableBombs() << '/' << player->getBombCapacity()
                << "   Fire: " << player->getBombRadius()
                << "   Speed: " << std::fixed << std::setprecision(1) << player->getSpeedMultiplier() << 'x';
    }

    hudText << "   Enemies: " << manager.world().enemiesAlive();

    sf::Text text(hudText.str(), manager.font(), 20);
    text.setPosition(24.0F, 19.0F);
    manager.window()->draw(text);
}

/**
 * @brief Main menu state with play, instructions, and high scores.
 */
class MenuState final : public State {
public:
    using State::State;

    void processEvent(const sf::Event& event) override
    {
        if (event.type == sf::Event::MouseButtonPressed) {
            const auto mouse = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                           static_cast<float>(event.mouseButton.y));
            if (playButtonBounds.contains(mouse)) {
                manager.transitionTo(StateId::Playing);
            } else if (instructionsButtonBounds.contains(mouse)) {
                manager.transitionTo(StateId::Instructions);
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            manager.transitionTo(StateId::Playing);
        }

    }

    void update(float) override {}

    void render() override
    {
        sf::Text title("Bomberman", manager.font(), 64);
        title.setPosition(286.0F, 90.0F);
        manager.window()->draw(title);

        sf::Text heading("Top scores", manager.font(), 32);
        heading.setPosition(392.0F, 220.0F);
        manager.window()->draw(heading);

        const auto& highScores = manager.world().score()->getHighScores();
        for (std::size_t index = 0; index < 5; ++index) {
            const int value = index < highScores.size() ? highScores[index].score : 0;
            sf::Text row(std::to_string(index + 1) + ". " + std::to_string(value), manager.font(), 28);
            row.setPosition(410.0F, 280.0F + static_cast<float>(index) * 46.0F);
            manager.window()->draw(row);
        }

        drawButton(*manager.window(), manager.font(), playButtonBounds, "Play", 330.0F);
        drawButton(*manager.window(), manager.font(), instructionsButtonBounds, "Instructions", 518.0F);
    }
};

/**
 * @brief Instructions screen state that shows controls and power-ups.
 */
class InstructionsState final : public State {
public:
    using State::State;

    void processEvent(const sf::Event& event) override
    {
        if (event.type == sf::Event::MouseButtonPressed) {
            const auto mouse = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                           static_cast<float>(event.mouseButton.y));
            if (instructionsBackButtonBounds.contains(mouse)) {
                manager.transitionTo(StateId::Menu);
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            manager.transitionTo(StateId::Menu);
        }
    }

    void update(float) override {}

    void render() override
    {
        sf::Text title("Instructions", manager.font(), 54);
        title.setPosition(312.0F, 54.0F);
        manager.window()->draw(title);

        drawText("Goal", 60.0F, 135.0F, 28);
        drawText("Destroy soft blocks, defeat all enemies, then enter the exit that appears.", 60.0F, 174.0F, 22);
        drawText("There are 3 levels. Power-ups unlock gradually and remain available in later levels.", 60.0F, 204.0F, 22);

        drawText("Controls", 60.0F, 260.0F, 28);
        drawText("Move: Arrow keys or WASD", 60.0F, 300.0F, 22);
        drawText("Place bomb: Space", 60.0F, 330.0F, 22);
        drawText("Kick bomb after Punch Glove: K", 60.0F, 360.0F, 22);
        drawText("Pause/resume: Enter    Return to menu while paused: Esc", 60.0F, 390.0F, 22);

        drawText("Power-ups and Downs", 60.0F, 446.0F, 28);
        drawText("Fire: explosion range +1    Bomb: bomb capacity +1    Skates: speed up", 60.0F, 486.0F, 21);
        drawText("Stars: bonus points", 60.0F, 516.0F, 21);
        drawText("Punch Glove: enables K bomb kick    Purple Tear: kicked bombs bounce", 60.0F, 546.0F, 21);
        drawText("Bomb with Red X: temporarily blocks bomb placement", 60.0F, 576.0F, 21);
        drawText("Wooden Clogs: speed down    Skull: bad item, player loses life", 60.0F, 606.0F, 21);

        drawText("Level Items", 60.0F, 662.0F, 28);
        drawText("L1: Fire, Bomb, Skates, Stars", 60.0F, 702.0F, 21);
        drawText("L2: + Punch Glove, Purple Tear, Red X    L3: + Wooden Clogs, Skull", 60.0F, 732.0F, 21);

        drawButton(*manager.window(), manager.font(), instructionsBackButtonBounds, "Back", 448.0F);
    }

private:
    void drawText(const std::string& value, float x, float y, unsigned int size)
    {
        sf::Text text(value, manager.font(), size);
        text.setPosition(x, y);
        manager.window()->draw(text);
    }
};

/**
 * @brief Active gameplay state that forwards input to the logic world.
 */
class PlayingState final : public State {
public:
    using State::State;

    void processEvent(const sf::Event& event) override
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            paused = !paused;
            executePlayerCommand(manager, stopHorizontalCommand);
            executePlayerCommand(manager, stopVerticalCommand);
        }

        if (paused && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            executePlayerCommand(manager, stopHorizontalCommand);
            executePlayerCommand(manager, stopVerticalCommand);
            manager.transitionTo(StateId::Menu);
        }

        if (!paused && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
            executePlayerCommand(manager, placeBombCommand);
        }

        if (!paused && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::K) {
            executePlayerCommand(manager, kickBombCommand);
        }
    }

    void update(float deltaTime) override
    {
        if (paused) {
            return;
        }

        syncMovementInput();
        manager.world().update(deltaTime);
        if (manager.world().player() != nullptr && !manager.world().player()->isAlive()) {
            manager.transitionTo(StateId::GameOver);
        } else if (manager.world().playerWon()) {
            manager.transitionTo(StateId::Victory);
        }
    }

    void render() override
    {
        manager.factory()->drawViews();
        renderHud(manager);
        if (paused) {
            renderPauseOverlay();
        }
    }

private:
    void renderPauseOverlay()
    {
        sf::RectangleShape overlay({960.0F, 832.0F});
        overlay.setFillColor(sf::Color(12, 16, 20, 150));
        manager.window()->draw(overlay);

        sf::Text title("Paused", manager.font(), 64);
        title.setPosition(360.0F, 330.0F);
        manager.window()->draw(title);

        sf::Text prompt("Press Enter to resume", manager.font(), 28);
        prompt.setPosition(322.0F, 415.0F);
        manager.window()->draw(prompt);

        sf::Text menuPrompt("Press Esc to return to menu", manager.font(), 24);
        menuPrompt.setPosition(310.0F, 462.0F);
        manager.window()->draw(menuPrompt);
    }

    void syncMovementInput()
    {
        const bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        const bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        const bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        const bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);

        if (left == right) {
            executePlayerCommand(manager, stopHorizontalCommand);
        } else if (left) {
            executePlayerCommand(manager, moveLeftCommand);
        } else {
            executePlayerCommand(manager, moveRightCommand);
        }

        if (up == down) {
            executePlayerCommand(manager, stopVerticalCommand);
        } else if (up) {
            executePlayerCommand(manager, moveUpCommand);
        } else {
            executePlayerCommand(manager, moveDownCommand);
        }
    }

    bool paused{false};
};

/**
 * @brief Game-over screen state shown when the player has no lives left.
 */
class GameOverState final : public State {
public:
    using State::State;

    void processEvent(const sf::Event& event) override
    {
        if (event.type == sf::Event::MouseButtonPressed) {
            const auto mouse = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                           static_cast<float>(event.mouseButton.y));
            if (playAgainButtonBounds.contains(mouse)) {
                manager.transitionTo(StateId::Playing);
            } else if (menuButtonBounds.contains(mouse)) {
                manager.transitionTo(StateId::Menu);
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            manager.transitionTo(StateId::Playing);
        }
    }

    void update(float) override {}

    void render() override
    {
        manager.factory()->drawViews();
        renderHud(manager);

        sf::RectangleShape overlay({960.0F, 832.0F});
        overlay.setFillColor(sf::Color(12, 16, 20, 190));
        manager.window()->draw(overlay);

        sf::Text title("Game Over", manager.font(), 64);
        title.setPosition(300.0F, 220.0F);
        manager.window()->draw(title);

        sf::Text score("Final score: " + std::to_string(manager.world().score()->getCurrentScore()), manager.font(), 32);
        score.setPosition(340.0F, 330.0F);
        manager.window()->draw(score);

        drawButton(*manager.window(), manager.font(), playAgainButtonBounds, "Play again", 294.0F);
        drawButton(*manager.window(), manager.font(), menuButtonBounds, "Menu", 566.0F);
    }
};

/**
 * @brief Victory screen state shown after the player reaches the level exit.
 */
class VictoryState final : public State {
public:
    using State::State;

    void processEvent(const sf::Event& event) override
    {
        if (event.type == sf::Event::MouseButtonPressed) {
            const auto mouse = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                           static_cast<float>(event.mouseButton.y));
            if (playAgainButtonBounds.contains(mouse)) {
                continueAfterVictory();
            } else if (menuButtonBounds.contains(mouse)) {
                manager.transitionTo(StateId::Menu);
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            continueAfterVictory();
        }
    }

    void update(float) override {}

    void render() override
    {
        manager.factory()->drawViews();
        renderHud(manager);
        const float seconds = animationSeconds();

        sf::RectangleShape overlay({960.0F, 832.0F});
        const auto overlayAlpha = static_cast<sf::Uint8>(170.0F + std::sin(seconds * 3.0F) * 18.0F);
        overlay.setFillColor(sf::Color(12, 24, 20, overlayAlpha));
        manager.window()->draw(overlay);

        renderCelebration(seconds);

        const bool finalLevel = manager.world().finalLevelComplete();
        sf::Text title(finalLevel ? "Game Complete" : "Level Clear", manager.font(), 64);
        title.setPosition(finalLevel ? 252.0F : 300.0F, 220.0F);
        const float titlePulse = 1.0F + std::sin(seconds * 5.0F) * 0.04F;
        title.setScale(titlePulse, titlePulse);
        manager.window()->draw(title);

        sf::Text score((finalLevel ? "Final score: " : "Score: ") +
                           std::to_string(manager.world().score()->getCurrentScore()),
                       manager.font(),
                       32);
        score.setPosition(finalLevel ? 340.0F : 385.0F, 330.0F);
        manager.window()->draw(score);

        drawButton(*manager.window(), manager.font(), playAgainButtonBounds, finalLevel ? "Play again" : "Next level", finalLevel ? 294.0F : 290.0F);
        drawButton(*manager.window(), manager.font(), menuButtonBounds, "Menu", 566.0F);
    }

private:
    float animationSeconds() const
    {
        return std::chrono::duration<float>(std::chrono::steady_clock::now() - animationStart).count();
    }

    void renderCelebration(float seconds)
    {
        static const std::array<sf::Color, 6> colors{
            sf::Color(255, 212, 76),
            sf::Color(255, 96, 84),
            sf::Color(98, 205, 255),
            sf::Color(116, 230, 132),
            sf::Color(194, 125, 255),
            sf::Color(255, 154, 72),
        };

        for (int index = 0; index < 36; ++index) {
            const float column = static_cast<float>((index * 73) % 900) + 30.0F;
            const float phase = static_cast<float>(index % 9) * 0.47F;
            const float y = std::fmod(seconds * (72.0F + static_cast<float>(index % 5) * 14.0F) +
                                          static_cast<float>((index * 41) % 760),
                                      760.0F) +
                            44.0F;
            const float x = column + std::sin(seconds * 2.2F + phase) * 24.0F;

            sf::RectangleShape particle({8.0F, 14.0F});
            particle.setOrigin(4.0F, 7.0F);
            particle.setPosition(x, y);
            particle.setRotation(seconds * 160.0F + static_cast<float>(index * 17));
            particle.setFillColor(colors[static_cast<std::size_t>(index) % colors.size()]);
            manager.window()->draw(particle);
        }
    }

    void continueAfterVictory()
    {
        if (manager.world().finalLevelComplete()) {
            manager.transitionTo(StateId::Playing);
            return;
        }
        manager.continueToNextLevel();
    }

    std::chrono::steady_clock::time_point animationStart{std::chrono::steady_clock::now()};
};

} // namespace state

State::State(StateManager& manager)
    : manager(manager)
{
}

StateManager::StateManager(std::shared_ptr<sf::RenderWindow> window,
                           const sf::Font& font,
                           std::shared_ptr<ConcreteFactory> factory,
                           logic::World& world)
    : renderWindow(std::move(window)), fontRef(font), entityFactory(std::move(factory)), gameWorld(world)
{
    transitionTo(StateId::Menu);
}

void StateManager::processEvent(const sf::Event& event)
{
    currentState->processEvent(event);
}

void StateManager::update(float deltaTime)
{
    currentState->update(deltaTime);
}

void StateManager::render()
{
    currentState->render();
}

void StateManager::transitionTo(StateId stateId)
{
    switch (stateId) {
    case StateId::Menu:
        currentState = std::make_unique<state::MenuState>(*this);
        break;
    case StateId::Instructions:
        currentState = std::make_unique<state::InstructionsState>(*this);
        break;
    case StateId::Playing:
        entityFactory->clearViews();
        gameWorld.startNewGame();
        logic::Stopwatch::instance().reset();
        currentState = std::make_unique<state::PlayingState>(*this);
        break;
    case StateId::GameOver:
        currentState = std::make_unique<state::GameOverState>(*this);
        break;
    case StateId::Victory:
        currentState = std::make_unique<state::VictoryState>(*this);
        break;
    }
}

void StateManager::continueToNextLevel()
{
    entityFactory->clearViews();
    gameWorld.startNextLevel();
    logic::Stopwatch::instance().reset();
    currentState = std::make_unique<state::PlayingState>(*this);
}

std::shared_ptr<sf::RenderWindow> StateManager::window() const { return renderWindow; }

const sf::Font& StateManager::font() const { return fontRef; }

std::shared_ptr<ConcreteFactory> StateManager::factory() const { return entityFactory; }

logic::World& StateManager::world() const { return gameWorld; }

}
