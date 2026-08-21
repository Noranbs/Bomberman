#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#define private public
#include "logic/World.h"
#undef private

using namespace bomberman::logic;

class TestFactory final : public AbstractFactory {
public:
    std::shared_ptr<Character> createCharacter(std::size_t id,
                                               EntityType type,
                                               Vec2 position,
                                               Vec2 size) override
    {
        return std::make_shared<Character>(id, type, position, size);
    }

    std::shared_ptr<Wall> createWall(std::size_t id, EntityType type, Vec2 position, Vec2 size) override
    {
        return std::make_shared<Wall>(id, type, position, size);
    }

    std::shared_ptr<Bomb> createBomb(std::size_t id, Vec2 position, Vec2 size) override
    {
        return std::make_shared<Bomb>(id, position, size);
    }

    std::shared_ptr<Explosion> createExplosion(std::size_t id,
                                               Vec2 position,
                                               Vec2 size,
                                               ExplosionShape explosionShape) override
    {
        return std::make_shared<Explosion>(id, position, size, explosionShape);
    }

    std::shared_ptr<Exit> createExit(std::size_t id, Vec2 position, Vec2 size) override
    {
        return std::make_shared<Exit>(id, position, size);
    }

    std::shared_ptr<PowerUp> createPowerUp(std::size_t id,
                                           Vec2 position,
                                           Vec2 size,
                                           PowerUpType powerUpType) override
    {
        return std::make_shared<PowerUp>(id, position, size, powerUpType);
    }
};

void testMovementAndBombReuse()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();

    assert(world.player() != nullptr);
    assert(world.entities().size() > 4);

    const Vec2 start = world.player()->getPosition();
    world.handlePlayerAction(Action::MoveRight, true);
    world.update(0.25F);
    world.handlePlayerAction(Action::MoveRight, false);

    assert(world.player()->getPosition().x > start.x);
    assert(world.player()->getPosition().x - start.x > 0.09F);
    assert(world.player()->getPosition().x <= 1.0F);

    const auto beforeBomb = world.entities().size();
    world.handlePlayerAction(Action::PlaceBomb, true);
    assert(world.entities().size() == beforeBomb + 1);
    world.update(2.1F);
    assert(world.player()->getAvailableBombs() == 1);
}

void testPlayerCommandsExecuteWorldActions()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();

    const MoveRightCommand moveRight{};
    const StopHorizontalCommand stopHorizontal{};
    const PlaceBombCommand placeBomb{};

    const Vec2 start = world.player()->getPosition();
    moveRight.execute(world, true);
    world.update(0.25F);
    stopHorizontal.execute(world, true);

    assert(world.player()->getPosition().x > start.x);

    const auto beforeBomb = world.entities().size();
    placeBomb.execute(world, true);
    assert(world.entities().size() == beforeBomb + 1);
}

void testPowerUpsPersistBetweenLevels()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();

    world.applyPowerUp(*world.player(), PowerUpType::Fire);
    world.applyPowerUp(*world.player(), PowerUpType::ExtraBomb);
    world.applyPowerUp(*world.player(), PowerUpType::Skates);
    world.applyPowerUp(*world.player(), PowerUpType::PunchGlove);
    world.applyPowerUp(*world.player(), PowerUpType::PurpleTear);

    world.startNextLevel();

    assert(world.currentLevel() == 2);
    assert(world.player()->getBombRadius() == 2);
    assert(world.player()->getBombCapacity() == 2);
    assert(world.player()->getAvailableBombs() == 2);
    assert(world.player()->getSpeed() > 0.5F);
    assert(world.player()->canKickBombs());
    assert(world.player()->hasRubberBombs());
}

void testLifeLossResetsPlayerPowerUpsOnly()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();

    const auto entityCount = world.entities().size();
    world.applyPowerUp(*world.player(), PowerUpType::ExtraBomb);
    world.applyPowerUp(*world.player(), PowerUpType::Skates);
    world.applyPowerUp(*world.player(), PowerUpType::PunchGlove);
    world.applyPowerUp(*world.player(), PowerUpType::PurpleTear);

    world.damagePlayer();

    assert(world.playerLives() == 4);
    assert(world.entities().size() == entityCount);
    assert(world.player()->getBombCapacity() == 1);
    assert(world.player()->getSpeed() == 0.5F);
    assert(!world.player()->canKickBombs());
    assert(!world.player()->hasRubberBombs());
}

void testLevelCap()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGameAtLevel(4);
    assert(world.currentLevel() == 3);

    world.startNextLevel();
    assert(world.currentLevel() == 3);
}

void testBombKickAndRubberBounceFlags()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();
    world.applyPowerUp(*world.player(), PowerUpType::PunchGlove);
    world.applyPowerUp(*world.player(), PowerUpType::PurpleTear);

    world.player()->setDirection(Direction::Right);
    const auto [row, col] = world.tileForPosition(world.player()->getPosition());
    auto bomb = factory->createBomb(world.nextId(), world.tileCenter(row, col + 1), world.tileDimensions);
    world.bombsList.push_back({bomb, world.player(), 2.0F, 1, true, false, {}, row, col + 1, false});
    world.entitiesList.push_back(bomb);

    world.handlePlayerAction(Action::KickBomb, true);

    assert(world.bombsList.back().moveDirection.x != 0.0F || world.bombsList.back().moveDirection.y != 0.0F);
    assert(world.bombsList.back().targetCol != col + 1 || world.bombsList.back().targetRow != row);
    assert(world.bombsList.back().rubberBounce);
}

void testExitAppearsAfterEnemiesAreDefeated()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();

    const Vec2 size{world.tileDimensions.x * 0.58F, world.tileDimensions.y * 0.58F};
    auto powerUp = factory->createPowerUp(world.nextId(), world.tileCenter(3, 3), size, PowerUpType::Fire);
    world.powerUpsList.push_back(powerUp);
    world.entitiesList.push_back(powerUp);

    for (const auto& enemy : world.enemiesList) {
        enemy->killEntity();
    }

    assert(world.levelObjectivesComplete());
    world.update(0.0F);
    assert(world.levelExit != nullptr);
    const auto [exitRow, exitCol] = world.tileForPosition(world.levelExit->getPosition());
    assert(!world.tileContains(EntityType::Wall, exitRow, exitCol));
    assert(!world.tileContains(EntityType::DestructibleBlock, exitRow, exitCol));
    assert(!world.tileContains(EntityType::PowerUp, exitRow, exitCol));
}

void testExplosionDestroysVisiblePowerUps()
{
    auto factory = std::make_shared<TestFactory>();
    World world(factory);
    world.startNewGame();

    const auto [row, col] = world.tileForPosition(world.player()->getPosition());
    const Vec2 size{world.tileDimensions.x * 0.58F, world.tileDimensions.y * 0.58F};
    auto powerUp = factory->createPowerUp(world.nextId(), world.tileCenter(row, col + 1), size, PowerUpType::Fire);
    world.powerUpsList.push_back(powerUp);
    world.entitiesList.push_back(powerUp);

    world.handlePlayerAction(Action::PlaceBomb, true);
    assert(!world.bombsList.empty());
    world.bombsList.back().timer = 0.0F;
    world.update(0.0F);

    assert(!powerUp->isAlive());
}

void testSurvivalTimeScoreAccumulatesSmallDeltas()
{
    Score score("/tmp/bomberman_score_test.txt");
    score.resetCurrentScore();

    for (int frame = 0; frame < 60; ++frame) {
        score.addSurvivalTimeScore(1.0F / 60.0F);
    }

    assert(score.getCurrentScore() == 2);
}

int main()
{
    testMovementAndBombReuse();
    testPlayerCommandsExecuteWorldActions();
    testPowerUpsPersistBetweenLevels();
    testLifeLossResetsPlayerPowerUpsOnly();
    testLevelCap();
    testBombKickAndRubberBounceFlags();
    testExitAppearsAfterEnemiesAreDefeated();
    testExplosionDestroysVisiblePowerUps();
    testSurvivalTimeScoreAccumulatesSmallDeltas();
    return 0;
}
