#include "logic/World.h"
#include "logic/Random.h"

#include <algorithm>
#include <vector>

namespace bomberman::logic {

namespace {

const std::vector<PowerUpType>& powerUpsForLevel(int level)
{
    static const std::vector<PowerUpType> level1{
        PowerUpType::Fire,
        PowerUpType::ExtraBomb,
        PowerUpType::Skates,
        PowerUpType::Stars,
    };
    static const std::vector<PowerUpType> level2{
        PowerUpType::Fire,
        PowerUpType::ExtraBomb,
        PowerUpType::Skates,
        PowerUpType::Stars,
        PowerUpType::PunchGlove,
        PowerUpType::PurpleTear,
        PowerUpType::RedX,
    };
    static const std::vector<PowerUpType> level3{
        PowerUpType::Fire,
        PowerUpType::ExtraBomb,
        PowerUpType::Skates,
        PowerUpType::Stars,
        PowerUpType::PunchGlove,
        PowerUpType::PurpleTear,
        PowerUpType::RedX,
        PowerUpType::WoodenClogs,
        PowerUpType::Skull,
    };

    if (level <= 1) {
        return level1;
    }
    if (level == 2) {
        return level2;
    }
    return level3;
}

} // namespace

void World::savePlayerStats()
{
    if (playerChar == nullptr) {
        return;
    }

    savedPlayerStats.bombRadius = playerChar->getBombRadius();
    savedPlayerStats.bombCapacity = playerChar->getBombCapacity();
    savedPlayerStats.speed = playerChar->getSpeed();
    savedPlayerStats.canKickBombs = playerChar->canKickBombs();
    savedPlayerStats.hasRubberBombs = playerChar->hasRubberBombs();
}

void World::restorePlayerStats()
{
    if (playerChar == nullptr) {
        return;
    }

    playerChar->setBombRadius(savedPlayerStats.bombRadius);
    playerChar->setBombCapacity(savedPlayerStats.bombCapacity);
    playerChar->setSpeed(savedPlayerStats.speed);
    if (savedPlayerStats.canKickBombs) {
        playerChar->enableBombKick();
    }
    if (savedPlayerStats.hasRubberBombs) {
        playerChar->enableRubberBombs();
    }
}

void World::resetPlayerStats()
{
    savedPlayerStats = {};
}

void World::createRandomPowerUp(int row, int col)
{
    if (!Random::instance().rollChance(0.25F)) {
        return;
    }

    //Later levels have more item types
    const auto& availableTypes = powerUpsForLevel(levelNumber);
    if (availableTypes.empty()) {
        return;
    }

    const int typeIndex = Random::instance().getRandomInt(0, static_cast<int>(availableTypes.size()) - 1);
    const auto type = availableTypes[static_cast<std::size_t>(typeIndex)];
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
            const int points = *powerUpType == PowerUpType::Stars ? 250 : 100;
            scoreTracker->onNotify({EventType::PowerUpCollected, powerUp->getId(), points});
        }
    }
}

void World::applyPowerUp(Character& character, PowerUpType type)
{
    switch (type) {
    case PowerUpType::Fire:
        character.expandExplosionRange();
        if (character.getType() == EntityType::Player) {
            savePlayerStats();
        }
        break;
    case PowerUpType::ExtraBomb:
        character.increaseMaxBombs();
        if (character.getType() == EntityType::Player) {
            savePlayerStats();
        }
        break;
    case PowerUpType::Skates:
        character.boostMovementSpeed(0.12F);
        if (character.getType() == EntityType::Player) {
            savePlayerStats();
        }
        break;
    case PowerUpType::Stars:
        break;
    case PowerUpType::PunchGlove:
        character.enableBombKick();
        if (character.getType() == EntityType::Player) {
            savePlayerStats();
        }
        break;
    case PowerUpType::PurpleTear:
        character.enableRubberBombs();
        if (character.getType() == EntityType::Player) {
            savePlayerStats();
        }
        break;
    case PowerUpType::RedX:
        character.disableBombs(5.0F);
        break;
    case PowerUpType::WoodenClogs:
        character.reduceMovementSpeed(0.12F);
        if (character.getType() == EntityType::Player) {
            savePlayerStats();
        }
        break;
    case PowerUpType::Skull:
        if (character.getType() == EntityType::Player) {
            damagePlayer();
        } else {
            character.killEntity();
        }
        break;
    }
}

}
