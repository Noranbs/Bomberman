#ifndef BOMBERMAN_AP_LOGIC_EVENT_H
#define BOMBERMAN_AP_LOGIC_EVENT_H

#include <cstddef>

namespace bomberman::logic {

enum class EventType {
    EntityMoved,
    EntityDied,
    BlockDestroyed,
    PowerUpCollected,
    EnemyKilled,
    PlayerWon,
    PlayerLost,
    Tick
};

struct Event {
    EventType type{EventType::Tick};
    std::size_t entityId{0};
    int value{0};
};

class Observer {
public:
    virtual ~Observer() = default;
    virtual void onNotify(const Event& event) = 0;
};

}

#endif //BOMBERMAN_AP_LOGIC_EVENT_H
