#ifndef BOMBERMAN_AP_LOGIC_EVENT_H
#define BOMBERMAN_AP_LOGIC_EVENT_H

#include <cstddef>

namespace bomberman::logic {

/**
 * @brief Types of events that can happen in the logic layer.
 */
enum class EventType {
    EntityMoved,       ///< An entity changed position or direction.
    EntityDied,        ///< An entity died.
    BlockDestroyed,    ///< A destructible block was destroyed.
    PowerUpCollected,  ///< A power-up was collected.
    EnemyKilled,       ///< An enemy was killed.
    PlayerDamaged,     ///< The player lost one life.
    PlayerWon,         ///< The player won the level or game.
    PlayerLost,        ///< The player lost the game.
    Tick               ///< General update event.
};

/**
 * @brief Small message sent from entities to observers.
 */
struct Event {
    /// What happened.
    EventType type{EventType::Tick};
    /// Id of the entity that caused the event.
    std::size_t entityId{0};
    /// Extra value for the event, for example remaining lives.
    int value{0};
};

/**
 * @brief Interface for classes that react to game events.
 */
class Observer {
public:
    virtual ~Observer() = default;

    /**
     * @brief Called when the observed object sends an event.
     */
    virtual void onNotify(const Event& event) = 0;
};

}

#endif //BOMBERMAN_AP_LOGIC_EVENT_H
