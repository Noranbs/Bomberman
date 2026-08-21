#include "logic/World.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace bomberman::logic {

bool World::bombBlocksCharacter(const Entity& bomb, const Character& character) const
{
    //Find the bomb state for this bomb entity
    const auto found = std::find_if(bombsList.begin(), bombsList.end(), [&bomb](const BombState& state) {
        return state.entity != nullptr && state.entity->getId() == bomb.getId();
    });

    if (found == bombsList.end()) {
        return true;
    }

    const auto owner = found->owner.lock();
    //Bomb blocks everybody, except owner before he walks away
    return owner == nullptr || owner->getId() != character.getId() || found->blocksOwner;
}

bool World::hasBombAt(int row, int col) const
{
    //Use tiny box in tile center to check if bomb sits on this tile
    const Vec2 center = tileCenter(row, col);
    return std::any_of(bombsList.begin(), bombsList.end(), [&center](const BombState& state) {
        return state.entity != nullptr && state.entity->isAlive() &&
               state.entity->getBounds().intersects({center, {0.001F, 0.001F}});
    });
}

bool World::canBombMoveTo(int row, int col, std::size_t movingBombId) const
{
    if (!isInsideArena(row, col)) {
        return false;
    }

    //Kicked bombs cannot pass walls, soft blocks, or other bombs
    const Rect bounds{tileCenter(row, col), {tileDimensions.x * 0.5F, tileDimensions.y * 0.5F}};
    return std::none_of(entitiesList.begin(), entitiesList.end(), [movingBombId, &bounds](const auto& entity) {
        if (entity == nullptr || !entity->isAlive() || entity->getId() == movingBombId) {
            return false;
        }
        if (entity->getType() != EntityType::Wall &&
            entity->getType() != EntityType::DestructibleBlock &&
            entity->getType() != EntityType::Bomb) {
            return false;
        }
        return entity->getBounds().intersects(bounds);
    });
}

void World::placePlayerBomb()
{
    //Player bomb uses same function as enemy bomb
    placeBombFor(playerChar);
}

void World::placeBombFor(const std::shared_ptr<Character>& character)
{
    //tryPlaceBomb checks capacity and temporary no-bomb effect
    if (character == nullptr || !character->isAlive() || !character->tryPlaceBomb()) {
        return;
    }

    const auto [row, col] = tileForPosition(character->getPosition());
    if (hasBombAt(row, col)) {
        //No double bomb on one tile, give the slot back
        character->replenishBomb();
        return;
    }

    //New bomb starts on character tile and uses character radius
    auto bomb = factory->createBomb(nextId(), tileCenter(row, col), tileDimensions);
    bombsList.push_back({bomb, character, 2.0F, character->getBombRadius(), false, false, {}, row, col, false});
    entitiesList.push_back(bomb);
}

void World::kickPlayerBomb()
{
    if (playerChar == nullptr || !playerChar->isAlive() || !playerChar->canKickBombs()) {
        return;
    }

    //Look one tile in the direction player faces
    const auto [playerRow, playerCol] = tileForPosition(playerChar->getPosition());
    int rowStep = 0;
    int colStep = 0;
    switch (playerChar->getDirection()) {
    case Direction::Down:
        rowStep = 1;
        break;
    case Direction::Left:
        colStep = -1;
        break;
    case Direction::Right:
        colStep = 1;
        break;
    case Direction::Up:
        rowStep = -1;
        break;
    }

    const int bombRow = playerRow + rowStep;
    const int bombCol = playerCol + colStep;
    //Only kick if there is a live bomb right there
    const auto found = std::find_if(bombsList.begin(), bombsList.end(), [this, bombRow, bombCol](const BombState& bomb) {
        if (bomb.exploded || bomb.entity == nullptr || !bomb.entity->isAlive()) {
            return false;
        }
        const auto [row, col] = tileForPosition(bomb.entity->getPosition());
        return row == bombRow && col == bombCol;
    });

    if (found == bombsList.end() || found->entity == nullptr) {
        return;
    }

    const int targetRow = bombRow + rowStep;
    const int targetCol = bombCol + colStep;
    if (!canBombMoveTo(targetRow, targetCol, found->entity->getId())) {
        //Without rubber bomb upgrade, blocked kick does nothing
        if (!playerChar->hasRubberBombs()) {
            return;
        }

        //With rubber bomb, it may bounce back if the back tile is free
        const int bounceRow = bombRow - rowStep;
        const int bounceCol = bombCol - colStep;
        if (!canBombMoveTo(bounceRow, bounceCol, found->entity->getId())) {
            return;
        }

        rowStep = -rowStep;
        colStep = -colStep;
    }

    //Save movement data, updateMovingBomb will move it each frame
    found->blocksOwner = true;
    found->moveDirection = {static_cast<float>(colStep), static_cast<float>(rowStep)};
    found->targetRow = bombRow + rowStep;
    found->targetCol = bombCol + colStep;
    found->rubberBounce = playerChar->hasRubberBombs();
}

void World::updateBombs(float deltaTime)
{
    //First update timers and moving bombs
    for (auto& bomb : bombsList) {
        if (bomb.exploded || bomb.entity == nullptr || !bomb.entity->isAlive()) {
            continue;
        }

        updateMovingBomb(bomb, deltaTime);

        const auto owner = bomb.owner.lock();
        if (owner == nullptr || !owner->getBounds().intersects(bomb.entity->getBounds())) {
            //Owner can stand on bomb until walking away
            bomb.blocksOwner = true;
        }

        bomb.timer -= deltaTime;
    }

    bool detonatedBomb = true;
    while (detonatedBomb) {
        //Loop because bombs can trigger bombs
        detonatedBomb = false;
        for (std::size_t index = 0; index < bombsList.size(); ++index) {
            if (bombsList[index].exploded ||
                bombsList[index].entity == nullptr ||
                !bombsList[index].entity->isAlive() ||
                bombsList[index].timer > 0.0F) {
                continue;
            }

            explodeBomb(index);
            detonatedBomb = true;
        }
    }

    //Remove bombs that already exploded
    bombsList.erase(std::remove_if(bombsList.begin(),
                                   bombsList.end(),
                                   [](const BombState& bomb) {
                                       return bomb.exploded ||
                                              bomb.entity == nullptr ||
                                              !bomb.entity->isAlive();
                                   }),
                    bombsList.end());
}

void World::updateMovingBomb(BombState& bomb, float deltaTime)
{
    if (bomb.entity == nullptr || (bomb.moveDirection.x == 0.0F && bomb.moveDirection.y == 0.0F)) {
        return;
    }

    //Move kicked bomb to the next tile center
    static constexpr float movingBombSpeed = 1.55F;
    const Vec2 target = tileCenter(bomb.targetRow, bomb.targetCol);
    Vec2 position = bomb.entity->getPosition();
    const float dx = target.x - position.x;
    const float dy = target.y - position.y;
    const float distance = std::sqrt(dx * dx + dy * dy);
    const float step = movingBombSpeed * deltaTime;

    if (distance <= step || distance <= 0.0001F) {
        bomb.entity->setPosition(target);

        //Kicked bomb can keep going, bounce, or stop
        const int rowStep = static_cast<int>(bomb.moveDirection.y);
        const int colStep = static_cast<int>(bomb.moveDirection.x);
        const int nextRow = bomb.targetRow + rowStep;
        const int nextCol = bomb.targetCol + colStep;
        if (canBombMoveTo(nextRow, nextCol, bomb.entity->getId())) {
            //Next tile free, keep rolling
            bomb.targetRow = nextRow;
            bomb.targetCol = nextCol;
        } else if (bomb.rubberBounce) {
            const int bounceRow = bomb.targetRow - rowStep;
            const int bounceCol = bomb.targetCol - colStep;
            if (canBombMoveTo(bounceRow, bounceCol, bomb.entity->getId())) {
                //Rubber bomb changes direction
                bomb.moveDirection = {-bomb.moveDirection.x, -bomb.moveDirection.y};
                bomb.targetRow = bounceRow;
                bomb.targetCol = bounceCol;
            } else {
                //Blocked both ways
                bomb.moveDirection = {};
            }
        } else {
            bomb.moveDirection = {};
        }
        return;
    }

    position.x += dx / distance * step;
    position.y += dy / distance * step;
    bomb.entity->setPosition(position);
}

void World::updateExplosions(float deltaTime)
{
    //Explosion tiles are visual and only live for a short time
    for (auto& explosion : explosionsList) {
        explosion.timer -= deltaTime;
        if (explosion.timer <= 0.0F && explosion.entity != nullptr) {
            explosion.entity->killEntity();
        }
    }
}

void World::explodeBomb(std::size_t index)
{
    if (index >= bombsList.size() || bombsList[index].exploded || bombsList[index].entity == nullptr) {
        return;
    }

    //Copy the state because the vector can change while chain reactions happen
    auto bomb = bombsList[index];
    bombsList[index].exploded = true;
    const auto [row, col] = tileForPosition(bomb.entity->getPosition());
    const auto owner = bomb.owner.lock();
    const bool playerOwnedBomb = owner != nullptr && owner->getType() == EntityType::Player;
    bomb.entity->killEntity();

    if (owner != nullptr) {
        //Bomb slot comes back when bomb explodes
        owner->replenishBomb();
    }

    //Center tile also gets explosion
    applyExplosionToTile(row, col, playerOwnedBomb);

    //Blast goes four ways from center
    static constexpr std::array<std::pair<int, int>, 4> directions{
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0},
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
    };

    for (const auto& [rowStep, colStep] : directions) {
        for (int distance = 1; distance <= bomb.radius; ++distance) {
            if (!applyExplosionToTile(row + rowStep * distance, col + colStep * distance, playerOwnedBomb)) {
                break;
            }
        }
    }
}

void World::createExplosionTile(int row, int col)
{
    auto explosion = factory->createExplosion(nextId(), tileCenter(row, col), tileDimensions, ExplosionShape::Center);
    explosionsList.push_back({explosion, 0.65F});
    entitiesList.push_back(explosion);
}

bool World::applyExplosionToTile(int row, int col, bool playerOwnedBomb)
{
    if (!isInsideArena(row, col)) {
        return false;
    }

    //This rectangle represents the tile hit by the blast
    const Rect tileBounds{tileCenter(row, col), {tileDimensions.x * 0.5F, tileDimensions.y * 0.5F}};

    for (auto& entity : entitiesList) {
        if (entity->isAlive() && entity->getType() == EntityType::Wall && entity->getBounds().intersects(tileBounds)) {
            //Hard wall stops blast
            return false;
        }
    }

    //If no hard wall blocks it, draw fire on this tile
    createExplosionTile(row, col);

    const auto destructibleBlock = std::find_if(entitiesList.begin(), entitiesList.end(), [&tileBounds](const auto& entity) {
        return entity->isAlive() && entity->getType() == EntityType::DestructibleBlock &&
               entity->getBounds().intersects(tileBounds);
    });

    if (destructibleBlock != entitiesList.end()) {
        //Soft wall breaks, maybe drops item, then blast stops
        (*destructibleBlock)->killEntity();
        if (playerOwnedBomb) {
            scoreTracker->onNotify({EventType::BlockDestroyed, (*destructibleBlock)->getId(), 50});
        }
        createRandomPowerUp(row, col);
        return false;
    }

    for (auto& entity : entitiesList) {
        if (!entity->isAlive() || !entity->getBounds().intersects(tileBounds)) {
            continue;
        }

        switch (entity->getType()) {
        case EntityType::Player:
            damagePlayer();
            break;

        case EntityType::Enemy:
            entity->killEntity();
            if (playerOwnedBomb) {
                //Only player bombs give player score
                scoreTracker->onNotify({EventType::EnemyKilled, entity->getId(), 250});
            }
            break;

        case EntityType::Bomb: {
            //Chain reaction
            const auto foundBomb = std::find_if(bombsList.begin(), bombsList.end(), [&entity](const BombState& state) {
                return state.entity != nullptr && state.entity->getId() == entity->getId();
            });
            if (foundBomb != bombsList.end() && !foundBomb->exploded) {
                foundBomb->timer = 0.0F;
            }
            break;
        }

        case EntityType::Wall:
        case EntityType::DestructibleBlock:
        case EntityType::Explosion:
        case EntityType::Exit:
            break;

        case EntityType::PowerUp:
            //visible power ups burn too
            entity->killEntity();
            break;
        }
    }

    return true;
}

}
