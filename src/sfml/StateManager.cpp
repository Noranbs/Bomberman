#include "bomberman/sfml/StateManager.h"
#include "bomberman/logic/Stopwatch.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace bomberman::sfml {

namespace {

const sf::FloatRect playButtonBounds{380.0F, 620.0F, 200.0F, 64.0F};
const sf::FloatRect playAgainButtonBounds{260.0F, 620.0F, 200.0F, 64.0F};
const sf::FloatRect menuButtonBounds{500.0F, 620.0F, 200.0F, 64.0F};

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
    hudText << "Score: " << manager.world().score()->getCurrentScore()
            << "   Time: " << std::setfill('0') << std::setw(2) << minutes << ':'
            << std::setw(2) << seconds << std::setfill(' ');

    if (player != nullptr) {
        hudText << "   Bombs: " << player->getAvailableBombs() << '/' << player->getBombCapacity()
                << "   Fire: " << player->getBombRadius()
                << "   Speed: " << std::fixed << std::setprecision(1) << player->getSpeedMultiplier() << 'x';
    }

    hudText << "   Enemies: " << manager.world().enemiesAlive();

    sf::Text text(hudText.str(), manager.font(), 20);
    text.setPosition(24.0F, 19.0F);
    manager.window()->draw(text);
}

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

        drawButton(*manager.window(), manager.font(), playButtonBounds, "Play", 450.0F);
    }
};

class PlayingState final : public State {
public:
    using State::State;

    void processEvent(const sf::Event& event) override
    {
        if (event.type == sf::Event::KeyPressed) {
            handleKey(event.key.code, true);
        }
        if (event.type == sf::Event::KeyReleased) {
            handleKey(event.key.code, false);
        }
    }

    void update(float deltaTime) override
    {
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
    }

private:
    void handleKey(sf::Keyboard::Key key, bool pressed)
    {
        switch (key) {
        case sf::Keyboard::Left:
        case sf::Keyboard::A:
            manager.world().handlePlayerAction(logic::Action::MoveLeft, pressed);
            break;
        case sf::Keyboard::Right:
        case sf::Keyboard::D:
            manager.world().handlePlayerAction(logic::Action::MoveRight, pressed);
            break;
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            manager.world().handlePlayerAction(logic::Action::MoveUp, pressed);
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            manager.world().handlePlayerAction(logic::Action::MoveDown, pressed);
            break;
        case sf::Keyboard::Space:
            manager.world().handlePlayerAction(logic::Action::PlaceBomb, pressed);
            break;
        default:
            break;
        }
    }
};

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

class VictoryState final : public State {
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
        overlay.setFillColor(sf::Color(12, 24, 20, 185));
        manager.window()->draw(overlay);

        sf::Text title("You Win", manager.font(), 64);
        title.setPosition(344.0F, 220.0F);
        manager.window()->draw(title);

        sf::Text score("Final score: " + std::to_string(manager.world().score()->getCurrentScore()), manager.font(), 32);
        score.setPosition(340.0F, 330.0F);
        manager.window()->draw(score);

        drawButton(*manager.window(), manager.font(), playAgainButtonBounds, "Play again", 294.0F);
        drawButton(*manager.window(), manager.font(), menuButtonBounds, "Menu", 566.0F);
    }
};

} // namespace

State::State(StateManager& manager)
    : manager(manager)
{
}

StateManager::StateManager(std::shared_ptr<sf::RenderWindow> window,
                           const sf::Font& font,
                           std::shared_ptr<SfmlFactory> factory,
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
        currentState = std::make_unique<MenuState>(*this);
        break;
    case StateId::Playing:
        entityFactory->clearViews();
        gameWorld.startNewGame();
        logic::Stopwatch::instance().reset();
        currentState = std::make_unique<PlayingState>(*this);
        break;
    case StateId::GameOver:
        currentState = std::make_unique<GameOverState>(*this);
        break;
    case StateId::Victory:
        currentState = std::make_unique<VictoryState>(*this);
        break;
    }
}

std::shared_ptr<sf::RenderWindow> StateManager::window() const { return renderWindow; }

const sf::Font& StateManager::font() const { return fontRef; }

std::shared_ptr<SfmlFactory> StateManager::factory() const { return entityFactory; }

logic::World& StateManager::world() const { return gameWorld; }

}
