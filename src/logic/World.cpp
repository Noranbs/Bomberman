#include "logic/World.h"
#include "logic/Random.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace bomberman::logic {

World::World(std::shared_ptr<AbstractFactory> factory)
    : factory(std::move(factory)), scoreTracker(std::make_shared<Score>("scores.txt"))
{
    if (this->factory == nullptr) {
        throw std::invalid_argument("World requires an entity factory");
    }
    tileDimensions = {2.0F / static_cast<float>(numCols), 2.0F / static_cast<float>(numRows)};
}

void World::startNewGame()
{
    startNewGameAtLevel(1);
}

void World::startNewGameAtLevel(int level)
{
    clearArenaEntities();
    entityCounter = 1;
    playerInput = {};
    totalTime = 0.0F;
    remainingPlayerLives = 5;
    levelNumber = std::clamp(level, 1, maxLevel);
    resetPlayerStats();
    hasWon = false;

    scoreTracker->resetCurrentScore();
    createArena();
}

void World::startNextLevel()
{
    savePlayerStats();
    levelNumber = std::min(levelNumber + 1, maxLevel);
    clearArenaEntities();
    playerInput = {};
    hasWon = false;
    createArena();
}

void World::update(float deltaTime)
{
    if (playerChar != nullptr && playerChar->isAlive()) {
        playerChar->updateStatus(deltaTime);
        if (!playerChar->isFrozen()) {
            moveCharacter(*playerChar, playerInput, deltaTime);
        }
        totalTime += deltaTime;
        scoreTracker->addSurvivalTimeScore(deltaTime);
    }
    updateEnemies(deltaTime);
    updatePowerUpPickups();
    updateBombs(deltaTime);
    updateExplosions(deltaTime);
    removeDeadTransientEntities();
    updateLevelExit();
}

void World::handlePlayerAction(Action action, bool active)
{
    const float val = active ? 1.0F : 0.0F;
    switch (action) {
    case Action::MoveLeft:
        playerInput.x = -val;
        if (active && playerChar != nullptr) {
            playerChar->setDirection(Direction::Left);
        }
        break;
    case Action::MoveRight:
        playerInput.x = val;
        if (active && playerChar != nullptr) {
            playerChar->setDirection(Direction::Right);
        }
        break;
    case Action::MoveUp:
        playerInput.y = -val;
        if (active && playerChar != nullptr) {
            playerChar->setDirection(Direction::Up);
        }
        break;
    case Action::MoveDown:
        playerInput.y = val;
        if (active && playerChar != nullptr) {
            playerChar->setDirection(Direction::Down);
        }
        break;
    case Action::StopHorizontal:
        playerInput.x = 0.0F;
        break;
    case Action::StopVertical:
        playerInput.y = 0.0F;
        break;
    case Action::PlaceBomb:
        if (active) {
            placePlayerBomb();
        }
        break;
    case Action::KickBomb:
        if (active) {
            kickPlayerBomb();
        }
        break;
    }
}

void MoveLeftCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::MoveLeft, active);
}

void MoveRightCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::MoveRight, active);
}

void MoveUpCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::MoveUp, active);
}

void MoveDownCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::MoveDown, active);
}

void StopHorizontalCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::StopHorizontal, active);
}

void StopVerticalCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::StopVertical, active);
}

void PlaceBombCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::PlaceBomb, active);
}

void KickBombCommand::execute(World& world, bool active) const
{
    world.handlePlayerAction(Action::KickBomb, active);
}

int World::enemiesAlive() const
{
    return static_cast<int>(std::count_if(enemiesList.begin(), enemiesList.end(), [](const auto& enemy) {
        return enemy != nullptr && enemy->isAlive();
    }));
}

std::size_t World::nextId()
{
    return entityCounter++;
}

Vec2 World::tileCenter(int row, int col) const
{
    return {-1.0F + tileDimensions.x * (static_cast<float>(col) + 0.5F),
            -1.0F + tileDimensions.y * (static_cast<float>(row) + 0.5F)};
}

std::pair<int, int> World::tileForPosition(Vec2 position) const
{
    const auto col = std::clamp(static_cast<int>((position.x + 1.0F) / tileDimensions.x), 0, numCols - 1);
    const auto row = std::clamp(static_cast<int>((position.y + 1.0F) / tileDimensions.y), 0, numRows - 1);
    return {row, col};
}

bool World::isInsideArena(int row, int col) const
{
    return row >= 0 && col >= 0 && row < numRows && col < numCols;
}

bool World::tileContains(EntityType type, int row, int col) const
{
    if (!isInsideArena(row, col)) {
        return false;
    }

    const Rect bounds{tileCenter(row, col), {tileDimensions.x * 0.5F, tileDimensions.y * 0.5F}};
    return std::any_of(entitiesList.begin(), entitiesList.end(), [type, &bounds](const auto& entity) {
        return entity != nullptr && entity->isAlive() && entity->getType() == type &&
               entity->getBounds().intersects(bounds);
    });
}

bool World::canEnterTile(const Character& character, int row, int col) const
{
    return isInsideArena(row, col) && !collidesWithSolid(character, tileCenter(row, col));
}

bool World::collidesWithSolid(const Character& character, Vec2 target) const
{
    const Rect targetBounds{target, {character.getSize().x * 0.5F, character.getSize().y * 0.5F}};
    return std::any_of(entitiesList.begin(), entitiesList.end(), [this, &character, &targetBounds](const auto& entity) {
        if (entity->getId() == character.getId() || !entity->isAlive() || !entity->blocksMovement()) {
            return false;
        }
        if (entity->getType() == EntityType::Bomb && !bombBlocksCharacter(*entity, character)) {
            return false;
        }
        return targetBounds.intersects(entity->getBounds());
    });
}

bool World::levelObjectivesComplete() const
{
    //Enemies gone is enough, but wait for bombs and fire
    const bool bombsRemain = std::any_of(bombsList.begin(), bombsList.end(), [](const BombState& bomb) {
        return !bomb.exploded && bomb.entity != nullptr && bomb.entity->isAlive();
    });

    const bool explosionsRemain = std::any_of(explosionsList.begin(), explosionsList.end(), [](const ExplosionState& explosion) {
        return explosion.entity != nullptr && explosion.entity->isAlive();
    });

    return enemiesAlive() == 0 && !bombsRemain && !explosionsRemain;
}

void World::createArena()
{
    //Border and even/even tiles are hard walls
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            const bool border = row == 0 || col == 0 || row == numRows - 1 || col == numCols - 1;
            const bool pillar = row % 2 == 0 && col % 2 == 0;
            const bool spawnArea = (row <= 2 && col <= 2) ||
                                   (row <= 2 && col >= numCols - 3) ||
                                   (row >= numRows - 3 && col <= 2) ||
                                   (row >= numRows - 3 && col >= numCols - 3);

            //Keep corners open so nobody starts stuck
            if (border || pillar) {
                createWall(EntityType::Wall, row, col);
            } else if (!spawnArea && Random::instance().rollChance(0.72F)) {
                createWall(EntityType::DestructibleBlock, row, col);
            }
        }
    }

    const Vec2 characterSize{tileDimensions.x * 0.58F, tileDimensions.y * 0.70F};
    playerChar = factory->createCharacter(nextId(), EntityType::Player, tileCenter(1, 1), characterSize);
    restorePlayerStats();
    playerChar->addObserver(scoreTracker);
    entitiesList.push_back(playerChar);

    const std::vector<Vec2> enemyPositions{
        tileCenter(1, numCols - 2),
        tileCenter(numRows - 2, 1),
        tileCenter(numRows - 2, numCols - 2),
    };

    for (const auto& position : enemyPositions) {
        auto enemy = factory->createCharacter(nextId(), EntityType::Enemy, position, characterSize);
        applyEnemyLevelBonus(*enemy);
        enemiesList.push_back(enemy);
        const float firstDecisionDelay = std::max(0.05F, Random::instance().getRandomFloat(0.2F, 0.8F) - 0.04F * static_cast<float>(levelNumber - 1));
        const float firstBombCooldown = std::max(0.25F, 1.0F - 0.1F * static_cast<float>(levelNumber - 1));
        enemyAiStates.push_back({enemy, {}, firstDecisionDelay, firstBombCooldown});
        entitiesList.push_back(enemy);
    }
}

void World::clearArenaEntities()
{
    entitiesList.clear();
    enemiesList.clear();
    bombsList.clear();
    explosionsList.clear();
    powerUpsList.clear();
    enemyAiStates.clear();
    playerChar.reset();
    levelExit.reset();
}

void World::applyEnemyLevelBonus(Character& enemy)
{
    const int bonusSteps = levelNumber - 1;
    for (int step = 0; step < bonusSteps; ++step) {
        if (step % 3 == 0) {
            enemy.boostMovementSpeed(0.05F);
        } else if (step % 3 == 1) {
            enemy.expandExplosionRange();
        } else {
            enemy.increaseMaxBombs();
        }
    }
}

void World::updateLevelExit()
{
    if (hasWon || playerChar == nullptr || !playerChar->isAlive()) {
        return;
    }

    if (levelExit == nullptr && levelObjectivesComplete()) {
        createLevelExit();
    }

    if (levelExit != nullptr && levelExit->isAlive() && playerChar->getBounds().intersects(levelExit->getBounds())) {
        hasWon = true;
        scoreTracker->onNotify({EventType::PlayerWon, playerChar->getId(), 1000});
    }
}

void World::createLevelExit()
{
    int exitRow = numRows / 2;
    int exitCol = numCols / 2;
    int bestDistance = numRows + numCols;

    //Put exit near middle but only on free tile
    for (int row = 1; row < numRows - 1; ++row) {
        for (int col = 1; col < numCols - 1; ++col) {
            if (tileContains(EntityType::Wall, row, col) ||
                tileContains(EntityType::DestructibleBlock, row, col) ||
                tileContains(EntityType::Bomb, row, col) ||
                tileContains(EntityType::PowerUp, row, col)) {
                continue;
            }

            const int distance = std::abs(row - numRows / 2) + std::abs(col - numCols / 2);
            if (distance < bestDistance) {
                bestDistance = distance;
                exitRow = row;
                exitCol = col;
            }
        }
    }

    const Vec2 size{tileDimensions.x * 0.8F, tileDimensions.y * 0.8F};
    levelExit = factory->createExit(nextId(), tileCenter(exitRow, exitCol), size);
    entitiesList.push_back(levelExit);
}

void World::createWall(EntityType type, int row, int col)
{
    auto wall = factory->createWall(nextId(), type, tileCenter(row, col), tileDimensions);
    entitiesList.push_back(wall);
}

void World::moveCharacter(Character& character, Vec2 velocity, float deltaTime)
{
    if (velocity.x == 0.0F && velocity.y == 0.0F) {
        return;
    }

    const float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if (length > 0.0F) {
        velocity.x /= length;
        velocity.y /= length;
        velocity = velocity * character.getSpeed();
    }

    Vec2 current = character.getPosition();
    const auto [row, col] = tileForPosition(current);
    const Vec2 laneCenter = tileCenter(row, col);
    const float maxAssist = character.getSpeed() * deltaTime * 0.75F;
    const float horizontalTolerance = tileDimensions.x * 0.32F;
    const float verticalTolerance = tileDimensions.y * 0.32F;

    //Small lane help so player does not get stuck on corners
    if (velocity.y != 0.0F && velocity.x == 0.0F && std::abs(laneCenter.x - current.x) <= horizontalTolerance) {
        Vec2 assisted{current.x + std::clamp(laneCenter.x - current.x, -maxAssist, maxAssist), current.y};
        if (!collidesWithSolid(character, assisted)) {
            character.setPosition(assisted);
            current = character.getPosition();
        }
    }

    if (velocity.x != 0.0F && velocity.y == 0.0F && std::abs(laneCenter.y - current.y) <= verticalTolerance) {
        Vec2 assisted{current.x, current.y + std::clamp(laneCenter.y - current.y, -maxAssist, maxAssist)};
        if (!collidesWithSolid(character, assisted)) {
            character.setPosition(assisted);
            current = character.getPosition();
        }
    }

    const Vec2 horizontal{current.x + velocity.x * deltaTime, current.y};
    if (!collidesWithSolid(character, horizontal)) {
        character.setPosition(horizontal);
    }

    const Vec2 afterHorizontal = character.getPosition();
    const Vec2 vertical{afterHorizontal.x, afterHorizontal.y + velocity.y * deltaTime};
    if (!collidesWithSolid(character, vertical)) {
        character.setPosition(vertical);
    }
}

void World::damagePlayer()
{
    if (playerChar == nullptr || !playerChar->isAlive()) {
        return;
    }

    --remainingPlayerLives;
    if (remainingPlayerLives <= 0) {
        //Last life means game over and save score
        remainingPlayerLives = 0;
        playerChar->killEntity();
        scoreTracker->onNotify({EventType::PlayerLost, playerChar->getId(), 0});
        return;
    }

    //Lost life means reset upgrades and go spawn
    playerInput = {};
    resetPlayerStats();
    playerChar->resetPowerUps();
    playerChar->notify({EventType::PlayerDamaged, playerChar->getId(), remainingPlayerLives});
    playerChar->setPosition(tileCenter(1, 1));
}

void World::removeDeadTransientEntities()
{
    explosionsList.erase(std::remove_if(explosionsList.begin(),
                                         explosionsList.end(),
                                         [](const ExplosionState& state) {
                                             return state.entity == nullptr || !state.entity->isAlive();
                                         }),
                          explosionsList.end());
    powerUpsList.erase(std::remove_if(powerUpsList.begin(),
                                       powerUpsList.end(),
                                       [](const std::shared_ptr<PowerUp>& powerUp) {
                                           return powerUp == nullptr || !powerUp->isAlive();
                                       }),
                        powerUpsList.end());
    entitiesList.erase(std::remove_if(entitiesList.begin(),
                                      entitiesList.end(),
                                      [](const std::shared_ptr<Entity>& entity) {
                                          return entity != nullptr &&
                                                 !entity->isAlive() &&
                                                 (entity->getType() == EntityType::DestructibleBlock ||
                                                  entity->getType() == EntityType::Bomb ||
                                                  entity->getType() == EntityType::Explosion ||
                                                  entity->getType() == EntityType::PowerUp);
                                      }),
                       entitiesList.end());
}

} // namespace bomberman::logic
