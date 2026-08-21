#ifndef BOMBERMAN_AP_LOGIC_EVENT_H
#define BOMBERMAN_AP_LOGIC_EVENT_H

#include <cstddef>
#include <memory>
#include <vector>

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

/**
 * @brief Base class for objects that can notify observers about events.
 */
class Subject {
public:
    virtual ~Subject() = default;

    /**
     * @brief Adds an observer that listens to this subject.
     * @param observer Observer stored as weak pointer.
     */
    void addObserver(const std::weak_ptr<Observer>& observer);

    /**
     * @brief Sends an event to all observers.
     * @param event Event to send.
     */
    void notify(const Event& event);

private:
    std::vector<std::weak_ptr<Observer>> observers{}; ///< Objects listening to this subject.
};

}

#endif //BOMBERMAN_AP_LOGIC_EVENT_H
