#ifndef BOMBERMAN_AP_SFML_CAMERA_H
#define BOMBERMAN_AP_SFML_CAMERA_H

#include "bomberman/logic/Geometry.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace bomberman::sfml {

class Camera final {
public:
    explicit Camera(sf::Vector2u windowSize)
        : windowSize(windowSize)
    {
    }

    sf::Vector2f project(logic::Vec2 position) const
    {
        return {(position.x + 1.0F) * 0.5F * static_cast<float>(windowSize.x),
                (position.y + 1.0F) * 0.5F * static_cast<float>(windowSize.y)};
    }

    sf::Vector2f projectSize(logic::Vec2 size) const
    {
        return {size.x * 0.5F * static_cast<float>(windowSize.x),
                size.y * 0.5F * static_cast<float>(windowSize.y)};
    }

private:
    sf::Vector2u windowSize{};
};

}

#endif //BOMBERMAN_AP_SFML_CAMERA_H
