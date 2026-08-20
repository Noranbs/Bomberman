#include "bomberman/logic/World.h"
#include "bomberman/logic/Random.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <queue>
#include <utility>

namespace bomberman::logic {

namespace {

static constexpr std::array<std::pair<int, int>, 4> directions{
    std::pair<int, int>{-1, 0},
    std::pair<int, int>{1, 0},
    std::pair<int, int>{0, -1},
    std::pair<int, int>{0, 1},
};

bool isHarmfulPowerUp(PowerUpType type)
{
    return type == PowerUpType::RedX || type == PowerUpType::WoodenClogs || type == PowerUpType::Skull;
}

} // namespace

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

    //Check from bomb to tile, walls stop all and soft blocks stop after hit
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

std::optional<Vec2> World::findEscapeInput(const Character& character, int bombRow, int bombCol, int radius, int maxDistanceOverride) const
{
    //Small search, only need first move
    struct SearchNode {
        int row{0};
        int col{0};
        int distance{0};
        Vec2 firstInput{};
    };

    const int maxDistance = maxDistanceOverride >= 0 ? maxDistanceOverride : radius + 4;
    std::vector<bool> visited(static_cast<std::size_t>(numRows * numCols), false);
    std::queue<SearchNode> queue{};
    const auto indexFor = [this](int r, int c) {
        return static_cast<std::size_t>(r * numCols + c);
    };
    const auto [startRow, startCol] = tileForPosition(character.getPosition());

    visited[indexFor(startRow, startCol)] = true;

    //Start with four tiles around enemy
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

        //First safe tile is good enough
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

std::optional<std::vector<Vec2>> World::findEscapePath(const Character& character,
                                                        int bombRow,
                                                        int bombCol,
                                                        int radius,
                                                        int maxDistanceOverride) const
{
    //Same search but keep full path after enemy places bomb
    struct SearchNode {
        int row{0};
        int col{0};
        int distance{0};
        std::vector<Vec2> path{};
    };

    const int maxDistance = maxDistanceOverride >= 0 ? maxDistanceOverride : radius + 4;
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
        queue.push({r, c, 1, {directionForStep(rowStep, colStep)}});
    }

    while (!queue.empty()) {
        SearchNode node = queue.front();
        queue.pop();

        if (!isTileDangerous(node.row, node.col) &&
            !isTileThreatenedByBomb(node.row, node.col, bombRow, bombCol, radius)) {
            return node.path;
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
            auto nextPath = node.path;
            nextPath.push_back(directionForStep(rowStep, colStep));
            queue.push({r, c, node.distance + 1, std::move(nextPath)});
        }
    }

    return std::nullopt;
}

std::optional<Vec2> World::findStraightEscapeInput(const Character& character,
                                                   int bombRow,
                                                   int bombCol,
                                                   int radius) const
{
    //Try straight escape first because enemy moves tile by tile
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
    static constexpr int huntRange = 8;
    const auto [row, col] = tileForPosition(enemy.getPosition());

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
            if (!tileContains(EntityType::Bomb, nextRow, nextCol) && !isTileDangerous(nextRow, nextCol)) {
                return directionForStep(rowStep, colStep);
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

    if (playerChar != nullptr && playerChar->isAlive()) {
        const auto [playerRow, playerCol] = tileForPosition(playerChar->getPosition());
        const int rowDistance = playerRow - row;
        const int colDistance = playerCol - col;
        if (std::abs(rowDistance) + std::abs(colDistance) <= huntRange) {
            if (std::abs(rowDistance) >= std::abs(colDistance) && rowDistance != 0) {
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

            if (rowDistance != 0) {
                const int rowStep = rowDistance > 0 ? 1 : -1;
                if (canEnterTile(enemy, row + rowStep, col) && !isTileDangerous(row + rowStep, col)) {
                    return directionForStep(rowStep, 0);
                }
            }
        }
    }

    for (const auto& powerUp : powerUpsList) {
        if (powerUp == nullptr || !powerUp->isAlive()) {
            continue;
        }
        const auto powerUpType = powerUp->powerUpType();
        if (powerUpType.has_value() && isHarmfulPowerUp(*powerUpType)) {
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

bool World::hasAdjacentDestructibleBlock(int row, int col) const
{
    return std::any_of(directions.begin(), directions.end(), [this, row, col](const auto& direction) {
        return tileContains(EntityType::DestructibleBlock, row + direction.first, col + direction.second);
    });
}

void World::updateEnemies(float deltaTime)
{
    for (auto& ai : enemyAiStates) {
        const auto enemy = ai.enemy.lock();
        if (enemy == nullptr || !enemy->isAlive()) {
            continue;
        }

        enemy->updateStatus(deltaTime);
        if (enemy->isFrozen()) {
            ai.input = {};
            ai.hasTarget = false;
            continue;
        }

        ai.decisionTimer -= deltaTime;
        ai.bombCooldown -= deltaTime;

        const auto [row, col] = tileForPosition(enemy->getPosition());
        if (ai.escapingOwnBomb) {
            if (!hasBombAt(ai.escapeBombRow, ai.escapeBombCol)) {
                ai.escapingOwnBomb = false;
                ai.escapePath.clear();
                ai.input = {};
                ai.hasTarget = false;
                ai.decisionTimer = 0.0F;
            }
        }

        const bool dangerous = isTileDangerous(row, col);
        const auto straightEscapeInput = findStraightEscapeInput(*enemy, row, col, enemy->getBombRadius());
        const int bombRadiusForCheck = enemy->getBombRadius();

        //Before bomb, enemy checks way out
        std::vector<Vec2> escapeSteps{};
        if (straightEscapeInput.has_value()) {
            escapeSteps.assign(static_cast<std::size_t>(bombRadiusForCheck + 1), *straightEscapeInput);
        } else if (const auto path = findEscapePath(*enemy, row, col, bombRadiusForCheck, bombRadiusForCheck + 2);
                   path.has_value()) {
            escapeSteps = *path;
        }

        bool playerInBlastRange = false;
        if (playerChar != nullptr && playerChar->isAlive()) {
            const auto [playerRow, playerCol] = tileForPosition(playerChar->getPosition());
            playerInBlastRange = isTileThreatenedByBomb(playerRow, playerCol, row, col, enemy->getBombRadius());
        }

        //Bomb only if useful and enemy can run
        if (!ai.escapingOwnBomb &&
            !dangerous &&
            ai.bombCooldown <= 0.0F &&
            (hasAdjacentDestructibleBlock(row, col) || playerInBlastRange) &&
            !escapeSteps.empty()) {
            const int bombRadius = enemy->getBombRadius();
            placeBombFor(enemy);
            ai.bombCooldown = Random::instance().getRandomFloat(2.5F, 4.5F);
            ai.escapingOwnBomb = true;
            ai.escapeBombRow = row;
            ai.escapeBombCol = col;
            ai.escapeBombRadius = bombRadius;
            ai.escapePath = escapeSteps;
            ai.input = ai.escapePath.front();
            ai.hasTarget = false;
            ai.decisionTimer = 0.15F;
        }

        if (!ai.escapingOwnBomb &&
            (dangerous || ai.decisionTimer <= 0.0F || (ai.input.x == 0.0F && ai.input.y == 0.0F))) {
            //Pick new direction when old one is bad
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
        //Enemy moves tile by tile
        ai.targetRow = row + (ai.input.y > 0.0F ? 1 : (ai.input.y < 0.0F ? -1 : 0));
        ai.targetCol = col + (ai.input.x > 0.0F ? 1 : (ai.input.x < 0.0F ? -1 : 0));

        if (!canEnterTile(enemy, ai.targetRow, ai.targetCol)) {
            if (ai.escapingOwnBomb) {
                ai.escapePath.clear();
                if (const auto escapeInput = findEscapeInput(enemy,
                                                             ai.escapeBombRow,
                                                             ai.escapeBombCol,
                                                             ai.escapeBombRadius);
                    escapeInput.has_value()) {
                    ai.input = *escapeInput;
                    ai.hasTarget = false;
                } else {
                    ai.input = {};
                    ai.escapingOwnBomb = false;
                    ai.decisionTimer = 0.0F;
                }
            } else {
                ai.input = {};
                ai.decisionTimer = 0.0F;
            }
            return;
        }

        ai.hasTarget = true;
    }

    if (!ai.escapingOwnBomb && isTileDangerous(ai.targetRow, ai.targetCol)) {
        ai.hasTarget = false;
        ai.input = {};
        ai.decisionTimer = 0.0F;
        return;
    }

    const Vec2 target = tileCenter(ai.targetRow, ai.targetCol);
    Vec2 position = enemy.getPosition();


    if (ai.input.x != 0.0F) {
        //Snap to lane center so enemy stays clean
        const Vec2 currentCenter = tileCenter(row, col);
        position.y = currentCenter.y;
    } else if (ai.input.y != 0.0F) {
        //Same snap for vertical move
        const Vec2 currentCenter = tileCenter(row, col);
        position.x = currentCenter.x;
    }

    const float maxStep = enemy.getSpeed() * deltaTime;
    const float dx = target.x - position.x;
    const float dy = target.y - position.y;
    const float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= maxStep || distance <= 0.001F) {
        enemy.setPosition(target);
        ai.hasTarget = false;

        if (ai.escapingOwnBomb && hasBombAt(ai.escapeBombRow, ai.escapeBombCol)) {
            if (!ai.escapePath.empty()) {
                ai.escapePath.erase(ai.escapePath.begin());
            }

            if (!ai.escapePath.empty()) {
                ai.input = ai.escapePath.front();
            } else {
                const auto [currentRow, currentCol] = tileForPosition(enemy.getPosition());
                if (!isTileThreatenedByBomb(currentRow, currentCol, ai.escapeBombRow, ai.escapeBombCol, ai.escapeBombRadius) &&
                    !isTileDangerous(currentRow, currentCol)) {
                    ai.escapingOwnBomb = false;
                    ai.input = {};
                    ai.decisionTimer = 0.0F;
                    return;
                }

                if (const auto path = findEscapePath(enemy,
                                                     ai.escapeBombRow,
                                                     ai.escapeBombCol,
                                                     ai.escapeBombRadius,
                                                     ai.escapeBombRadius + 4);
                    path.has_value() && !path->empty()) {
                    ai.escapePath = *path;
                    ai.input = ai.escapePath.front();
                } else if (isTileThreatenedByBomb(currentRow,
                                                  currentCol,
                                                  ai.escapeBombRow,
                                                  ai.escapeBombCol,
                                                  ai.escapeBombRadius)) {
                    if (const auto reroute = findEscapeInput(enemy,
                                                             ai.escapeBombRow,
                                                             ai.escapeBombCol,
                                                             ai.escapeBombRadius);
                        reroute.has_value()) {
                        ai.input = *reroute;
                    } else {
                        ai.escapingOwnBomb = false;
                        ai.decisionTimer = 0.0F;
                    }
                }
            }
        }
        return;
    }

    position.x += dx / distance * maxStep;
    position.y += dy / distance * maxStep;

    //Move to chosen tile center
    enemy.setPosition(position);
}

}
