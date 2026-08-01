#include "bomberman/logic/World.h"
#include "bomberman/logic/Random.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <queue>
#include <stdexcept>
#include <utility>

namespace bomberman::logic {

World::World(std::shared_ptr<EntityFactory> factory)
    : factory(std::move(factory)), scoreTracker(std::make_shared<Score>("scores.txt"))
{
    if (this->factory == nullptr) {
        throw std::invalid_argument("World requires an entity factory");
    }
    tileDimensions = {2.0F / static_cast<float>(numCols), 2.0F / static_cast<float>(numRows)};
}

void World::startNewGame()
{
    entitiesList.clear();
    enemiesList.clear();
    bombsList.clear();
    explosionsList.clear();
    powerUpsList.clear();
    enemyAiStates.clear();
    playerChar.reset();
    entityCounter = 1;
    playerInput = {};
    totalTime = 0.0F;
    hasWon = false;

    scoreTracker->resetCurrentScore();
    createArena();
}

void World::update(float deltaTime)
{
    if (playerChar != nullptr && playerChar->isAlive()) {
        moveCharacter(*playerChar, playerInput * playerChar->getSpeed(), deltaTime);
        totalTime += deltaTime;
        scoreTracker->addSurvivalTimeScore(deltaTime);
    }
    updateEnemies(deltaTime);
    updatePowerUpPickups();
    updateBombs(deltaTime);
    updateExplosions(deltaTime);
    removeDeadTransientEntities();
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
    }
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

bool World::isTileDangerous(int row, int col) const
{
    if (!isInsideArena(row, col)) {
        return true;
    }

    if (tileContains(EntityType::Explosion, row, col)) {
        return true;
    }

    for (const auto& bomb : bombsList) {
        if (bomb.entity == nullptr || !bomb.entity->isAlive()) {
            continue;
        }

        const auto [bombRow, bombCol] = tileForPosition(bomb.entity->getPosition());
        if (bombRow == row && bombCol == col) {
            return true;
        }

        if (isTileThreatenedByBomb(row, col, bombRow, bombCol, bomb.radius)) {
            return true;
        }
    }

    return false;
}

bool World::isTileThreatenedByBomb(int row, int col, int bombRow, int bombCol, int radius) const
{
    if (bombRow == row && bombCol == col) {
        return true;
    }

    if (bombRow != row && bombCol != col) {
        return false;
    }

    const int rowStep = row == bombRow ? 0 : (row > bombRow ? 1 : -1);
    const int colStep = col == bombCol ? 0 : (col > bombCol ? 1 : -1);
    const int distance = std::abs(row - bombRow) + std::abs(col - bombCol);
    if (distance > radius) {
        return false;
    }

    for (int step = 1; step <= distance; ++step) {
        const int checkRow = bombRow + rowStep * step;
        const int checkCol = bombCol + colStep * step;
        if (tileContains(EntityType::Wall, checkRow, checkCol)) {
            return false;
        }
        if (step < distance && tileContains(EntityType::DestructibleBlock, checkRow, checkCol)) {
            return false;
        }
    }

    return true;
}

bool World::hasEscapeFromBomb(const Character& character, int bombRow, int bombCol, int radius) const
{
    return findEscapeInput(character, bombRow, bombCol, radius).has_value();
}

std::optional<Vec2> World::findEscapeInput(const Character& character, int bombRow, int bombCol, int radius) const
{
    static constexpr std::array<std::pair<int, int>, 4> directions{
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0},
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
    };

    struct SearchNode {
        int row{0};
        int col{0};
        int distance{0};
        Vec2 firstInput{};
    };

    const int maxDistance = radius + 4;
    std::vector<bool> visited(static_cast<std::size_t>(numRows * numCols), false);
    std::queue<SearchNode> queue{};
    const auto indexFor = [this](int r, int c) {
        return static_cast<std::size_t>(r * numCols + c);
    };
    const auto [startRow, startCol] = tileForPosition(character.getPosition());

    visited[indexFor(startRow, startCol)] = true;

    for (const auto& [rowStep, colStep] : directions) {
        const int r = startRow + rowStep;
        const int c = startCol + colStep;
        if (!canEnterTile(character, r, c) || visited[indexFor(r, c)]) {
            continue;
        }

        visited[indexFor(r, c)] = true;
        queue.push({r, c, 1, directionForStep(rowStep, colStep)});
    }

    while (!queue.empty()) {
        const SearchNode node = queue.front();
        queue.pop();

        if (!isTileDangerous(node.row, node.col) &&
            !isTileThreatenedByBomb(node.row, node.col, bombRow, bombCol, radius)) {
            return node.firstInput;
        }

        if (node.distance >= maxDistance) {
            continue;
        }

        for (const auto& [rowStep, colStep] : directions) {
            const int r = node.row + rowStep;
            const int c = node.col + colStep;
            if (!isInsideArena(r, c) || visited[indexFor(r, c)] || !canEnterTile(character, r, c)) {
                continue;
            }

            visited[indexFor(r, c)] = true;
            queue.push({r, c, node.distance + 1, node.firstInput});
        }
    }

    return std::nullopt;
}

std::optional<Vec2> World::findStraightEscapeInput(const Character& character,
                                                   int bombRow,
                                                   int bombCol,
                                                   int radius) const
{
    static constexpr std::array<std::pair<int, int>, 4> directions{
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0},
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
    };

    for (const auto& [rowStep, colStep] : directions) {
        bool clearPath = true;
        for (int distance = 1; distance <= radius + 1; ++distance) {
            const int r = bombRow + rowStep * distance;
            const int c = bombCol + colStep * distance;
            if (!canEnterTile(character, r, c) || isTileDangerous(r, c)) {
                clearPath = false;
                break;
            }
        }

        const int safeRow = bombRow + rowStep * (radius + 1);
        const int safeCol = bombCol + colStep * (radius + 1);
        if (clearPath && !isTileThreatenedByBomb(safeRow, safeCol, bombRow, bombCol, radius)) {
            return directionForStep(rowStep, colStep);
        }
    }

    return std::nullopt;
}

Vec2 World::directionForStep(int rowStep, int colStep) const
{
    if (rowStep < 0) {
        return {0.0F, -1.0F};
    }
    if (rowStep > 0) {
        return {0.0F, 1.0F};
    }
    if (colStep < 0) {
        return {-1.0F, 0.0F};
    }
    if (colStep > 0) {
        return {1.0F, 0.0F};
    }
    return {};
}

Vec2 World::chooseEnemyInput(Character& enemy)
{
    const auto [row, col] = tileForPosition(enemy.getPosition());
    static constexpr std::array<std::pair<int, int>, 4> directions{
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0},
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
    };

    std::vector<std::pair<int, int>> validDirections{};
    for (const auto& [rowStep, colStep] : directions) {
        const int nextRow = row + rowStep;
        const int nextCol = col + colStep;
        if (canEnterTile(enemy, nextRow, nextCol)) {
            validDirections.push_back({rowStep, colStep});
        }
    }

    if (validDirections.empty()) {
        return {};
    }

    if (isTileDangerous(row, col)) {
        for (const auto& bomb : bombsList) {
            if (bomb.entity == nullptr || !bomb.entity->isAlive()) {
                continue;
            }

            const auto [bombRow, bombCol] = tileForPosition(bomb.entity->getPosition());
            if (isTileThreatenedByBomb(row, col, bombRow, bombCol, bomb.radius)) {
                if (const auto escapeInput = findEscapeInput(enemy, bombRow, bombCol, bomb.radius);
                    escapeInput.has_value()) {
                    return *escapeInput;
                }
            }
        }

        for (const auto& [rowStep, colStep] : validDirections) {
            const int nextRow = row + rowStep;
            const int nextCol = col + colStep;
            if (!tileContains(EntityType::Bomb, nextRow, nextCol)) {
                return directionForStep(rowStep, colStep);
            }
        }
    }

    for (const auto& powerUp : powerUpsList) {
        if (powerUp == nullptr || !powerUp->isAlive()) {
            continue;
        }

        const auto [powerUpRow, powerUpCol] = tileForPosition(powerUp->getPosition());
        const int rowDistance = powerUpRow - row;
        const int colDistance = powerUpCol - col;
        if (std::abs(rowDistance) + std::abs(colDistance) > 5) {
            continue;
        }

        if (std::abs(rowDistance) > std::abs(colDistance) && rowDistance != 0) {
            const int rowStep = rowDistance > 0 ? 1 : -1;
            if (canEnterTile(enemy, row + rowStep, col) && !isTileDangerous(row + rowStep, col)) {
                return directionForStep(rowStep, 0);
            }
        }

        if (colDistance != 0) {
            const int colStep = colDistance > 0 ? 1 : -1;
            if (canEnterTile(enemy, row, col + colStep) && !isTileDangerous(row, col + colStep)) {
                return directionForStep(0, colStep);
            }
        }
    }

    const int start = Random::instance().getRandomInt(0, static_cast<int>(validDirections.size()) - 1);
    for (std::size_t offset = 0; offset < validDirections.size(); ++offset) {
        const auto& [rowStep, colStep] = validDirections[(static_cast<std::size_t>(start) + offset) %
                                                         validDirections.size()];
        if (!isTileDangerous(row + rowStep, col + colStep)) {
            return directionForStep(rowStep, colStep);
        }
    }

    const auto& [rowStep, colStep] = validDirections.front();
    return directionForStep(rowStep, colStep);
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

bool World::bombBlocksCharacter(const Entity& bomb, const Character& character) const
{
    const auto found = std::find_if(bombsList.begin(), bombsList.end(), [&bomb](const BombState& state) {
        return state.entity != nullptr && state.entity->getId() == bomb.getId();
    });

    if (found == bombsList.end()) {
        return true;
    }

    const auto owner = found->owner.lock();
    return owner == nullptr || owner->getId() != character.getId() || found->blocksOwner;
}

bool World::hasBombAt(int row, int col) const
{
    const Vec2 center = tileCenter(row, col);
    return std::any_of(bombsList.begin(), bombsList.end(), [&center](const BombState& state) {
        return state.entity != nullptr && state.entity->isAlive() &&
               state.entity->getBounds().intersects({center, {0.001F, 0.001F}});
    });
}

bool World::hasAdjacentDestructibleBlock(int row, int col) const
{
    static constexpr std::array<std::pair<int, int>, 4> directions{
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0},
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
    };

    return std::any_of(directions.begin(), directions.end(), [this, row, col](const auto& direction) {
        return tileContains(EntityType::DestructibleBlock, row + direction.first, col + direction.second);
    });
}

void World::createArena()
{
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            const bool border = row == 0 || col == 0 || row == numRows - 1 || col == numCols - 1;
            const bool pillar = row % 2 == 0 && col % 2 == 0;
            const bool spawnArea = (row <= 2 && col <= 2) ||
                                   (row <= 2 && col >= numCols - 3) ||
                                   (row >= numRows - 3 && col <= 2) ||
                                   (row >= numRows - 3 && col >= numCols - 3);

            if (border || pillar) {
                createBlock(EntityType::Wall, row, col);
            } else if (!spawnArea && Random::instance().rollChance(0.72F)) {
                createBlock(EntityType::DestructibleBlock, row, col);
            }
        }
    }

    const Vec2 characterSize{tileDimensions.x * 0.58F, tileDimensions.y * 0.70F};
    playerChar = factory->createCharacter(nextId(), EntityType::Player, tileCenter(1, 1), characterSize);
    playerChar->addObserver(scoreTracker);
    entitiesList.push_back(playerChar);

    const std::vector<Vec2> enemyPositions{
        tileCenter(1, numCols - 2),
        tileCenter(numRows - 2, 1),
        tileCenter(numRows - 2, numCols - 2),
    };

    for (const auto& position : enemyPositions) {
        auto enemy = factory->createCharacter(nextId(), EntityType::Enemy, position, characterSize);
        enemiesList.push_back(enemy);
        enemyAiStates.push_back({enemy, {}, Random::instance().getRandomFloat(0.2F, 0.8F), 1.0F});
        entitiesList.push_back(enemy);
    }
}

void World::createBlock(EntityType type, int row, int col)
{
    auto block = factory->createBlock(nextId(), type, tileCenter(row, col), tileDimensions);
    entitiesList.push_back(block);
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

    const Vec2 current = character.getPosition();
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

void World::placePlayerBomb()
{
    placeBombFor(playerChar);
}

void World::placeBombFor(const std::shared_ptr<Character>& character)
{
    if (character == nullptr || !character->isAlive() || !character->tryPlaceBomb()) {
        return;
    }

    const auto [row, col] = tileForPosition(character->getPosition());
    if (hasBombAt(row, col)) {
        character->replenishBomb();
        return;
    }

    auto bomb = factory->createBlock(nextId(), EntityType::Bomb, tileCenter(row, col), tileDimensions);
    bombsList.push_back({bomb, character, 2.0F, character->getBombRadius(), false});
    entitiesList.push_back(bomb);
}

void World::updateEnemies(float deltaTime)
{
    for (auto& ai : enemyAiStates) {
        const auto enemy = ai.enemy.lock();
        if (enemy == nullptr || !enemy->isAlive()) {
            continue;
        }

        ai.decisionTimer -= deltaTime;
        ai.bombCooldown -= deltaTime;

        const auto [row, col] = tileForPosition(enemy->getPosition());
        if (ai.escapingOwnBomb) {
            if (!hasBombAt(ai.escapeBombRow, ai.escapeBombCol)) {
                ai.escapingOwnBomb = false;
                ai.input = {};
                ai.hasTarget = false;
                ai.decisionTimer = 0.0F;
            }
        }

        const bool dangerous = isTileDangerous(row, col);
        const auto straightEscapeInput = findStraightEscapeInput(*enemy, row, col, enemy->getBombRadius());
        if (!ai.escapingOwnBomb &&
            !dangerous &&
            ai.bombCooldown <= 0.0F &&
            hasAdjacentDestructibleBlock(row, col) &&
            straightEscapeInput.has_value()) {
            const int bombRadius = enemy->getBombRadius();
            placeBombFor(enemy);
            ai.bombCooldown = Random::instance().getRandomFloat(2.5F, 4.5F);
            ai.escapingOwnBomb = true;
            ai.escapeBombRow = row;
            ai.escapeBombCol = col;
            ai.escapeBombRadius = bombRadius;
            ai.input = *straightEscapeInput;
            ai.hasTarget = false;
            ai.decisionTimer = 0.15F;
        }

        if (!ai.escapingOwnBomb &&
            (dangerous || ai.decisionTimer <= 0.0F || (ai.input.x == 0.0F && ai.input.y == 0.0F))) {
            ai.input = chooseEnemyInput(*enemy);
            ai.hasTarget = false;
            ai.decisionTimer = dangerous ? 0.15F : Random::instance().getRandomFloat(0.35F, 0.9F);
        }

        if (ai.input.x < 0.0F) {
            enemy->setDirection(Direction::Left);
        } else if (ai.input.x > 0.0F) {
            enemy->setDirection(Direction::Right);
        } else if (ai.input.y < 0.0F) {
            enemy->setDirection(Direction::Up);
        } else if (ai.input.y > 0.0F) {
            enemy->setDirection(Direction::Down);
        }

        moveEnemyTowardTarget(*enemy, ai, deltaTime);
    }
}

void World::moveEnemyTowardTarget(Character& enemy, EnemyAiState& ai, float deltaTime)
{
    if (ai.input.x == 0.0F && ai.input.y == 0.0F) {
        return;
    }

    const auto [row, col] = tileForPosition(enemy.getPosition());
    if (!ai.hasTarget) {
        ai.targetRow = row + (ai.input.y > 0.0F ? 1 : (ai.input.y < 0.0F ? -1 : 0));
        ai.targetCol = col + (ai.input.x > 0.0F ? 1 : (ai.input.x < 0.0F ? -1 : 0));

        if (!canEnterTile(enemy, ai.targetRow, ai.targetCol)) {
            if (ai.escapingOwnBomb) {
                if (const auto escapeInput = findEscapeInput(enemy,
                                                             ai.escapeBombRow,
                                                             ai.escapeBombCol,
                                                             ai.escapeBombRadius);
                    escapeInput.has_value()) {
                    ai.input = *escapeInput;
                    ai.hasTarget = false;
                } else {
                    ai.input = {};
                }
            } else {
                ai.input = {};
                ai.decisionTimer = 0.0F;
            }
            return;
        }

        ai.hasTarget = true;
    }

    const Vec2 target = tileCenter(ai.targetRow, ai.targetCol);
    Vec2 position = enemy.getPosition();
    const float maxStep = enemy.getSpeed() * deltaTime;
    const float dx = target.x - position.x;
    const float dy = target.y - position.y;
    const float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= maxStep || distance <= 0.001F) {
        enemy.setPosition(target);
        ai.hasTarget = false;

        if (ai.escapingOwnBomb && hasBombAt(ai.escapeBombRow, ai.escapeBombCol)) {
            const auto [currentRow, currentCol] = tileForPosition(enemy.getPosition());
            const int nextRow = currentRow + (ai.input.y > 0.0F ? 1 : (ai.input.y < 0.0F ? -1 : 0));
            const int nextCol = currentCol + (ai.input.x > 0.0F ? 1 : (ai.input.x < 0.0F ? -1 : 0));
            if (!canEnterTile(enemy, nextRow, nextCol) ||
                (!isTileThreatenedByBomb(currentRow, currentCol, ai.escapeBombRow, ai.escapeBombCol, ai.escapeBombRadius) &&
                 !isTileThreatenedByBomb(nextRow, nextCol, ai.escapeBombRow, ai.escapeBombCol, ai.escapeBombRadius))) {
                ai.input = findEscapeInput(enemy,
                                           ai.escapeBombRow,
                                           ai.escapeBombCol,
                                           ai.escapeBombRadius)
                               .value_or(ai.input);
            }
        }
        return;
    }

    position.x += dx / distance * maxStep;
    position.y += dy / distance * maxStep;
    if (!collidesWithSolid(enemy, position)) {
        enemy.setPosition(position);
    } else {
        ai.hasTarget = false;
        ai.input = {};
        ai.decisionTimer = 0.0F;
    }
}

void World::createRandomPowerUp(int row, int col)
{
    if (!Random::instance().rollChance(0.25F)) {
        return;
    }

    const int typeIndex = Random::instance().getRandomInt(0, 2);
    const auto type = static_cast<PowerUpType>(typeIndex);
    const Vec2 size{tileDimensions.x * 0.58F, tileDimensions.y * 0.58F};
    auto powerUp = factory->createPowerUp(nextId(), tileCenter(row, col), size, type);
    powerUpsList.push_back(powerUp);
    entitiesList.push_back(powerUp);
}

void World::updatePowerUpPickups()
{
    if (playerChar != nullptr && playerChar->isAlive()) {
        collectPowerUps(*playerChar);
    }

    for (const auto& enemy : enemiesList) {
        if (enemy != nullptr && enemy->isAlive()) {
            collectPowerUps(*enemy);
        }
    }
}

void World::collectPowerUps(Character& character)
{
    for (auto& powerUp : powerUpsList) {
        if (powerUp == nullptr || !powerUp->isAlive() || !character.getBounds().intersects(powerUp->getBounds())) {
            continue;
        }

        const auto powerUpType = powerUp->powerUpType();
        if (!powerUpType.has_value()) {
            continue;
        }

        applyPowerUp(character, *powerUpType);
        powerUp->killEntity();
        if (character.getType() == EntityType::Player) {
            scoreTracker->onNotify({EventType::PowerUpCollected, powerUp->getId(), 100});
        }
    }
}

void World::applyPowerUp(Character& character, PowerUpType type)
{
    switch (type) {
    case PowerUpType::Fire:
        character.expandExplosionRange();
        break;
    case PowerUpType::ExtraBomb:
        character.increaseMaxBombs();
        break;
    case PowerUpType::Skates:
        character.boostMovementSpeed(0.12F);
        break;
    }
}

void World::updateBombs(float deltaTime)
{
    for (auto& bomb : bombsList) {
        if (bomb.entity == nullptr || !bomb.entity->isAlive()) {
            continue;
        }

        const auto owner = bomb.owner.lock();
        if (owner == nullptr || !owner->getBounds().intersects(bomb.entity->getBounds())) {
            bomb.blocksOwner = true;
        }

        bomb.timer -= deltaTime;
    }

    for (std::size_t index = 0; index < bombsList.size();) {
        if (bombsList[index].entity != nullptr && bombsList[index].entity->isAlive() && bombsList[index].timer <= 0.0F) {
            explodeBomb(index);
        } else {
            ++index;
        }
    }
}

void World::updateExplosions(float deltaTime)
{
    for (auto& explosion : explosionsList) {
        explosion.timer -= deltaTime;
        if (explosion.timer <= 0.0F && explosion.entity != nullptr) {
            explosion.entity->killEntity();
        }
    }
}

void World::explodeBomb(std::size_t index)
{
    if (index >= bombsList.size() || bombsList[index].entity == nullptr) {
        return;
    }

    auto bomb = bombsList[index];
    const auto [row, col] = tileForPosition(bomb.entity->getPosition());
    bomb.entity->killEntity();

    if (const auto owner = bomb.owner.lock(); owner != nullptr) {
        owner->replenishBomb();
    }

    applyExplosionToTile(row, col);

    static constexpr std::array<std::pair<int, int>, 4> directions{
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0},
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
    };

    for (const auto& [rowStep, colStep] : directions) {
        for (int distance = 1; distance <= bomb.radius; ++distance) {
            if (!applyExplosionToTile(row + rowStep * distance, col + colStep * distance)) {
                break;
            }
        }
    }

    bombsList.erase(bombsList.begin() + static_cast<std::ptrdiff_t>(index));
}

void World::createExplosionTile(int row, int col)
{
    auto explosion = factory->createBlock(nextId(), EntityType::Explosion, tileCenter(row, col), tileDimensions);
    explosionsList.push_back({explosion, 0.35F});
    entitiesList.push_back(explosion);
}

bool World::applyExplosionToTile(int row, int col)
{
    if (row < 0 || col < 0 || row >= numRows || col >= numCols) {
        return false;
    }

    const Rect tileBounds{tileCenter(row, col), {tileDimensions.x * 0.5F, tileDimensions.y * 0.5F}};

    for (auto& entity : entitiesList) {
        if (entity->isAlive() && entity->getType() == EntityType::Wall && entity->getBounds().intersects(tileBounds)) {
            return false;
        }
    }

    createExplosionTile(row, col);

    const auto destructibleBlock = std::find_if(entitiesList.begin(), entitiesList.end(), [&tileBounds](const auto& entity) {
        return entity->isAlive() && entity->getType() == EntityType::DestructibleBlock &&
               entity->getBounds().intersects(tileBounds);
    });

    if (destructibleBlock != entitiesList.end()) {
        (*destructibleBlock)->killEntity();
        scoreTracker->onNotify({EventType::BlockDestroyed, (*destructibleBlock)->getId(), 50});
        createRandomPowerUp(row, col);
        return false;
    }

    for (auto& entity : entitiesList) {
        if (!entity->isAlive() || !entity->getBounds().intersects(tileBounds)) {
            continue;
        }

        switch (entity->getType()) {
        case EntityType::Player:
        case EntityType::Enemy:
            entity->killEntity();
            if (entity->getType() == EntityType::Player) {
                scoreTracker->onNotify({EventType::PlayerLost, entity->getId(), 0});
            } else {
                scoreTracker->onNotify({EventType::EnemyKilled, entity->getId(), 250});
                const bool enemiesRemain = std::any_of(enemiesList.begin(), enemiesList.end(), [](const auto& enemy) {
                    return enemy != nullptr && enemy->isAlive();
                });
                if (!enemiesRemain && playerChar != nullptr && playerChar->isAlive() && !hasWon) {
                    hasWon = true;
                    scoreTracker->onNotify({EventType::PlayerWon, playerChar->getId(), 1000});
                }
            }
            break;

        case EntityType::PowerUp:
            entity->killEntity();
            break;

        case EntityType::Bomb: {
            const auto foundBomb = std::find_if(bombsList.begin(), bombsList.end(), [&entity](const BombState& state) {
                return state.entity != nullptr && state.entity->getId() == entity->getId();
            });
            if (foundBomb != bombsList.end()) {
                foundBomb->timer = 0.0F;
            }
            break;
        }

        case EntityType::Wall:
        case EntityType::DestructibleBlock:
        case EntityType::Explosion:
            break;
        }
    }

    return true;
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

}
